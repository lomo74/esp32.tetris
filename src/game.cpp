/*
TETRIS for ESP32 with SSD1306 OLED display

MIT License

Copyright (c) 2025 Lorenzo Monti

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "game.h"
#include "joystick.h"
#include "tetromino.h"

const uint32_t FALLING_SPEED = 400; // milliseconds

Game::Game(Joystick* pJoystick, Adafruit_SSD1306* pDisplay)
: m_pJoystick(pJoystick), m_pDisplay(pDisplay) {
}

Game::~Game() {
}

bool Game::begin() {
    return true;
}

void Game::clear() {
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            m_Board[i][j] = false;
        }
    }
}

void Game::waitCoins() {
    m_pJoystick->enable();

    render(RENDER_MODE_INSERT_COINS);
    m_pJoystick->waitMove(portMAX_DELAY); // wait for a button press
}

void Game::playMatch() {
    m_pJoystick->enable();
    
    clear();

    for (;;) {
        // try to place a new tetromino
        // if it overlaps, game over
        if (newTetromino()) {
            render(RENDER_MODE_PLAYING);

            uint32_t timeLeft = FALLING_SPEED; // the time left for the tetromino to fall

            for (;;) {
                bool moved = false, landed = false, fallen = false;

                uint32_t startTime = millis();

                Joystick::Button button = m_pJoystick->waitMove(timeLeft);

                switch (button) {
                    case Joystick::BUTTON_LEFT:
                        moved = moveTetromino(-1);
                        break;
                    case Joystick::BUTTON_RIGHT:
                        moved = moveTetromino(1);
                        break;
                    case Joystick::BUTTON_ROTATE:
                        moved = rotateTetromino();
                        break;
                    default:
                        fallen = true;
                        landed = !moveTetromino(0, -1); // if it can't move down, it has landed
                        timeLeft = FALLING_SPEED;
                        break;
                }

                if (fallen || landed || moved) {
                    // something changed on the screen, redraw needed
                    render(RENDER_MODE_PLAYING);
                }
                
                if (!fallen) {
                    // if the tetromino hasn't fallen yet, calculate the time left for another move
                    uint32_t endTime = millis();

                    uint32_t elapsedTime = endTime - startTime;
                    if (timeLeft > elapsedTime) {
                        timeLeft -= elapsedTime;
                    } else {
                        timeLeft = 0;
                    }

                    Serial.printf("Time left: %dms\n", timeLeft);
                }

                if (landed) {
                    Serial.printf("X=%d Y=%d - tetromino landed!\n", m_TetrominoX, m_TetrominoY);
                    placeTetromino(); // place the tetromino on the board
                    break;
                }
            }

            removeCompletedRows();
        } else {
            return;
        }
    }
}

void Game::over() {
    m_pJoystick->enable();

    render(RENDER_MODE_GAME_OVER);
    m_pJoystick->waitMove(portMAX_DELAY); // wait for a button press

    m_pJoystick->disable();
}

void Game::render(RenderMode mode) {
    m_pDisplay->clearDisplay();

    // game board border
    m_pDisplay->drawFastVLine(0, 0, PLAYSCREEN_HEIGHT, SSD1306_WHITE);
    m_pDisplay->drawFastVLine(PLAYSCREEN_WIDTH - 1, 0, PLAYSCREEN_HEIGHT, SSD1306_WHITE);

    switch (mode) {
        case RENDER_MODE_INSERT_COINS:
            m_pDisplay->setTextSize(2);
            m_pDisplay->setCursor(15, 20);
            m_pDisplay->print(F("INS"));
            m_pDisplay->setCursor(15, 40);
            m_pDisplay->print(F("ERT"));
            m_pDisplay->setCursor(21, 60);
            m_pDisplay->print(F("CO"));
            m_pDisplay->setCursor(15, 80);
            m_pDisplay->print(F("INS"));
            break;

        case RENDER_MODE_PLAYING:
            // draw the game board
            for (int i = 0; i < BOARD_HEIGHT; i++) {
                for (int j = 0; j < BOARD_WIDTH; j++) {
                    if (m_Completed[i]) {
                        // we're going to remove this row, so draw some dots to simulate an explosion
                        m_pDisplay->drawPixel(LEFT_MARGIN + (j * BLOCK_WIDTH) + (BLOCK_WIDTH / 2), PLAYSCREEN_HEIGHT - (i * BLOCK_HEIGHT) - (BLOCK_HEIGHT / 2), SSD1306_WHITE);
                    } else if (m_Board[i][j]) {
                        m_pDisplay->fillRect(LEFT_MARGIN + (j * BLOCK_WIDTH), PLAYSCREEN_HEIGHT - ((i + 1) * BLOCK_HEIGHT), BLOCK_WIDTH, BLOCK_HEIGHT, SSD1306_WHITE);
                    }
                }
            }
            
            // draw the falling tetromino
            // m_pTetromino is NULL if the tetromino has landed
            if (m_pTetromino) {
                for (int i = 0; i < 4; i++) {
                    assert(!m_Completed[i]);
                    int8_t x = m_TetrominoX + m_pTetromino->blocks[i].x;
                    int8_t y = m_TetrominoY + m_pTetromino->blocks[i].y;
                    m_pDisplay->fillRect(LEFT_MARGIN + (x * BLOCK_WIDTH), PLAYSCREEN_HEIGHT - ((y + 1) * BLOCK_HEIGHT), BLOCK_WIDTH, BLOCK_HEIGHT, SSD1306_WHITE);
                }

                // falling hint - left boundary
                for (int i = m_TetrominoY + m_pTetromino->leftboundary.y - 1; i >= 0; i--) {
                    int8_t x = m_TetrominoX + m_pTetromino->leftboundary.x;
                    m_pDisplay->drawPixel(LEFT_MARGIN + (x * BLOCK_WIDTH), PLAYSCREEN_HEIGHT - (i * BLOCK_HEIGHT) - (BLOCK_HEIGHT / 2), SSD1306_WHITE);
                    if (i == 0 || m_Board[i][x])
                        break;
                }

                // falling hint - right boundary
                for (int i = m_TetrominoY + m_pTetromino->rightboundary.y - 1; i >= 0; i--) {
                    int8_t x = m_TetrominoX + m_pTetromino->rightboundary.x + 1;
                    m_pDisplay->drawPixel(LEFT_MARGIN + (x * BLOCK_WIDTH) - 1, PLAYSCREEN_HEIGHT - (i * BLOCK_HEIGHT) - (BLOCK_HEIGHT / 2), SSD1306_WHITE);
                    if (i == 0 || m_Board[i][x - 1])
                        break;
                }
            }
            break;

        case RENDER_MODE_GAME_OVER:
            m_pDisplay->setTextSize(2);
            m_pDisplay->setCursor(9, 30);
            m_pDisplay->print(F("GAME"));
            m_pDisplay->setCursor(9, 60);
            m_pDisplay->print(F("OVER"));
            break;

        default:
            break;
    }

    m_pDisplay->display();
}

bool Game::newTetromino() {
    m_TetrominoX = BOARD_WIDTH / 2 - 1;
    m_TetrominoY = BOARD_HEIGHT - 1;
    m_TetrominoType = static_cast<TetrominoType>(random(0, TETROMINO_COUNT));
    m_TetrominoRotation = ROTATION_0;
    m_pTetromino = &(Pieces[m_TetrominoType][m_TetrominoRotation]);

    return !tetrominoOverlaps(); // if it overlaps, game over
}

// deltaX and deltaY allow us to check for overlaps befor the move takes place
// pTetromino is used to check for overlaps when rotating the tetromino
bool Game::tetrominoOverlaps(const Tetromino* pTetromino, int8_t deltaX, int8_t deltaY) {
    if (pTetromino == NULL) {
        pTetromino = m_pTetromino;
    }

    assert(pTetromino != NULL);

    for (int i = 0; i < 4; i++) {
        int8_t x = m_TetrominoX + pTetromino->blocks[i].x + deltaX;
        int8_t y = m_TetrominoY + pTetromino->blocks[i].y + deltaY;

        assert(y < BOARD_HEIGHT);

        if (x < 0 || x >= BOARD_WIDTH || y < 0) {
            return true;
        }

        if (m_Board[y][x]) {
            return true;
        }
    }

    return false;
}

void Game::placeTetromino() {
    assert(m_pTetromino != NULL);

    for (int i = 0; i < 4; i++) {
        int8_t x = m_TetrominoX + m_pTetromino->blocks[i].x;
        int8_t y = m_TetrominoY + m_pTetromino->blocks[i].y;

        assert(x >= 0 && x < BOARD_WIDTH);
        assert(y >= 0 && y < BOARD_HEIGHT);

        m_Board[y][x] = true;
    }

    m_pTetromino = NULL;
}

bool Game::moveTetromino(int8_t deltaX, int8_t deltaY) {
    assert(m_pTetromino != NULL);

    if (tetrominoOverlaps(m_pTetromino, deltaX, deltaY)) {
        return false;
    }
    
    m_TetrominoX += deltaX;
    m_TetrominoY += deltaY;

    return true;
}

bool Game::rotateTetromino() {
    // pointer to the next rotation of the tetromino
    const Tetromino* pTetromino = &(Pieces[m_TetrominoType][(m_TetrominoRotation + 1) % ROTATION_COUNT]);

    // would the rotated tetromino overlap?
    if (tetrominoOverlaps(pTetromino)) {
        return false;
    }

    m_TetrominoRotation = static_cast<TetrominoRotation>((m_TetrominoRotation + 1) % ROTATION_COUNT);
    m_pTetromino = pTetromino;

    return true;
}

void Game::removeCompletedRows() {
    if (clearCompletedRows()) {
        m_pJoystick->disable(); // we don't accept any input while the rows are being removed

        render(RENDER_MODE_PLAYING); // draw the game board. The completed rows are drawn using dots
        
        delay(200);
        compactBoard(); // remove the completed rows
        
        render(RENDER_MODE_PLAYING); // draw the game board again
        
        m_pJoystick->enable(); // enable input again
    }
}

bool Game::clearCompletedRows() {
    bool hasCompletedRows = false;

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        m_Completed[i] = true;

        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (!m_Board[i][j]) {
                m_Completed[i] = false;
                break;
            }
        }

        hasCompletedRows = hasCompletedRows || m_Completed[i];
    }

    return hasCompletedRows;
}

void Game::compactBoard() {
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        if (m_Completed[i]) {
            // move all rows above the completed row down by one
            for (int j = i; j < BOARD_HEIGHT - 1; j++) {
                for (int k = 0; k < BOARD_WIDTH; k++) {
                    m_Board[j][k] = m_Board[j + 1][k];
                }
            }

            // clear the topmost row
            for (int k = 0; k < BOARD_WIDTH; k++) {
                m_Board[BOARD_HEIGHT - 1][k] = false;
            }

            // mark the completed row as not completed anymore
            m_Completed[i] = false;
        }
    }
}

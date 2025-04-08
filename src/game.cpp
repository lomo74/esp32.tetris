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
    m_pJoystick->waitMove(portMAX_DELAY);
}

void Game::playMatch() {
    m_pJoystick->enable();
    
    clear();

    for (;;) {
        if (newTetromino()) {
            uint32_t timeLeft = FALLING_SPEED;

            for (;;) {
                bool shouldRedraw = false;
                bool landed = false, fallen = false;

                uint32_t startTime = millis();

                Joystick::Button button = m_pJoystick->waitMove(timeLeft);

                switch (button) {
                    case Joystick::BUTTON_LEFT:
                        shouldRedraw = moveTetromino(-1);
                        break;
                    case Joystick::BUTTON_RIGHT:
                        shouldRedraw = moveTetromino(1);
                        break;
                    case Joystick::BUTTON_ROTATE:
                        shouldRedraw = rotateTetromino();
                        break;
                    default:
                        fallen = true;
                        landed = !moveTetromino(0, -1);
                        timeLeft = FALLING_SPEED;
                        break;
                }

                if (fallen || landed || shouldRedraw) {
                    render(RENDER_MODE_PLAYING);
                }
                
                if (!fallen) {
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
                    placeTetromino();
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
    m_pJoystick->waitMove(portMAX_DELAY);

    m_pJoystick->disable();
}

void Game::render(RenderMode mode) {
    m_pDisplay->clearDisplay();

    m_pDisplay->drawFastVLine(0, 0, PLAYSCREEN_HEIGHT, SSD1306_WHITE);
    m_pDisplay->drawFastVLine(PLAYSCREEN_WIDTH - 1, 0, PLAYSCREEN_HEIGHT, SSD1306_WHITE);

    switch (mode) {
        case RENDER_MODE_INSERT_COINS:
            m_pDisplay->setCursor(8, 20);
            m_pDisplay->print(F("INS"));
            m_pDisplay->setCursor(8, 35);
            m_pDisplay->print(F("ERT"));
            m_pDisplay->setCursor(11, 50);
            m_pDisplay->print(F("CO"));
            m_pDisplay->setCursor(8, 65);
            m_pDisplay->print(F("INS"));
            break;

        case RENDER_MODE_PLAYING:
            for (int i = 0; i < BOARD_HEIGHT; i++) {
                for (int j = 0; j < BOARD_WIDTH; j++) {
                    if (m_Completed[i]) {
                        m_pDisplay->drawPixel(1 + (j * BLOCK_WIDTH) + (BLOCK_WIDTH / 2), PLAYSCREEN_HEIGHT - (i * BLOCK_HEIGHT) - (BLOCK_HEIGHT / 2), SSD1306_WHITE);
                    } else if (m_Board[i][j]) {
                        m_pDisplay->fillRect(1 + (j * BLOCK_WIDTH), PLAYSCREEN_HEIGHT - ((i + 1) * BLOCK_HEIGHT), BLOCK_WIDTH, BLOCK_HEIGHT, SSD1306_WHITE);
                    }
                }
            }
            
            if (m_pTetromino) {
                for (int i = 0; i < 4; i++) {
                    assert(!m_Completed[i]);
                    int8_t x = m_TetrominoX + m_pTetromino->blocks[i].x;
                    int8_t y = m_TetrominoY + m_pTetromino->blocks[i].y;
                    m_pDisplay->fillRect(1 + (x * BLOCK_WIDTH), PLAYSCREEN_HEIGHT - ((y + 1) * BLOCK_HEIGHT), BLOCK_WIDTH, BLOCK_HEIGHT, SSD1306_WHITE);
                }

                for (int i = m_TetrominoY + m_pTetromino->leftboundary.y - 1; i >= 0; i--) {
                    int8_t x = m_TetrominoX + m_pTetromino->leftboundary.x;
                    m_pDisplay->drawPixel(1 + (x * BLOCK_WIDTH), PLAYSCREEN_HEIGHT - (i * BLOCK_HEIGHT) - (BLOCK_HEIGHT / 2), SSD1306_WHITE);
                    if (i == 0 || m_Board[i][x])
                        break;
                }

                for (int i = m_TetrominoY + m_pTetromino->rightboundary.y - 1; i >= 0; i--) {
                    int8_t x = m_TetrominoX + m_pTetromino->rightboundary.x + 1;
                    m_pDisplay->drawPixel(x * BLOCK_WIDTH, PLAYSCREEN_HEIGHT - (i * BLOCK_HEIGHT) - (BLOCK_HEIGHT / 2), SSD1306_WHITE);
                    if (i == 0 || m_Board[i][x - 1])
                        break;
                }
            }
            break;

        case RENDER_MODE_GAME_OVER:
            m_pDisplay->setCursor(5, 30);
            m_pDisplay->print(F("GAME"));
            m_pDisplay->setCursor(5, 50);
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

    return !tetrominoOverlaps();
}

bool Game::tetrominoOverlaps(const Tetromino* pTetromino, int8_t deltaX, int8_t deltaY) {
    if (pTetromino == NULL) {
        pTetromino = m_pTetromino;
    }

    assert(pTetromino != NULL);

    for (int i = 0; i < 4; i++) {
        int8_t x = m_TetrominoX + pTetromino->blocks[i].x + deltaX;
        int8_t y = m_TetrominoY + pTetromino->blocks[i].y + deltaY;

        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
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
    const Tetromino* pTetromino = &(Pieces[m_TetrominoType][(m_TetrominoRotation + 1) % ROTATION_COUNT]);

    if (tetrominoOverlaps(pTetromino)) {
        return false;
    }

    m_TetrominoRotation = static_cast<TetrominoRotation>((m_TetrominoRotation + 1) % ROTATION_COUNT);
    m_pTetromino = pTetromino;

    return true;
}

void Game::removeCompletedRows() {
    if (clearCompletedRows()) {
        m_pJoystick->disable();
        render(RENDER_MODE_PLAYING);
        delay(200);
        compactBoard();
        render(RENDER_MODE_PLAYING);
        m_pJoystick->enable();
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

        if (m_Completed[i]) {
            for (int j = 0; j < BOARD_WIDTH; j++) {
                m_Board[i][j] = false;
            }

            hasCompletedRows = true;
        }
    }

    return hasCompletedRows;
}

void Game::compactBoard() {
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        if (m_Completed[i]) {
            for (int j = i; j < BOARD_HEIGHT - 1; j++) {
                for (int k = 0; k < BOARD_WIDTH; k++) {
                    m_Board[j][k] = m_Board[j + 1][k];
                }
            }

            for (int k = 0; k < BOARD_WIDTH; k++) {
                m_Board[BOARD_HEIGHT - 1][k] = false;
            }

            m_Completed[i] = false;
        }
    }
}

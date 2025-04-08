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
#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "joystick.h"
#include "tetromino.h"

#define PLAYSCREEN_WIDTH 32
#define PLAYSCREEN_HEIGHT 128

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 25

#define BLOCK_WIDTH 3
#define BLOCK_HEIGHT 5

class Game {
private:
    bool m_Board[BOARD_HEIGHT][BOARD_WIDTH];
    Joystick* m_pJoystick;
    Adafruit_SSD1306* m_pDisplay;
    EventGroupHandle_t m_Pressed;
    bool m_Completed[BOARD_HEIGHT];

public:
    explicit Game(Joystick* pJoystick, Adafruit_SSD1306* pDisplay);
    virtual ~Game();

private:
    int8_t m_TetrominoX;
    int8_t m_TetrominoY;
    TetrominoType m_TetrominoType;
    TetrominoRotation m_TetrominoRotation;
    const Tetromino* m_pTetromino;

    enum RenderMode {
        RENDER_MODE_NONE = 0,
        RENDER_MODE_INSERT_COINS,
        RENDER_MODE_PLAYING,
        RENDER_MODE_GAME_OVER,
        RENDER_MODE_COUNT
    };

    bool newTetromino();
    bool rotateTetromino();
    bool moveTetromino(int8_t deltaX, int8_t deltaY = 0);
    void placeTetromino();
    bool tetrominoOverlaps(const Tetromino* pTetromino = NULL, int8_t deltaX = 0, int8_t deltaY = 0);
    void render(RenderMode mode = RENDER_MODE_PLAYING);
    void clear();
    void removeCompletedRows();
    bool clearCompletedRows();
    void compactBoard();

public:
    bool begin();
    void waitCoins();
    void playMatch();
    void over();
};

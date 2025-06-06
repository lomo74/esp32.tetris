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
#include "tetromino.h"

const Tetromino Pieces[TETROMINO_COUNT][ROTATION_COUNT] = {
    // I
    {
        // ROTATION_0
        {
            { {-1, -1}, {0, -1}, {1, -1}, {2, -1} },
            {-1, -1}, {2, -1}
        },
        // ROTATION_90
        {
            { {0, 0}, {0, -1}, {0, -2}, {0, -3} },
            {0, -3}, {0, -3}
        },
        // ROTATION_180
        {
            { {-1, -1}, {0, -1}, {1, -1}, {2, -1} },
            {-1, -1}, {2, -1}
        },
        // ROTATION_270
        {
            { {0, 0}, {0, -1}, {0, -2}, {0, -3} },
            {0, -3}, {0, -3}
        }
    },
    // J
    {
        // ROTATION_0
        {
            { {-1, 0}, {-1, -1}, {0, -1}, {1, -1} },
            {-1, -1}, {1, -1}
        },
        // ROTATION_90
        {
            { {0, 0}, {0, -1}, {0, -2}, {-1, -2} },
            {-1, -2}, {0, -2}
        },
        // ROTATION_180
        {
            { {-1, -1}, {0, -1}, {1, -1}, {1, -2} },
            {-1, -1}, {1, -2}
        },
        // ROTATION_270
        {
            { {0, 0}, {1, 0}, {0, -1}, {0, -2} },
            {0, -2}, {1, 0}
        }
    },
    // L
    {
        // ROTATION_0
        {
            { {1, 0}, {1, -1}, {0, -1}, {-1, -1} },
            {-1, -1}, {1, -1}
        },
        // ROTATION_90
        {
            { {-1, 0}, {0, 0}, {0, -1}, {0, -2} },
            {-1, 0}, {0, -2}
        },
        // ROTATION_180
        {
            { {-1, -1}, {0, -1}, {1, -1}, {-1, -2} },
            {-1, -2}, {1, -1}
        },
        // ROTATION_270
        {
            { {0, 0}, {0, -1}, {0, -2}, {1, -2} },
            {0, -2}, {1, -2}
        }
    },
    // O
    {
        // ROTATION_0
        {
            { {0, 0}, {1, 0}, {0, -1}, {1, -1} },
            {0, -1}, {1, -1}
        },
        // ROTATION_90
        {
            { {0, 0}, {1, 0}, {0, -1}, {1, -1} },
            {0, -1}, {1, -1}
        },
        // ROTATION_180
        {
            { {0, 0}, {1, 0}, {0, -1}, {1, -1} },
            {0, -1}, {1, -1}
        },
        // ROTATION_270
        {
            { {0, 0}, {1, 0}, {0, -1}, {1, -1} },
            {0, -1}, {1, -1}
        }
    },
    // S
    {
        // ROTATION_0
        {
            { {0, -1}, {1, -1}, {1, 0}, {2, 0} },
            {0, -1}, {2, 0}
        },
        // ROTATION_90
        {
            { {0, 0}, {0, -1}, {1, -1}, {1, -2} },
            {0, -1}, {1, -2}
        },
        // ROTATION_180
        {
            { {0, -1}, {1, -1}, {1, 0}, {2, 0} },
            {0, -1}, {2, 0}
        },
        // ROTATION_270
        {
            { {0, 0}, {0, -1}, {1, -1}, {1, -2} },
            {0, -1}, {1, -2}
        }
    },
    // T
    {
        // ROTATION_0
        {
            { {1, 0}, {0, -1}, {1, -1}, {2, -1} },
            {0, -1}, {2, -1}
        },
        // ROTATION_90
        {
            { {1, 0}, {1, -1}, {0, -1}, {1, -2} },
            {0, -1}, {1, -2}
        },
        // ROTATION_180
        {
            { {0, 0}, {1, 0}, {2, 0}, {1, -1} },
            {0, 0}, {2, 0}
        },
        // ROTATION_270
        {
            { {0, 0}, {0, -1}, {0, -2}, {1, -1} },
            {0, -2}, {1, -1}
        }
    },
    // Z
    {
        // ROTATION_0
        {
            { {0, 0}, {1, 0}, {1, -1}, {2, -1} },
            {0, 0}, {2, -1}
        },
        // ROTATION_90
        {
            { {1, 0}, {1, -1}, {0, -1}, {0, -2} },
            {0, -2}, {1, -1}
        },
        // ROTATION_180
        {
            { {0, 0}, {1, 0}, {1, -1}, {2, -1} },
            {0, 0}, {2, -1}
        },
        // ROTATION_270
        {
            { {1, 0}, {1, -1}, {0, -1}, {0, -2} },
            {0, -2}, {1, -1}
        }
    }
};

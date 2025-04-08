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

#define PIN_BUTTON_LEFT 41
#define PIN_BUTTON_RIGHT 37
#define PIN_BUTTON_ROTATE 35

#define BIT_BUTTON_LEFT (1 << Joystick::Button::BUTTON_LEFT)
#define BIT_BUTTON_RIGHT (1 << Joystick::Button::BUTTON_RIGHT)
#define BIT_BUTTON_ROTATE (1 << Joystick::Button::BUTTON_ROTATE)
#define BITS_ALL_BUTTONS (BIT_BUTTON_LEFT | BIT_BUTTON_RIGHT | BIT_BUTTON_ROTATE)

class Joystick {
public:
    enum Button {
        BUTTON_NONE = 0,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_ROTATE,
        BUTTON_COUNT
    };

    explicit Joystick();
    virtual ~Joystick();

    bool begin();
    Button waitMove(uint32_t timeout = 0);

    void enable() { m_bEnabled = true; }
    void disable() { m_bEnabled = false; }

private:
    EventGroupHandle_t m_xPressed;
    bool m_bEnabled;

    void notifyButtonPressed(EventBits_t button);
    static void readButtonTask(void* pvParameters);
};

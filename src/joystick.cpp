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
#include <Arduino.h>

#include "joystick.h"

typedef struct {
    Joystick* pJoystick;
    uint8_t pin;
    EventBits_t button;
} ButtonData;

Joystick::Joystick()
: m_bEnabled(false) {
}

Joystick::~Joystick() {
}

bool Joystick::begin() {
    pinMode(PIN_BUTTON_LEFT, INPUT);
    pinMode(PIN_BUTTON_RIGHT, INPUT);
    pinMode(PIN_BUTTON_ROTATE, INPUT);

    m_xPressed = xEventGroupCreate();

    if (m_xPressed == NULL) {
        return false;
    }

    static ButtonData buttonData[] = {
        {this, PIN_BUTTON_LEFT, BIT_BUTTON_LEFT},
        {this, PIN_BUTTON_RIGHT, BIT_BUTTON_RIGHT},
        {this, PIN_BUTTON_ROTATE, BIT_BUTTON_ROTATE}
    };

    if (xTaskCreate(readButtonTask, "Button Task LEFT", 2048, &buttonData[0], 1, NULL) != pdPASS) {
        return false;
    }

    if (xTaskCreate(readButtonTask, "Button Task RIGHT", 2048, &buttonData[1], 1, NULL) != pdPASS) {
        return false;
    }

    if (xTaskCreate(readButtonTask, "Button Task ROTATE", 2048, &buttonData[2], 1, NULL) != pdPASS) {
        return false;
    }

    return true;
}

void Joystick::readButtonTask(void* pvParameters) {
    unsigned long debounceDelay = 20;
    unsigned long lastDebounceTime = 0;
    int buttonState;
    int lastButtonState = HIGH;
    ButtonData* pData = (ButtonData*)pvParameters;

    for (;;) {
        int reading = digitalRead(pData->pin);

        if (reading != lastButtonState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (reading != buttonState) {
                buttonState = reading;

                if (buttonState == HIGH) {
                    pData->pJoystick->notifyButtonPressed(pData->button);
                }
            }
        }

        lastButtonState = reading;

        delay(10);
    }

    vTaskDelete(NULL);
}

void Joystick::notifyButtonPressed(EventBits_t button) {
    if (!m_bEnabled) {
        return;
    }

    xEventGroupSetBits(m_xPressed, button);
}

Joystick::Button Joystick::waitMove(uint32_t timeout) {
    Button button = BUTTON_NONE;
    TickType_t xTicksToWait = timeout / portTICK_PERIOD_MS;

    EventBits_t uxBits = xEventGroupWaitBits(m_xPressed, BITS_ALL_BUTTONS, pdTRUE, pdFALSE, xTicksToWait);

    if (uxBits & BIT_BUTTON_LEFT) {
        button = BUTTON_LEFT;
    } else if (uxBits & BIT_BUTTON_RIGHT) {
        button = BUTTON_RIGHT;
    } else if (uxBits & BIT_BUTTON_ROTATE) {
        button = BUTTON_ROTATE;
    }

    return button;
}

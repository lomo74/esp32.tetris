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
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "game.h"
#include "joystick.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

#define I2C_SDA 7
#define I2C_SCL 6

#define OLED_RESET -1

Joystick joystick;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Game game(&joystick, &display);

void setup() {
  Serial.begin(115200);

  // setup I2C pins
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();

  // setup OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 initialization failed!"));
    for (;;);
  }

  display.setRotation(3);
  display.setTextColor(SSD1306_WHITE);

  // setup joystick
  if (!joystick.begin()) {
    Serial.println(F("Joystick initialization failed!"));
    for (;;);
  }

  // initiaze game
  if (!game.begin()) {
    Serial.println(F("Game initialization failed!"));
    for (;;);
  }

  Serial.println(F("Game initialized successfully!"));
}

void loop() {
  // let's play Tetris!
  game.waitCoins();
  game.playMatch();
  game.over();
}

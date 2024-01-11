// Copyright (c) 2023 -	Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include <Esp.h>  // ESP.restart()
#include "FluidNCModel.h"
#include "FileParser.h"
#include "Scene.h"

HardwareSerial Serial_FNC(1);  // Serial port for comm with FNC

extern "C" int milliseconds() {
    return millis();
}
extern "C" void fnc_putchar(uint8_t c) {
    Serial_FNC.write(c);
}

extern "C" int fnc_getchar() {
    if (Serial_FNC.available()) {
        update_rx_time();
        int c = Serial_FNC.read();
        log_write(c);  // echo
        return c;
    }
    return -1;
}

void drawSplashScreen() {
    display.clear();
    display.fillScreen(BLACK);
    display.drawPngFile(LittleFS, "/fluid_dial.png", 0, 0, display.width(), display.height(), 0, 0, 0.0f, 0.0f, datum_t::middle_center);
}

void setup() {
    init_system();

    pinMode(GPIO_NUM_46, OUTPUT);
    digitalWrite(GPIO_NUM_46, 1);

    greenButton.init(GREEN_BUTTON_PIN, true);
    redButton.init(RED_BUTTON_PIN, true);
    dialButton.init(DIAL_BUTTON_PIN, true);

    Serial_FNC.begin(115200, SERIAL_8N1, FNC_RX_PIN, FNC_TX_PIN);

    drawSplashScreen();
    delay(3000);  // view the logo and wait for the debug port to connect

    log_println("FluidNC Pendant v0.3");

    speaker.setVolume(255);

    errorExpire = millis();

    extern Scene* initMenus();
    activate_scene(initMenus());
    init_listener();
    init_file_list();
}

void loop() {
    dispatch_events();

    while (debugPort.available()) {
        char c = debugPort.read();
        if (c == 'R' || c == 'r') {
            ESP.restart();
            while (1) {}
        }
    }

    while (Serial_FNC.available()) {
        fnc_poll();  // Handle messages from FluidNC
    }
}

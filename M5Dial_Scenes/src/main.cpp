// Copyright (c) 2023 -	Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include <Esp.h>  // ESP.restart()
#include <EEPROM.h>
#include "alarm.h"
#include "FluidNCModel.h"
#include "FileParser.h"
#include "Scene.h"
#include "Menu.h"

constexpr static const int RED_BUTTON_PIN   = GPIO_NUM_13;
constexpr static const int GREEN_BUTTON_PIN = GPIO_NUM_15;
constexpr static const int DIAL_BUTTON_PIN  = GPIO_NUM_42;
constexpr static const int UPDATE_RATE_MS   = 30;  // minimum refresh rate in milliseconds

// hardware
HardwareSerial Serial_FNC(1);  // Serial port for comm with FNC

void drawSplashScreen() {
    display.clear();
    display.fillScreen(BLACK);
    display.drawPngFile(LittleFS, "/fluid_dial.png", 0, 0, display.width(), display.height(), 0, 0, 0.0f, 0.0f, datum_t::middle_center);
}

void DRO::draw(int axis, bool highlight) {
    Stripe::draw(axisNumToString(axis), floatToString(myAxes[axis], 2), highlight, myLimitSwitches[axis] ? GREEN : WHITE);
}

extern "C" void show_error(int error) {
    errorExpire = millis() + 1000;
    lastError   = error;
    current_scene->reDisplay();
}

extern "C" void show_state(const char* state_string) {
    static state_t old_state = Idle;
    decode_state_string(state_string);
    if (state != old_state) {
        current_scene->onStateChange(state);
        old_state = state;
    }
}

extern "C" void end_status_report() {
    current_scene->onDROChange();
}

extern "C" void show_alarm(int alarm) {
    lastAlarm = alarm;
    current_scene->reDisplay();
}

extern "C" void show_gcode_modes(struct gcode_modes* modes) {
    myModeString = String(modes->wcs);
    myModeString += "|" + String(modes->units);
    myModeString += "|" + String(modes->distance);
    myModeString += "|" + String(modes->spindle);
    myModeString += "|" + String(modes->coolant);
    myModeString += "|T" + String(modes->tool);
    current_scene->reDisplay();
}

extern "C" int fnc_getchar() {
    if (Serial_FNC.available()) {
        int c = Serial_FNC.read();
        log_write(c);  // echo
        return c;
    }
    return -1;
}
extern "C" void fnc_putchar(uint8_t c) {
    Serial_FNC.write(c);
}
extern "C" int milliseconds() {
    return millis();
}

extern Scene mainScene;

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    log_print("Listing directory: ");
    log_println(dirname);

    File root = fs.open(dirname);
    if (!root) {
        log_println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        log_println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            log_print("  DIR : ");
            log_println(file.name());

            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            log_print("  FILE: ");
            log_print(file.name());
            log_print("  SIZE: ");

            log_println(String(file.size()));
        }
        file = root.openNextFile();
    }
}

void setup() {
    init_system();

    greenButton.init(GREEN_BUTTON_PIN, true);
    redButton.init(RED_BUTTON_PIN, true);
    dialButton.init(DIAL_BUTTON_PIN, true);

    Serial_FNC.begin(115200, SERIAL_8N1, 1, 2);  // assign pins to the M5Stamp Port B

    drawSplashScreen();
    delay(3000);  // view the logo and wait for the debug port to connect

    log_println("FluidNC Pendant v0.3");

    log_println("\r\nFluidNC Pendant Begin");
    fnc_realtime(StatusReport);  // Request fresh status
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

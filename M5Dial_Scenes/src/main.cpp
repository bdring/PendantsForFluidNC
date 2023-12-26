// Copyright (c) 2023 -	Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

/*  TO DO
    Save prefs to flash
*/

#include <Arduino.h>
#include <Esp.h>  // ESP.restart()
#include <EEPROM.h>
#include "alarm.h"
#include "FluidNCModel.h"
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
    display.fillScreen(WHITE);
    display.drawPngFile(LittleFS, "/fnc_logo.png", 0, 0, display.width(), display.height(), 0, 0, 0.0f, 0.0f, datum_t::middle_center);
    centered_text("Fluid Dial", 36, BLACK, SMALL);
    centered_text("Pendant", 65, BLACK, SMALL);
    centered_text("B. Dring", 190, BLACK, SMALL);
}

void drawErrorScreen(const String& s) {
    display.clear();
    display.fillScreen(RED);
    text("Error " + s, display.height() / 2, WHITE, LARGE);
}

void DRO::draw(int axis, bool highlight) {
    Stripe::draw(axisNumToString(axis), floatToString(myAxes[axis], 2), highlight, myLimitSwitches[axis] ? GREEN : WHITE);
}

extern "C" void show_error(int error) {
    drawErrorScreen(String(error));
    speaker.tone(3000, 1000);
    delay(1000);
    current_scene->reDisplay();
}

extern "C" void show_state(const char* state_string) {
    decode_state_string(state_string);
    current_scene->onStateChange(state);
}

extern "C" void end_status_report() {
    current_scene->reDisplay();
}

extern "C" void show_alarm(int alarm) {
    lastAlarm = alarm;
    current_scene->reDisplay();
}

extern "C" int fnc_getchar() {
    if (Serial_FNC.available()) {
        int c = Serial_FNC.read();
        debugPort.write(c);  // echo
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

void printTime(time_t t) {
    struct tm* tmstruct = localtime(&t);
    debugPort.printf("  at %d-%02d-%02d %02d:%02d:%02d\n",
                     (tmstruct->tm_year) + 1900,
                     (tmstruct->tm_mon) + 1,
                     tmstruct->tm_mday,
                     tmstruct->tm_hour,
                     tmstruct->tm_min,
                     tmstruct->tm_sec);
}

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    debugPort.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        debugPort.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        debugPort.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            debugPort.print("  DIR : ");

            debugPort.print(file.name());
            printTime(file.getLastWrite());

            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            debugPort.print("  FILE: ");
            debugPort.print(file.name());
            debugPort.print("  SIZE: ");

            debugPort.print(file.size());
            printTime(file.getLastWrite());
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

    listDir(LittleFS, "/", 0);
    log_msg("FluidNC Pendant v0.3");

    debugPort.println("\r\nFluidNC Pendant Begin");
    fnc_realtime(StatusReport);  // Request fresh status
    speaker.setVolume(255);

    extern Scene* initMenus();
    activate_scene(initMenus());
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

// Copyright (c) 2023 -	Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

/*  TO DO
General
  Save prefs to flash

Main Screen

Home Screen

Probe Screen

Jog Screen

Saved
    int jog_inc_level[3]
    jog_rate_level[3]
*/

#include <Arduino.h>
#include <LittleFS.h>
#include <Esp.h>  // ESP.restart()
#include <EEPROM.h>
#include "alarm.h"
#include "FluidNCModel.h"
#include "Scene.h"

#define FORMAT_LITTLEFS_IF_FAILED true

constexpr static const int RED_BUTTON_PIN   = GPIO_NUM_13;
constexpr static const int GREEN_BUTTON_PIN = GPIO_NUM_15;
constexpr static const int DIAL_BUTTON_PIN  = GPIO_NUM_42;
constexpr static const int UPDATE_RATE_MS   = 30;  // minimum refresh rate in milliseconds

// hardware
HardwareSerial Serial_FNC(1);  // Serial port for comm with FNC

void drawSplashScreen() {
    M5Dial.Display.clear();
    M5Dial.Display.fillScreen(WHITE);
    M5Dial.Display.drawPngFile(LittleFS, "/bitmap.png", 0, 70);

    centered_text("Fluid Dial", 36, BLACK, SMALL);
    centered_text("Pendant", 65, BLACK, SMALL);
    centered_text("B. Dring", 190, BLACK, SMALL);
}

void drawErrorScreen(const String& s) {
    M5Dial.Display.clear();
    M5Dial.Display.fillScreen(RED);
    text("Error " + s, VERTICAL_CENTER, WHITE, LARGE);
}

void DRO::draw(int axis, bool highlight) {
    Stripe::draw(axisNumToString(axis), floatToString(myAxes[axis], 2), highlight, myLimitSwitches[axis] ? GREEN : WHITE);
}

void savePrefs() {
    // EEPROM.put(0, myPrefs);
    // log_msg("put prefs");
}

void readPrefs() {
    // if (EEPROM.get(0, eeprom_ver) != 2) {
    //     // ver wrong, so save a default set
    //     savePrefs();
    //     return;
    // }

    // EEPROM.get(0, myPrefs);
    // log_msg("get prefs");
}

extern "C" void show_error(int error) {
    drawErrorScreen(String(error));
    M5Dial.Speaker.tone(3000, 1000);
    delay(1000);
    current_scene->display();
}

extern "C" void show_state(const char* state_string) {
    decode_state_string(state_string);
    current_scene->onStateChange(state);
}

extern "C" void end_status_report() {
    current_scene->display();
}

extern "C" void show_alarm(int alarm) {
    lastAlarm = alarm;
    current_scene->display();
}

extern "C" int fnc_getchar() {
    if (Serial_FNC.available()) {
        int c = Serial_FNC.read();
        USBSerial.write(c);  // echo
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
    USBSerial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        USBSerial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        USBSerial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            USBSerial.print("  DIR : ");

            USBSerial.print(file.name());
            time_t     t        = file.getLastWrite();
            struct tm* tmstruct = localtime(&t);
            USBSerial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                             (tmstruct->tm_year) + 1900,
                             (tmstruct->tm_mon) + 1,
                             tmstruct->tm_mday,
                             tmstruct->tm_hour,
                             tmstruct->tm_min,
                             tmstruct->tm_sec);

            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            USBSerial.print("  FILE: ");
            USBSerial.print(file.name());
            USBSerial.print("  SIZE: ");

            USBSerial.print(file.size());
            time_t     t        = file.getLastWrite();
            struct tm* tmstruct = localtime(&t);
            USBSerial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                             (tmstruct->tm_year) + 1900,
                             (tmstruct->tm_mon) + 1,
                             tmstruct->tm_mday,
                             tmstruct->tm_hour,
                             tmstruct->tm_min,
                             tmstruct->tm_sec);
        }
        file = root.openNextFile();
    }
}

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    greenButton.init(GREEN_BUTTON_PIN, true);
    redButton.init(RED_BUTTON_PIN, true);
    dialButton.init(DIAL_BUTTON_PIN, true);

    USBSerial.begin(921600);
    Serial_FNC.begin(115200, SERIAL_8N1, 1, 2);  // assign pins to the M5Stamp Port B

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        USBSerial.println("LittleFS Mount Failed");
        return;
    }
    USBSerial.println("LittleFS Mount Succeeded");
    listDir(LittleFS, "/", 0);

    drawSplashScreen();
    delay(3000);  // view the logo and wait for the USBSerial to be detected by the PC

    log_msg("M5Dial Pendant v0.2");

    USBSerial.println("\r\nM5Dial Pendant Begin");
    fnc_realtime(StatusReport);  // Request fresh status
    M5Dial.Speaker.setVolume(255);

    readPrefs();

    activate_scene(&mainScene);
}

void loop() {
    dispatch_events();

    while (USBSerial.available()) {
        char c = USBSerial.read();
        if (c == 'R' || c == 'r') {
            ESP.restart();
            while (1) {}
        }
    }

    while (Serial_FNC.available()) {
        fnc_poll();  // Handle messages from FluidNC
    }
}

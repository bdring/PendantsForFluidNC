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
#include "logo_img.h"
#include <EEPROM.h>
#include "alarm.h"
#include "FluidNCModel.h"
#include "Scene.h"

constexpr static const int RED_BUTTON_PIN   = GPIO_NUM_13;
constexpr static const int GREEN_BUTTON_PIN = GPIO_NUM_15;
constexpr static const int DIAL_BUTTON_PIN  = GPIO_NUM_42;
constexpr static const int UPDATE_RATE_MS   = 30;  // minimum refresh rate in milliseconds

// hardware
HardwareSerial Serial_FNC(1);  // Serial port for comm with FNC

void drawSplashScreen() {
    M5Dial.Display.clear();
    M5Dial.Display.fillScreen(WHITE);
    M5Dial.Display.pushImage(0, 70, WIDTH, 100, logo_img);

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

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    greenButton.init(GREEN_BUTTON_PIN, true);
    redButton.init(RED_BUTTON_PIN, true);
    dialButton.init(DIAL_BUTTON_PIN, true);

    USBSerial.begin(921600);
    Serial_FNC.begin(115200, SERIAL_8N1, 1, 2);  // assign pins to the M5Stamp Port B

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

    while (Serial_FNC.available()) {
        fnc_poll();  // Handle messages from FluidNC
    }
}

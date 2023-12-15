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
#include "Scene.h"

constexpr static const int RED_BUTTON_PIN   = GPIO_NUM_13;
constexpr static const int GREEN_BUTTON_PIN = GPIO_NUM_15;
constexpr static const int DIAL_BUTTON_PIN  = GPIO_NUM_42;
constexpr static const int UPDATE_RATE_MS   = 30;  // minimum refresh rate in milliseconds

M5Canvas canvas(&M5Dial.Display);

// local copies of status items
String             stateString        = "N/C";
state_t            state              = Idle;
pos_t              myAxes[6]          = { 0 };
bool               myLimitSwitches[6] = { false };
bool               myProbeSwitch      = false;
String             myFile             = "";   // running SD filename
file_percent_t     myPercent          = 0.0;  // percent conplete of SD file
override_percent_t myFro              = 100;  // Feed rate override
int                lastAlarm          = 0;

// hardware
HardwareSerial Serial_FNC(1);  // Serial port for comm with FNC

void drawStatus() {
    int rect_color;

    switch (state) {
        case Idle:
            rect_color = WHITE;
            break;
        case Alarm:
            rect_color = RED;
            break;
        case Cycle:
            rect_color = GREEN;
            break;
        case Jog:
            rect_color = CYAN;
            break;
        case Homing:
            rect_color = CYAN;
            break;
        case Hold:
            rect_color = YELLOW;
            break;
        default:
            rect_color = WHITE;
            break;
    }

    static constexpr int x      = 100;
    static constexpr int y      = 24;
    static constexpr int width  = 140;
    static constexpr int height = 36;

    canvas.fillRoundRect(120 - width / 2, y, width, height, 5, rect_color);
    canvas.setTextColor(BLACK);
    canvas.setTextDatum(middle_center);
    if (state == Alarm) {
        canvas.setFont(&fonts::FreeSansBold12pt7b);
        canvas.drawString(stateString, 120, y + height / 2 - 4);
        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.drawString(alarm_name[lastAlarm], 120, y + height / 2 + 12);
    } else {
        canvas.setFont(&fonts::FreeSansBold18pt7b);
        canvas.drawString(stateString, 120, y + height / 2 + 3);
    }
}

void drawDRO(int x, int y, int width, int axis, float value, bool highlighted) {
    int color_value, color_hightlight;
    canvas.setFont(&fonts::FreeMonoBold18pt7b);

    static constexpr int height = 32;

    color_value = WHITE;
    if (highlighted) {
        color_hightlight = BLUE;
    } else {
        color_hightlight = NAVY;
    }

    canvas.setTextColor(WHITE);

    canvas.fillRoundRect(x, y, width, height, 5, color_hightlight);
    canvas.drawRoundRect(x, y, width, height, 5, color_value);

    canvas.setTextColor((myLimitSwitches[axis]) ? GREEN : WHITE);
    canvas.setTextDatum(middle_left);
    canvas.drawString(axisNumToString(axis), x + 5, y + height / 2 + 2);

    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_right);
    canvas.drawString(floatToString(value, 2), x + width - 5, y + height / 2 + 2);
}

void drawButton(int x, int y, int width, int height, int charSize, String text, bool highlighted) {
    int color_value, color_hightlight;

    switch (charSize) {
        case 9:
            canvas.setFont(&fonts::FreeSansBold9pt7b);
            break;
        case 12:
            canvas.setFont(&fonts::FreeSansBold12pt7b);
            break;
        case 18:
            canvas.setFont(&fonts::FreeSansBold18pt7b);
            break;
        default:
            canvas.setFont(&fonts::FreeSansBold24pt7b);
            break;
    }

    color_value = WHITE;
    if (highlighted) {
        color_hightlight = BLUE;
    } else {
        color_hightlight = NAVY;
    }
    canvas.fillRoundRect(x, y, width, height, 5, color_hightlight);
    canvas.drawRoundRect(x, y, width, height, 5, color_value);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_center);
    canvas.drawString(text, x + width / 2, y + height / 2 + 2);
}

// This shows on the display what the button currently do.
void buttonLegends(String red, String green, String orange) {
    canvas.setFont(&fonts::FreeSansBold9pt7b);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(RED);
    canvas.drawString(red, 80, 212);
    canvas.setTextColor(GREEN);
    canvas.drawString(green, 160, 212);
    canvas.setTextColor(ORANGE);
    canvas.drawString(orange, 120, 228);
}

void drawLed(int x, int y, int radius, bool active) {
    canvas.fillCircle(x, y, radius, (active) ? GREEN : DARKGREY);
    canvas.drawCircle(x, y, radius, WHITE);
}

void drawStartScreen() {
    M5Dial.Display.clear();
    M5Dial.Display.fillScreen(WHITE);
    M5Dial.Display.pushImage(0, 70, 240, 100, logo_img);
    M5Dial.Display.setFont(&fonts::FreeSansBold12pt7b);
    M5Dial.Display.setTextColor(BLACK);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.drawString("Fluid Dial", 120, 36);
    M5Dial.Display.drawString("Pendant", 120, 65);
    M5Dial.Display.drawString("B. Dring", 120, 190);
}

void menuTitle() {
    canvas.setFont(&fonts::FreeSansBold9pt7b);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(WHITE);
    canvas.drawString(current_scene->name(), 120, 12);
}

void drawErrorScreen(const String& s) {
    M5Dial.Display.clear();
    M5Dial.Display.fillScreen(RED);
    M5Dial.Display.setFont(&fonts::FreeSansBold24pt7b);
    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.drawString("Error", 90, 120);
    M5Dial.Display.drawString(s, 180, 120);
}

void refreshDisplaySprite() {
    M5Dial.Display.startWrite();
    canvas.pushSprite(0, 0);
    M5Dial.Display.endWrite();
}

void feedRateRotator(int& rate, bool up) {
    if (up) {
        if (rate < 10) {
            rate += 1;
        } else if (rate < 100) {
            rate += 10;
        } else if (rate < 1000) {
            rate += 100;
        } else {
            rate += 1000;
        }
    } else {
        if (rate > 1000) {
            rate -= 1000;
        } else if (rate > 100) {
            rate -= 100;
        } else if (rate > 10) {
            rate -= 10;
        } else if (rate > 2) {
            rate -= 1;
        }
    }
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

extern "C" void show_alarm(int alarm) {
    lastAlarm = alarm;
    current_scene->display();
}
extern "C" void show_overrides(override_percent_t feed, override_percent_t rapid, override_percent_t spindle) {
    myFro = feed;
}
extern "C" void show_state(const char* state_string) {
    if (stateString != state_string) {
        stateString = state_string;
        if (stateString.startsWith("Hold")) {
            state = Hold;
        } else if (stateString == "Run") {
            state = Cycle;
        } else if (stateString == "Alarm") {
            state = Alarm;
        } else if (stateString == "Idle") {
            state = Idle;
        } else if (stateString == "Jog") {
            state = Jog;
        } else if (stateString == "Home") {
            state = Homing;
        } else if (stateString.startsWith("Door")) {
            state = SafetyDoor;
        } else if (stateString == "Check") {
            state = CheckMode;
        } else if (stateString == "Sleep") {
            state = Sleep;
        }
        current_scene->onStateChange(state);
    }
}

extern "C" void end_status_report() {
    current_scene->display();
}

extern "C" void show_file(const char* filename, file_percent_t percent) {
    myPercent = percent;
}

extern "C" void show_limits(bool probe, const bool* limits, size_t n_axis) {
    myProbeSwitch = probe;
    memcpy(myLimitSwitches, limits, n_axis * sizeof(*limits));
}
extern "C" void show_dro(const pos_t* axes, const pos_t* wco, bool isMpos, bool* limits, size_t n_axis) {
    for (int axis = 0; axis < n_axis; axis++) {
        myAxes[axis] = axes[axis];
        if (isMpos) {
            myAxes[axis] -= wco[axis];
        }
    }
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

    USBSerial.begin(115200);
    Serial_FNC.begin(115200, SERIAL_8N1, 1, 2);  // assign pins to the M5Stamp Port B

    drawStartScreen();
    delay(3000);  // view the logo and wait for the USBSerial to be detected by the PC

    log_msg("M5Dial Pendant v0.2");

    USBSerial.println("\r\nM5Dial Pendant Begin");
    fnc_realtime(StatusReport);  // Request fresh status
    M5Dial.Speaker.setVolume(255);

    readPrefs();

    activate_scene(&mainScene);
}

void loop() {
    static uint32_t last_time = millis();  // controls the framerate
    dispatch_events();

    while (Serial_FNC.available()) {
        fnc_poll();  // do the serial port reading and echoing
    }

    while (millis() - last_time < UPDATE_RATE_MS) {}
}

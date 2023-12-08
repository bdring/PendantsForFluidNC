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
#include "M5Dial.h"
#include "logo_img.h"
#include "GrblParserC.h"
#include "Button.h"
#include <EEPROM.h>

constexpr static const int RED_BUTTON_PIN   = GPIO_NUM_13;
constexpr static const int GREEN_BUTTON_PIN = GPIO_NUM_15;
constexpr static const int UPDATE_RATE_MS   = 30;  // minimum refresh rate in milliseconds
constexpr static const int MAX_JOG_INC      = 5;

#define DEBUG_TO_FNC
#define DEBUG_TO_USB

enum class MenuName : uint8_t {
    Main    = 0,
    Homing  = 1,
    Jogging = 2,
    Probing = 3,
};

String menu_names[] = { "Main", "Home", "Jog Dial", "Probe" };  // As shown on display

// local copies of status items
String         stateString        = "N/C";
pos_t          myAxes[6]          = { 0 };
bool           myLimitSwitches[6] = { false };
bool           myProbeSwitch      = false;
String         myFile             = "";     // running SD filename
file_percent_t myPercent          = 0.0;    // percent conplete of SD file
float          myFro              = 100.0;  // Feed rate override
int            lastAlarm          = 0;

MenuName menu_number = MenuName::Main;  // The menu that is currently active
MenuName last_menu   = MenuName::Main;

// hardware
static m5::touch_state_t prev_state;
HardwareSerial           Serial_FNC(1);  // Serial port for comm with FNC
Button                   greenButton;
Button                   redButton;

// The menus
void main_menu(int32_t delta);
void homingMenu();
void joggingMenu(int32_t delta);
void probeMenu();

// draw stuff
void drawStartScreen();
void drawDRO(int x, int y, int width, int axis, float value, bool highlighted);
void drawStatus();
void drawButton(int x, int y, int width, int height, int charSize, String text, bool highlighted);
void drawLed(int x, int y, int radius, bool active);
void buttonLegends(String red, String green, String orange);
void menuTitle();
void refreshDisplaySprite();

// helper functions
void   rotateNumberLoop(int& currentVal, int increment, int min, int max);
void   feedRateRotator(int& rate, bool up);
bool   dial_button();
String M5TouchStateName(m5::touch_state_t state_num);
String floatToString(float val, int afterDecimal);
String axisNumToString(int axis);
void   debug(const char* info);
String AlarmNames(int num);
void   savePrefs();
void   readPrefs();

int jog_axis = 0;  // the axis currently being jogged

// saved
// int   jog_inc_level[3]  = { 2, 2, 1 };  // exponent 0=0.01, 2=0.1 ... 5 = 100.00
// int   jog_rate_level[3] = { 1000, 1000, 100 };
// int   jog_cont_speed[3] = { 1000, 1000, 1000 };
// float probe_offset      = 10.0;
// float probe_travel      = -20.0;
// float probe_rate        = 60.0;
// float probe_retract     = 20.0;
int probe_encoder = 0;
// int   probe_axis        = 2;  // Z is default

struct prefs {
    int8_t eeprom_ver        = 2;
    int    jog_inc_level[3]  = { 2, 2, 1 };  // exponent 0=0.01, 2=0.1 ... 5 = 100.00
    int    jog_rate_level[3] = { 1000, 1000, 100 };
    int    jog_cont_speed[3] = { 1000, 1000, 1000 };
    float  probe_offset      = 0.0;
    float  probe_travel      = -20.0;
    float  probe_rate        = 80.0;
    float  probe_retract     = 20.0;
    int    probe_axis        = 2;  // Z is default
} myPrefs;
bool prefsChanged = false;

extern "C" void show_alarm(int alarm) {
    lastAlarm = alarm;
}
extern "C" void show_state(const char* state) {
    stateString = state;
    if (stateString.startsWith("Hold")) {
        stateString = "Hold";
    }
}

extern "C" void show_file(const char* filename, file_percent_t percent) {
    myPercent = percent;
}

extern "C" void show_limits(bool probe, const bool* limits, size_t n_axis) {
    myProbeSwitch = probe;
    memcpy(myLimitSwitches, limits, sizeof(limits));
}
extern "C" void show_dro(const pos_t* axes, const pos_t* wco, bool isMpos, bool* limits, size_t n_axis) {
    char delim = ' ';
    for (int axis = 0; axis < n_axis; axis++) {
        myAxes[axis] = axes[axis];
        if (isMpos) {
            myAxes[axis] -= wco[axis];
        }
    }
}

extern "C" void end_status_report() {}
extern "C" int  fnc_getchar() {
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

M5Canvas canvas(&M5Dial.Display);

void send_line(const String& s, int timeout = 2000) {
    send_line(s.c_str(), timeout);
}
void send_line(const char* s, int timeout = 2000) {
    fnc_send_line(s, timeout);
}

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    greenButton.init(GREEN_BUTTON_PIN, true);
    redButton.init(RED_BUTTON_PIN, true);

    USBSerial.begin(115200);
    Serial_FNC.begin(115200, SERIAL_8N1, 1, 2);  // assign pins to the M5Stamp Port B

    drawStartScreen();
    delay(3000);  // view the logo and wait for the USBSerial to be detected by the PC

    send_line("$Log/Msg=*M5Dial Pendant v0.1");
    USBSerial.println("\r\nM5Dial Pendant Begin");
    fnc_realtime(StatusReport);  // Request fresh status
    M5Dial.Speaker.setVolume(255);

    readPrefs();

    menu_number = MenuName::Main;
    last_menu   = menu_number;
}

void loop() {
    static uint32_t last_time     = millis();  // used to control the framerate of this loop
    static int32_t  oldEncoderPos = 0;

    M5Dial.update();
    int32_t newEncoderPos = M5Dial.Encoder.read();
    int32_t encoderDelta  = newEncoderPos - oldEncoderPos;
    oldEncoderPos         = newEncoderPos;

    if (last_menu != menu_number) {
        if (prefsChanged) {
            savePrefs();
        }
        if (menu_number == MenuName::Probing) {
            M5Dial.Encoder.write(probe_encoder);
        } else if (menu_number == MenuName::Jogging) {
        }
        last_menu = menu_number;
    }
    while (Serial_FNC.available()) {
        fnc_poll();  // do the serial port reading and echoing
    }

    switch (menu_number) {
        case MenuName::Main:
            main_menu(encoderDelta);
            break;
        case MenuName::Homing:
            homingMenu();
            break;
        case MenuName::Jogging:
            joggingMenu(encoderDelta);
            break;
        case MenuName::Probing:
            probeMenu();
            break;
        default:
            USBSerial.println("Unknown menu");
            break;
    }

    while (millis() - last_time < UPDATE_RATE_MS) {}
}

void main_menu(int32_t delta) {
    static int menu_item = 0;

    if (M5Dial.BtnA.isPressed()) {  // if jog dial buttom was press
        while (M5Dial.BtnA.isPressed())
            M5Dial.update();
        delay(20);
        //USBSerial.println("dial btn");
        if (stateString == "Idle" || stateString == "Alarm") {
            switch (menu_item) {
                case 0:
                    menu_number = MenuName::Jogging;
                    return;
                case 1:
                    menu_number = MenuName::Homing;
                    return;
                case 2:
                    menu_number = MenuName::Probing;
                    return;
            }
        } else if (stateString == "Run") {
            fnc_realtime(FeedOvrReset);
        }
    }

    auto t = M5Dial.Touch.getDetail();
    if (prev_state != t.state) {
        //USBSerial.printf("%s\r\n", M5TouchStateName(t.state));
        if (t.state == m5::touch_state_t::touch_end) {
            M5Dial.Speaker.tone(1800, 30);
            if (!stateString.equals("Run")) {
                rotateNumberLoop(menu_item, 1, 0, 2);
            }
            fnc_realtime(StatusReport);  // sometimes you want an extra status
        }
        prev_state = t.state;
    }

    if (redButton.changed()) {
        if (redButton.active()) {
            if (stateString == "Alarm") {
                send_line("$X");
            } else if (stateString == "Run" || stateString == "Home") {
                fnc_realtime(Reset);
            } else if (stateString.equals("Hold")) {
                fnc_realtime(Reset);
            } else if (stateString.equals("Idle")) {
                menu_number = MenuName::Homing;
                return;
            }
        }
    }

    if (greenButton.changed()) {
        if (greenButton.active()) {
            if (stateString == "Run") {
                fnc_realtime(FeedHold);
            } else if (stateString.equals("Hold")) {
                fnc_realtime(CycleStart);
            } else if (stateString.equals("Idle")) {
                menu_number = MenuName::Probing;
                return;
            } else if (stateString.equals("Alarm")) {
                menu_number = MenuName::Homing;
                return;
            }
            fnc_realtime(StatusReport);
        } else {
        }
    }

    canvas.createSprite(240, 240);
    canvas.fillSprite(BLACK);

    menuTitle();
    drawStatus();

    int y       = 68;
    int spacing = 33;
    drawDRO(10, y, 220, 0, myAxes[0], menu_item == 0);
    drawDRO(10, y += spacing, 220, 1, myAxes[1], menu_item == 0);
    drawDRO(10, y += spacing, 220, 2, myAxes[2], menu_item == 0);

    y = 170;
    if (stateString == "Run" || (stateString.equals("Hold"))) {
        int width = 192;
        if (myPercent > 0) {
            canvas.fillRoundRect(20, y, width, 10, 5, LIGHTGREY);
            width = (float)width * myPercent / 100.0;
            if (width > 0)
                canvas.fillRoundRect(20, y, width, 10, 5, GREEN);
        }

        // Feed override
        canvas.setTextColor(WHITE);
        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.setTextDatum(middle_center);
        canvas.drawString("Feed Rate Ovr:" + floatToString(myFro, 0) + "%", 120, y + 23);
    }

    if (!stateString.equals("Run") && !stateString.equals("Hold")) {
        drawButton(38, 170, 74, 30, 12, "Home", menu_item == 1);
        drawButton(128, 170, 74, 30, 12, "Probe", menu_item == 2);
    }

    String encoder_button_text = "";
    switch (menu_item) {
        case 0:
            encoder_button_text = "Jog";
            break;
        case 1:
            encoder_button_text = "Home";
            break;
        case 2:
            encoder_button_text = "Probe";
            break;
    }
    String redButtonText   = "";
    String greenButtonText = "";
    if (stateString == "Alarm" || stateString == "Home") {
        redButtonText   = "Reset";
        greenButtonText = "Home";
    } else if (stateString == "Run") {
        redButtonText       = "E-Stop";
        greenButtonText     = "Hold";
        encoder_button_text = "FRO End";
    } else if (stateString.equals("Hold")) {
        redButtonText   = "Quit";
        greenButtonText = "Start";
    } else if (stateString == "Jog") {
        redButtonText = "Jog Cancel";
    } else if (stateString == "Idle") {
        redButtonText   = "Home";
        greenButtonText = "Probe";
    }

    buttonLegends(redButtonText, greenButtonText, encoder_button_text);

    refreshDisplaySprite();

    if (abs(delta) > 0) {
        if (stateString == "Run") {
            if (delta > 0) {
                if (myFro < 200)
                    fnc_realtime(FeedOvrFinePlus);
            } else {
                if (myFro > 10)
                    fnc_realtime(FeedOvrFineMinus);
            }
        }
    }
}

void homingMenu() {
    static int current_button = 0;

    if (dial_button()) {  // if jog dial buttom was press
        menu_number = MenuName::Main;
        return;
    }

    if (greenButton.changed()) {
        if (greenButton.active()) {
            if (stateString == "Idle" || stateString == "Alarm") {
                if (current_button == 0) {
                    send_line("$H");
                } else {
                    send_line("$H" + axisNumToString(current_button - 1));
                }
            }
            return;
        }
    }

    if (redButton.changed()) {
        if (redButton.active()) {
            if (stateString == "Home") {
                fnc_realtime(Reset);
            }
            return;
        }
    }

    auto t = M5Dial.Touch.getDetail();
    if (prev_state != t.state) {
        if (t.state == m5::touch_state_t::touch) {
            M5Dial.Speaker.tone(1800, 20);
            rotateNumberLoop(current_button, 1, 0, 3);
        }
        USBSerial.printf("%s\r\n", M5TouchStateName(t.state));
        prev_state = t.state;
    }

    //

    canvas.fillSprite(BLACK);
    drawStatus();

    int x      = 50;
    int y      = 65;
    int gap    = 34;
    int width  = 140;
    int height = 30;
    drawButton(x, y, width, height, 12, "Home All", current_button == 0);
    drawButton(x, y += gap, width, height, 12, "Home X", current_button == 1);
    drawLed(x - 16, y + 15, 10, myLimitSwitches[0]);
    drawButton(x, y += gap, width, height, 12, "Home Y", current_button == 2);
    drawLed(x - 16, y + 15, 10, myLimitSwitches[1]);
    drawButton(x, y += gap, width, height, 12, "Home Z", current_button == 3);
    drawLed(x - 16, y + 15, 10, myLimitSwitches[2]);

    menuTitle();

    String redLabel, grnLabel, orangeLabel = "";
    orangeLabel = "Main";
    if (stateString == "Home") {
        redLabel = "E-Stop";
    } else {
        if (current_button == 0) {
            grnLabel = "Home All";
        } else {
            grnLabel = "Home " + axisNumToString(current_button - 1);
        }
    }
    buttonLegends(redLabel, grnLabel, orangeLabel);

    refreshDisplaySprite();
}

void joggingMenu(int32_t delta) {
    bool        update         = false;
    static int  active_setting = 0;  // Dist or Rate
    static int  selection      = 0;
    static int  continuous     = 0;  // 0 off 1 = pos, 2 = neg
    static bool jog_continuous = false;

    float jog_increment = pow(10.0, abs(myPrefs.jog_inc_level[jog_axis])) / 100.0;

    // Dial Button handling
    if (M5Dial.BtnA.isPressed()) {
        while (M5Dial.BtnA.isPressed())
            M5Dial.update();
        delay(20);
        menu_number = MenuName::Main;
        return;
    }

    if (greenButton.changed()) {
        if (greenButton.active()) {
            if (stateString == "Idle") {
                if (selection % 2) {
                    send_line("G10L20P0" + axisNumToString(jog_axis) + "0");
                    send_line("$Log/Msg=*G10L20P0" + axisNumToString(jog_axis) + "0");
                    return;
                }
                if (jog_continuous) {
                    send_line("$J=G91F" + floatToString(myPrefs.jog_cont_speed[jog_axis], 0) + axisNumToString(jog_axis) + "10000");
                    return;
                } else {
                    if (active_setting == 0) {
                        if (myPrefs.jog_inc_level[jog_axis] == MAX_JOG_INC) {
                        } else {
                            myPrefs.jog_inc_level[jog_axis]++;
                            return;
                        }
                    } else {
                        feedRateRotator(myPrefs.jog_rate_level[jog_axis], true);
                    }
                    prefsChanged = true;
                }
            } else if (stateString == "Jog") {
                if (!jog_continuous) {
                    fnc_realtime(JogCancel);  // reset
                }
            }
        } else {  // green button up
            if (jog_continuous) {
                fnc_realtime(JogCancel);  // reset
            }
        }
    }

    if (redButton.changed()) {
        if (redButton.active()) {
            if (stateString == "Idle") {
                if (selection % 2) {
                    return;
                }
                if (jog_continuous) {
                    // $J=G91F1000X-10000
                    send_line("$J=G91F" + floatToString(myPrefs.jog_cont_speed[jog_axis], 0) + axisNumToString(jog_axis) + "-10000");
                    return;
                } else {
                    if (active_setting == 0) {
                        if (active_setting == 0) {
                            if (myPrefs.jog_inc_level[jog_axis] > 0) {
                                myPrefs.jog_inc_level[jog_axis]--;
                            }
                        } else {
                            feedRateRotator(myPrefs.jog_rate_level[jog_axis], false);
                        }
                        prefsChanged = true;

                        update = true;

                    } else {
                        feedRateRotator(myPrefs.jog_rate_level[jog_axis], false);
                    }
                }

                update = true;
            } else if (stateString == "Jog") {
                if (!jog_continuous) {
                    fnc_realtime(Reset);
                }
            }
        } else {  // red button up
            if (jog_continuous) {
                fnc_realtime(JogCancel);  // reset
            }
        }
    }

    // A touch allows you to rotate through the axis being jogged
    auto t = M5Dial.Touch.getDetail();
    if (prev_state != t.state) {
        if (t.state == m5::touch_state_t::touch_end) {
            M5Dial.Speaker.tone(1800, 20);
            //USBSerial.printf("Touch x:%i y:%i\r\n", t.x, t.y);
            //Use dial to break out of continuous mode
            if (t.y < 70) {
                jog_continuous = !jog_continuous;
            } else if (t.y < 140) {
                rotateNumberLoop(selection, 1, 0, 5);
                jog_axis = (selection) / 2;
            } else {
                rotateNumberLoop(active_setting, 1, 0, 1);
            }
            USBSerial.printf("Selection:%d Axis:%d\r\n", selection, jog_axis);
            update = true;
            return;
        }
        prev_state = t.state;
    }

    canvas.fillSprite(BLACK);

    drawStatus();
    int x      = 9;
    int y      = 69;
    int width  = 180;
    int offset = 33;

    drawDRO(x, y, width, 0, myAxes[0], jog_axis == 0);
    drawDRO(x, y += offset, width, 1, myAxes[1], jog_axis == 1);
    drawDRO(x, y += offset, width, 2, myAxes[2], jog_axis == 2);

    int height = 32;
    x          = x + width + 1;
    y          = 69;
    width      = 42;
    drawButton(x, y, width, height, 9, "zro", selection == 1);
    drawButton(x, y += offset, width, height, 9, "zro", selection == 3);
    drawButton(x, y += offset, width, height, 9, "zro", selection == 5);

    if (jog_continuous) {
        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.setTextDatum(middle_center);
        canvas.setTextColor(WHITE);
        canvas.drawString("Jog Rate: " + floatToString(myPrefs.jog_cont_speed[jog_axis], 0), 120, 185);
    } else {
        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.setTextDatum(middle_center);
        canvas.setTextColor((active_setting == 0) ? WHITE : DARKGREY);
        canvas.drawString("Jog Dist: " + floatToString(jog_increment, 2), 120, 177);
        canvas.setTextColor((active_setting == 1) ? WHITE : DARKGREY);
        canvas.drawString("Jog Rate: " + floatToString(myPrefs.jog_rate_level[jog_axis], 2), 120, 193);
    }
    if (jog_continuous) {
        if (delta != 0) {
            feedRateRotator(myPrefs.jog_cont_speed[jog_axis], delta > 0);
        }
    } else {
        if (delta != 0) {
            // $J=G91F200Z5.0
            //$Log/Msg = *
            send_line("$Log/Msg=*Jog delta:" + String(delta));
            String jogCmd = "$J=G91F" + floatToString(myPrefs.jog_rate_level[jog_axis], 0) + axisNumToString(jog_axis);
            if (delta < 0) {
                jogCmd += "-";
            }
            jogCmd += floatToString(jog_increment, 2);
            send_line(jogCmd);
        }
    }

    canvas.setFont(&fonts::FreeSansBold9pt7b);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(WHITE);
    canvas.drawString((jog_continuous) ? "Bttn Jog" : "Knob Jog", 120, 12);

    if (stateString == "Idle") {
        if (jog_continuous) {
            if (selection % 2) {
                buttonLegends("", "Zero " + axisNumToString(jog_axis), "Main");
            } else {
                buttonLegends("Jog-", "Jog+", "Main");
            }
        } else {
            if (selection % 2) {  // if zro selected
                buttonLegends("", "Zero " + axisNumToString(jog_axis), "Main");
            } else {
                buttonLegends("Dec", "Inc", "Main");
            }
        }

    } else if (stateString == "Jog") {
        if (jog_continuous) {
            buttonLegends("Jog-", "Jog+", "Main");
        } else {
            buttonLegends("E-Stop", "Cancel", "Main");
        }

    } else if (stateString == "Alarm") {
        buttonLegends("", "", "Main");
    }

    refreshDisplaySprite();
}

void probeMenu() {
    static long oldPosition = 0;
    long        newPosition = M5Dial.Encoder.read();
    long        delta       = (newPosition - oldPosition);

    oldPosition   = newPosition;
    probe_encoder = newPosition;

    static int selection = 0;
    canvas.createSprite(240, 240);
    canvas.fillSprite(BLACK);
    drawStatus();
    menuTitle();
    drawStatus();

    if (M5Dial.BtnA.isPressed()) {  // if jog dial buttom was press
        while (M5Dial.BtnA.isPressed())
            M5Dial.update();
        delay(20);
        menu_number = MenuName::Main;
        return;
    }

    if (greenButton.changed()) {
        if (greenButton.active()) {
            // G38.2 G91 F80 Z-20 P8.00
            if (stateString == "Idle") {
                String gcode = "G38.2G91";
                gcode += "F" + floatToString(myPrefs.probe_rate, 0);
                gcode += axisNumToString(myPrefs.probe_axis) + floatToString(myPrefs.probe_travel, 0);
                gcode += "P" + floatToString(myPrefs.probe_offset, 2);

                USBSerial.println(gcode);
                send_line(gcode);
            } else if (stateString == "Run") {
                USBSerial.println("Hold");
                fnc_realtime(FeedHold);
            } else if (stateString == "Hold") {
                USBSerial.println("Resume");
                fnc_realtime(CycleStart);
            }
        }
    }

    if (redButton.changed()) {
        if (redButton.active()) {
            // G38.2 G91 F80 Z-20 P8.00
            if (stateString.equals("Run")) {
                Serial1.println("Reset");
                fnc_realtime(Reset);
            } else if (stateString.equals("Idle")) {
                String gcode = "$J=G91F1000";
                gcode += axisNumToString(myPrefs.probe_axis);
                gcode += (myPrefs.probe_travel < 0) ? "+" : "-";  // retract is opposite travel
                gcode += floatToString(myPrefs.probe_retract, 0);
                send_line(gcode);
            }
        }
    }

    // A touch allows you to rotate through the items to be adjusted.
    auto t = M5Dial.Touch.getDetail();
    if (prev_state != t.state) {
        if (t.state == m5::touch_state_t::touch_end) {
            M5Dial.Speaker.tone(1800, 20);
            rotateNumberLoop(selection, 1, 0, 4);
        }
        //USBSerial.printf("%s\r\n", M5TouchStateName(t.state));
        prev_state = t.state;
    }

    if (abs(delta) > 0) {
        switch (selection) {
            case 0:
                myPrefs.probe_offset += (float)delta / 100;
                break;
            case 1:
                myPrefs.probe_travel += delta;
                break;
            case 2:
                myPrefs.probe_rate += delta;
                if (myPrefs.probe_rate < 1) {
                    myPrefs.probe_rate = 1;
                }
                break;
            case 3:
                myPrefs.probe_retract += delta;
                if (myPrefs.probe_retract < 0) {
                    myPrefs.probe_retract = 0;
                }
                break;
            case 4:
                rotateNumberLoop(myPrefs.probe_axis, 1, 0, 2);
        }
        prefsChanged = true;
    }

    int x      = 40;
    int y      = 62;
    int width  = 240 - (x * 2);
    int height = 25;
    int pitch  = 27;  // for spacing of buttons
    drawButton(x, y, width, height, 9, "Offset: " + floatToString(myPrefs.probe_offset, 2), selection == 0);
    drawButton(x, y += pitch, width, height, 9, "Max Travel: " + floatToString(myPrefs.probe_travel, 0), selection == 1);
    drawButton(x, y += pitch, width, height, 9, "Feed Rate: " + floatToString(myPrefs.probe_rate, 0), selection == 2);
    drawButton(x, y += pitch, width, height, 9, "Retract: " + floatToString(myPrefs.probe_retract, 0), selection == 3);
    drawButton(x, y += pitch, width, height, 9, "Axis: " + axisNumToString(myPrefs.probe_axis), selection == 4);

    drawLed(20, 128, 10, myProbeSwitch);

    String grnText = "";
    String redText = "";

    if (stateString == "Idle") {
        grnText = "Probe";
        redText = "Retract";
    } else if (stateString == "Run") {
        redText = "Reset";
        grnText = "Hold";
    } else if (stateString == "Hold") {
        redText = "Reset";
        grnText = "Resume";
    }

    buttonLegends(redText, grnText, "Main");
    refreshDisplaySprite();
}

void drawStatus() {
    int rect_color = WHITE;

    if (stateString == "Idle") {
        rect_color = WHITE;
    } else if (stateString == "Alarm") {
        rect_color = RED;
    } else if (stateString == "Run") {
        rect_color = GREEN;
    } else if (stateString == "Jog") {
        rect_color = CYAN;
    } else if (stateString == "Home") {
        rect_color = CYAN;
    } else if (stateString.equals("Hold")) {
        rect_color = YELLOW;
    }

    static constexpr int x      = 100;
    static constexpr int y      = 24;
    static constexpr int width  = 140;
    static constexpr int height = 36;

    canvas.fillRoundRect(120 - width / 2, y, width, height, 5, rect_color);
    canvas.setTextColor(BLACK);
    canvas.setTextDatum(middle_center);
    if (stateString.equals("Alarm")) {
        canvas.setFont(&fonts::FreeSansBold12pt7b);
        canvas.drawString(stateString, 120, y + height / 2 - 4);
        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.drawString(alarm_name(lastAlarm), 120, y + height / 2 + 12);
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

// Helpful function to rotate through an aaray of numbers
// Example:  rotateNumberLoop(2, 1, 0, 2); would change the current value to 0
void rotateNumberLoop(int& currentVal, int increment, int min, int max) {
    currentVal += increment;
    if (currentVal > max) {
        currentVal = min + (increment - 1);
    }
    if (currentVal < min) {
        currentVal = max - (increment + 1);
    }
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
    canvas.drawString(menu_names[(int)menu_number], 120, 12);
}

void refreshDisplaySprite() {
    M5Dial.Display.startWrite();
    canvas.pushSprite(0, 0);
    M5Dial.Display.endWrite();
}

bool dial_button() {
    M5Dial.update();
    if (M5Dial.BtnA.isPressed()) {
        while (M5Dial.BtnA.isPressed())
            M5Dial.update();
        delay(20);
        USBSerial.println("dial btn");  // debug info
        return true;
    }
    return false;
}

// Helpful for debugging touch development.
String M5TouchStateName(m5::touch_state_t state_num) {
    static constexpr const char* state_name[16] = { "none", "touch", "touch_end", "touch_begin", "___", "hold", "hold_end", "hold_begin",
                                                    "___",  "flick", "flick_end", "flick_begin", "___", "drag", "drag_end", "drag_begin" };

    return String(state_name[state_num]);
}

String axisNumToString(int axis) {
    return String("XYZABC").substring(axis, axis + 1);
}

String floatToString(float val, int afterDecimal) {
    char buffer[20];
    dtostrf(val, 1, afterDecimal, buffer);
    String str(buffer);
    return str;
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

void debug(const char* info) {
#ifdef DEBUG_TO_FNC
    String msg { "$Log/Msg=*" };
    send_line(msg + info);
#endif

#ifdef DEBUG_TO_USB
    USBSerial.println(info);
#endif
}

void savePrefs() {
    // EEPROM.put(0, myPrefs);
    // debug("put prefs");
}

void readPrefs() {
    // if (EEPROM.get(0, myPrefs.eeprom_ver) != 2) {
    //     // ver wrong, so save a default set
    //     savePrefs();
    //     return;
    // }

    // EEPROM.get(0, myPrefs);
    // debug("get prefs");
}

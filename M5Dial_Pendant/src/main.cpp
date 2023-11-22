#include <Arduino.h>
#include "M5Dial.h"
#include "logo_img.h"
#include "GrblParser.h"

#define RED_BUTTON GPIO_NUM_13
#define GREEN_BUTTON GPIO_NUM_15
#define BUTTON_REPEAT_RATE 250  // milliseconds
#define VERSION "0.9"

enum class MenuName : uint8_t {
    Main    = 0,
    Homing  = 1,
    Jogging = 2,
};

long                     oldPosition   = -4;
String                   stateString   = "Idle";
float                    myAxes[6]     = { 0 };
MenuName                 menu_number   = MenuName::Main;  // The menu that is currently active
int                      jog_axis      = 0;               // the axis currently being jogged
int                      jog_inc_level = 4;               // exponent 0=0.01, 2=0.1 ... 5 = 100.00
static m5::touch_state_t prev_state;
HardwareSerial           Serial_FNC(1);
bool                     statusUpdate = false;

void rotateNumberLoop(int& currentVal, int increment, int min, int max);

void main_menu(bool infoUpdate);
void homingMenu();
void joggingMenu();

// draw stuff
void   drawDRO(int x, int y, int axis, float value, bool highlighted);
void   drawStatus();
void   drawButton(int x, int y, int width, int height, int charSize, String text, bool highlighted);
void   buttonLegends(String red, String green, String orange);
void   greenButtonInt();
void   menuTitle();
void   refreshDisplaySprite();
bool   red_button();
bool   green_button();
String M5TouchStateName(m5::touch_state_t state_num);

class Displayer : public GrblParser {
    void show_state(const String& state) { stateString = state; }
    void show_dro(const float* axes, bool isMpos, bool* limits) {
        char delim = ' ';
        for (int i = 0; i < _n_axis; i++) {
            myAxes[i] = axes[i];
        }
    }
    void end_status_report() { statusUpdate = true; }
    int  getchar() {
        if (Serial_FNC.available()) {
            int c = Serial_FNC.read();
            USBSerial.write(c);  // echo
            return c;
        }
        return -1;
    }
    int milliseconds() { return millis(); }

    void poll_extra() {}

public:
    void putchar(uint8_t c) { Serial1.write(c); }

} displayer;

M5Canvas canvas(&M5Dial.Display);

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    pinMode(RED_BUTTON, INPUT_PULLUP);    // Port A SCL
    pinMode(GREEN_BUTTON, INPUT_PULLUP);  // Port A SDA

    USBSerial.begin(115200);
    Serial_FNC.begin(115200, SERIAL_8N1, 1, 2);  // assign pins to the M5Stamp Port B

    M5Dial.Display.clear();
    M5Dial.Display.fillScreen(WHITE);
    M5Dial.Display.drawBitmap(20, 83, 199, 74, logo_img);  // need to change the endianness to change to the new command

    delay(3000);  // view the log and wait for the USBSerial to be detected by the PC
    Serial_FNC.println("$Log/Msg=*M5Dial Pendant v0.1");
    USBSerial.println("Debug: M5Dial Pendant");
    Serial_FNC.write('?');  // change to displayer
    main_menu(true);
}

void loop() {
    M5Dial.update();
    displayer.poll();  // do the serial port rerading and echoing
    //long newPosition = M5Dial.Encoder.read();
    switch (menu_number) {
        case MenuName::Main:
            main_menu(statusUpdate);
            break;
        case MenuName::Homing:
            homingMenu();
            break;
        case MenuName::Jogging:
            joggingMenu();
            break;
    }
}

void main_menu(bool infoUpdate) {
    static long oldPosition = 0;
    static int  menu_item   = 0;
    long        newPosition = M5Dial.Encoder.read();
    long        delta       = (newPosition - oldPosition) / 4;
    if (M5Dial.BtnA.isPressed()) {  // if jog dial buttom was press
        while (M5Dial.BtnA.isPressed())
            M5Dial.update();
        delay(20);
        switch (menu_item) {
            case 0:
            case 1:
            case 2:
                menu_number = MenuName::Jogging;
                jog_axis    = menu_item;
                return;
            case 3:
                menu_number = MenuName::Homing;
                return;
        }
    }
    if (red_button()) {
        USBSerial.println("Red btn");  // debug info
        if (stateString == "Alarm") {
            Serial_FNC.println("$X");
        }
    }

    if (green_button()) {
        USBSerial.println("Grn btn");  // debug info
    }

    if (abs(delta) > 0 || infoUpdate) {
        rotateNumberLoop(menu_item, delta, 0, 4);
        oldPosition += delta * 4;
        // M5Dial.Speaker.tone(1000, 30);
        canvas.createSprite(240, 240);
        canvas.fillSprite(BLACK);
        drawStatus();
        int y       = 68;
        int spacing = 33;
        drawDRO(10, y, 0, myAxes[0], menu_item == 0);
        drawDRO(10, y += spacing, 1, myAxes[1], menu_item == 1);
        drawDRO(10, y += spacing, 2, myAxes[2], menu_item == 2);
        y = 170;
        drawButton(38, y, 74, 30, 12, "Home", menu_item == 3);
        drawButton(128, y, 74, 30, 12, "Probe", menu_item == 4);

        String encoder_button_text = "";
        switch (menu_item) {
            case 0:
                encoder_button_text = "Jog X";
                break;
            case 1:
                encoder_button_text = "Jog Y";
                break;
            case 2:
                encoder_button_text = "Jog Z";
                break;
            case 3:
                encoder_button_text = "Home";
                break;
            case 4:
                encoder_button_text = "Probe";
                break;
        }
        String redButtonText   = "";
        String greenButtonText = "";
        if (stateString == "Alarm") {
            redButtonText = "Reset";
        } else if (stateString == "Run") {
            redButtonText   = "Reset";
            greenButtonText = "Hold";
        } else if (stateString.startsWith("Hold")) {
            redButtonText   = "Reset";
            greenButtonText = "Start";
        } else if (stateString == "Jog") {
            redButtonText = "Jog Cancel";
        } else if (stateString == "Idle") {
            // no commands?
        }
        menuTitle();
        buttonLegends(redButtonText, greenButtonText, encoder_button_text);

        refreshDisplaySprite();
    }
}

void homingMenu() {
    static long oldPosition = 0;
    static int  menu_item   = 0;
    long        newPosition = M5Dial.Encoder.read();
    long        delta       = (newPosition - oldPosition) / 4;
    if (abs(delta) > 0) {
        oldPosition += delta * 4;
        drawStatus();

        int x      = 30;
        int y      = 65;
        int gap    = 30;
        int width  = 120;
        int height = 26;
        drawButton(x, y, width, height, 24, "Home All", true);
        drawButton(x, y += gap, width, height, 24, "Home X", false);
        drawButton(x, y += gap, width, height, 24, "Home Y", false);
        drawButton(x, y += gap, width, height, 24, "Home Z", false);

        x     = 153;
        y     = 65;
        width = 40;
        drawButton(x, y, width, height, 24, "0", false);
        drawButton(x, y += gap, width, height, 24, "0", false);
        drawButton(x, y += gap, width, height, 24, "0", false);
        drawButton(x, y += gap, width, height, 24, "0", false);

        menuTitle();
        buttonLegends("Rset", "Main", "Home All");
        refreshDisplaySprite();
    }
}

void joggingMenu() {
    static long oldPosition = 0;
    long        newPosition = M5Dial.Encoder.read();
    long        delta       = (newPosition - oldPosition) / 4;
    bool        touch       = false;

    // Dial Button handling
    if (M5Dial.BtnA.isPressed()) {
        while (M5Dial.BtnA.isPressed())
            M5Dial.update();
        delay(20);
        menu_number = MenuName::Main;
        return;
    }

    // A touch allows you to rotate through the axis being jogged
    auto t = M5Dial.Touch.getDetail();
    if (prev_state != t.state) {
        if (t.state == m5::touch_state_t::touch_end) {
            rotateNumberLoop(jog_axis, 1, 0, 2);
            touch = true;
        }
        USBSerial.printf("%s\r\n", M5TouchStateName(t.state));
        prev_state = t.state;
    }

    if (abs(delta) > 0 || touch) {
        oldPosition += delta * 4;

        drawStatus();

        drawDRO(10, 71, 0, myAxes[0], jog_axis == 0);
        drawDRO(10, 104, 1, myAxes[1], jog_axis == 1);
        drawDRO(10, 137, 2, myAxes[2], jog_axis == 2);

        canvas.setTextFont(&fonts::FreeMonoBold12pt7b);
        canvas.setTextColor(WHITE);
        canvas.setTextDatum(middle_center);
        char  buffer[20];  // Enough room for the digits you want and more to be safe
        float jog_increment = pow(10.0, jog_inc_level) / 100.0;
        dtostrf(jog_increment, 6, 2, buffer);
        String foo(buffer);
        foo = "Jog Inc:" + foo;
        canvas.drawString(foo, 120, 185);

        menuTitle();
        buttonLegends("Inc+", "Inc-", "Main");
        refreshDisplaySprite();
    }
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
    } else if (stateString.startsWith("Hold")) {
        rect_color = YELLOW;
    }

    static constexpr int x      = 100;
    static constexpr int y      = 24;
    static constexpr int width  = 140;
    static constexpr int height = 34;

    canvas.fillRoundRect(120 - width / 2, y, width, height, 5, rect_color);
    canvas.setTextFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(BLACK);
    canvas.setTextDatum(middle_center);
    canvas.drawString(stateString, 120, y + height / 2 + 3);
}

void drawDRO(int x, int y, int axis, float value, bool highlighted) {
    int color_value, color_hightlight;
    M5Dial.Display.setTextFont(&fonts::FreeMonoBold18pt7b);

    String axis_label = String("XYZABC").substring(axis, axis + 1);

    static constexpr int width  = 220;
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

    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_left);
    canvas.drawString(axis_label, x + 5, y + height / 2 + 2);
    char buffer[20];  // Enough room for the digits you want and more to be safe
    dtostrf(value, 9, 2, buffer);
    canvas.setTextDatum(middle_right);
    canvas.drawString(String(buffer), x + width - 5, y + height / 2 + 2);
}

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
    if (charSize = 12) {
        canvas.setTextFont(&fonts::FreeSansBold12pt7b);
    } else {
        canvas.setTextFont(&fonts::FreeSansBold18pt7b);
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

void buttonLegends(String red, String green, String orange) {
    canvas.setTextFont(&fonts::FreeMonoBold9pt7b);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(RED);
    canvas.drawString(red, 80, 209);
    canvas.setTextColor(GREEN);
    canvas.drawString(green, 160, 209);
    canvas.setTextColor(ORANGE);
    canvas.drawString(orange, 120, 224);
}

void menuTitle() {
    String menu_names[] = { "Main", "Homing", "Jogging" };

    canvas.setTextFont(&fonts::FreeMonoBold9pt7b);
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(WHITE);

    canvas.drawString(menu_names[(int)menu_number], 120, 12);
}

void refreshDisplaySprite() {
    M5Dial.Display.startWrite();
    canvas.pushSprite(0, 0);
    M5Dial.Display.endWrite();
}

// This returns true is a button is pressed and the debounce has not expired
// Button is active low
bool red_button() {
    static uint32_t last_change_millis = millis();

    if (!digitalRead(RED_BUTTON) && (millis() - last_change_millis > BUTTON_REPEAT_RATE)) {
        last_change_millis = millis();
        return true;
    }
    return false;
}

bool green_button() {
    static uint32_t last_change_millis = millis();

    if (!digitalRead(GREEN_BUTTON) && (millis() - last_change_millis > BUTTON_REPEAT_RATE)) {
        last_change_millis = millis();
        return true;
    }
    return false;
}

// Helpful for touch development.
String M5TouchStateName(m5::touch_state_t state_num) {
    static constexpr const char* state_name[16] = { "none", "touch", "touch_end", "touch_begin", "___", "hold", "hold_end", "hold_begin",
                                                    "___",  "flick", "flick_end", "flick_begin", "___", "drag", "drag_end", "drag_begin" };

    return String(state_name[state_num]);
}

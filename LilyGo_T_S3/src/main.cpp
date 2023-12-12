/*
 * 
 * 
 *  Note: If the USB is not connected this program locks up when that serial port is used. 
 *        Maybe there are some floating pins that generate data.
 */

#include "Arduino.h"
#include "GrblParserC.h"
#include "pin_config.h"
#include "fnc.h"
#include "TFT_eSPI.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#    error "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif

#define FNCSerial Serial1
#define DebugSerial Serial

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
   switch macro. */
#define LCD_MODULE_CMD_1
#define ECHO_RX_DATA
//#define DEBUG_USB

TFT_eSPI    tft     = TFT_eSPI();
TFT_eSprite sprite1 = TFT_eSprite(&tft);  // Used to prevent flickering

// local copies so we can do one update function
String myState              = "No data...";
float  myAxes[MAX_N_AXIS]   = { 0 };
int    my_n_axis            = 3;
bool   myLimits[MAX_N_AXIS] = { false };
bool   myProbe              = false;
bool   use_mm               = true;

void updateDisplay();

#if defined(LCD_MODULE_CMD_1)
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    { 0x11, { 0 }, 0 | 0x80 },
    { 0x3A, { 0X05 }, 1 },
    { 0xB2, { 0X0B, 0X0B, 0X00, 0X33, 0X33 }, 5 },
    { 0xB7, { 0X75 }, 1 },
    { 0xBB, { 0X28 }, 1 },
    { 0xC0, { 0X2C }, 1 },
    { 0xC2, { 0X01 }, 1 },
    { 0xC3, { 0X1F }, 1 },
    { 0xC6, { 0X13 }, 1 },
    { 0xD0, { 0XA7 }, 1 },
    { 0xD0, { 0XA4, 0XA1 }, 2 },
    { 0xD6, { 0XA1 }, 1 },
    { 0xE0, { 0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32 }, 14 },
    { 0xE1, { 0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37 }, 14 },
};
#endif

extern "C" void show_state(const char* state) {
    myState = state;
    updateDisplay();
}

extern "C" void show_dro(const pos_t* axes, const pos_t* wcos, bool isMpos, bool* limits, size_t n_axis) {
    my_n_axis = n_axis;
    for (int i = 0; i < n_axis; i++) {
        myAxes[i] = axes[i];
    }
    for (int i = 0; i < n_axis; i++) {
        myLimits[i] = limits[i];
    }
    updateDisplay();  // TO DO reduce the number of these
}

extern "C" void show_limits(bool probe, const bool* limits, size_t n_axis) {
    // limits done with DROs
    if (myProbe != probe) {
        myProbe = probe;
    }
    updateDisplay();
}

extern "C" int fnc_getchar() {
    if (FNCSerial.available()) {
        return FNCSerial.read();
    }
    return -1;
}
extern "C" void fnc_putchar(uint8_t c) {
    FNCSerial.write(c);
}
extern "C" void debug_putchar(char c) {
    DebugSerial.write(c);
}
extern "C" int milliseconds() {
    return millis();
}

void setup() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    pinMode(PIN_BUTTON_2, INPUT);  // active low has physical pullup and RC

#ifdef DEBUG_USB
    DebugSerial.begin(115200);  // used for debugging
    delay(2000);                // delay to allow USB connection of PC to connect
    Serial.println("Begin T-Display-S3");
#endif

    FNCSerial.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);  // connected to FluidNC

    tft.begin();
    tft.setRotation(3);
    tft.setSwapBytes(true);

    sprite1.setColorDepth(16);
    sprite1.createSprite(DISP_WIDTH, DISP_HEIGHT);
    sprite1.fillSprite(TFT_BLACK);

#if defined(LCD_MODULE_CMD_1)
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < (lcd_st7789v[i].len & 0x7f); j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);
#else
    ledcAttach(PIN_LCD_BL, 200, 8);
    ledcWrite(PIN_LCD_BL, 255);
#endif

    tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
    //tft.drawString("FluidNC Channel pendant", 0 , 0, 4);

    tft.pushImage(0, 0, 320, 170, logo);
    delay(2000);

    updateDisplay();

    fnc_wait_ready();  // Synchronize with FluidNC
    fnc_putchar('?');  // Initial status report
}

void loop() {
    fnc_poll();
}

void readButtons() {
    if (!digitalRead(PIN_BUTTON_2)) {
        if (myState == "Run") {
            debug_putchar('!');
            fnc_putchar('!');
        } else if (myState.startsWith("Hold")) {
            debug_putchar('~');
            fnc_putchar('~');
        }
        delay(50);
        while (!digitalRead(PIN_BUTTON_2)) {}
        delay(50);
    }
}

String DRO_format(int axis, float val) {
    String format;
    char   buf[12];
    int    len;
    String DRO;
    String axesNames = "XYZABC";

    if (use_mm) {
        format = "% 4.2f";
    } else {
        format = "% 3.3f";
    }

    len = sprintf(buf, format.c_str(), val);

    DRO = buf;

    while (DRO.length() < 9) {
        DRO = " " + DRO;
    }

    DRO = axesNames.substring(axis, axis + 1) + DRO;
    return DRO;
}

void drawCheckbox(int x, int y, int width, bool checked, String label) {
    if (checked) {
        sprite1.fillRect(x, y, width, width, TFT_GREEN);
    } else {
        sprite1.drawRect(x, y, width, width, TFT_GREEN);
    }

    sprite1.drawString(label, x + width + 5, y, 4);
}

void updateDisplay() {
    char   buf[12];
    String axesNames = "XYZABC";

    sprite1.fillSprite(TFT_BLACK);

    if (myState == "Alarm") {
        sprite1.setTextColor(TFT_RED, TFT_BLACK);
    } else if (myState.startsWith("Hold")) {
        sprite1.setTextColor(TFT_YELLOW, TFT_BLACK);
    } else {
        sprite1.setTextColor(TFT_GREEN, TFT_BLACK);
    }

    sprite1.drawString(myState, 0, 0, 4);
    drawCheckbox(150, 0, 18, myProbe, "Probe");

    for (int i = 0; i < my_n_axis; i++) {
        sprite1.drawString(DRO_format(i, myAxes[i]), 0, 26 + i * 24, 4);
        drawCheckbox(150, 26 + i * 24, 18, myLimits[i], "Limit");
    }

    sprite1.pushSprite(0, 0);
}

extern "C" void poll_extra() {
#ifdef DEBUG_USB
    while (DebugSerial.available()) {
        char c = DebugSerial.read();
        if (c != '\r') {
            debug_putchar(c);
            putchar(c);
        }
    }
#endif
    readButtons();
}

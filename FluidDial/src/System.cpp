// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "System.h"
#include <Esp.h>  // ESP.restart()
#include "FluidNCModel.h"

M5Canvas           canvas(&M5Dial.Display);
M5GFX&             display = M5Dial.Display;
m5::Speaker_Class& speaker = M5Dial.Speaker;
m5::Touch_Class&   touch   = M5Dial.Touch;

m5::Button_Class  greenButton;
m5::Button_Class  redButton;
m5::Button_Class& dialButton = M5Dial.BtnA;

Stream& debugPort = USBSerial;

HardwareSerial Serial_FNC(1);  // Serial port for comm with FNC

extern "C" int milliseconds() {
    return m5gfx::millis();
}
extern "C" void fnc_putchar(uint8_t c) {
    Serial_FNC.write(c);
}

extern "C" int fnc_getchar() {
    if (Serial_FNC.available()) {
        update_rx_time();
        int c = Serial_FNC.read();
#ifdef ECHO_FNC_TO_DEBUG
        dbg_write(c);
#endif
        return c;
    }
    return -1;
}

extern "C" void poll_extra() {
    if (debugPort.available()) {
        char c = debugPort.read();
        if (c == 0x12) {  // CTRL-R
            ESP.restart();
            while (1) {}
        }
        fnc_putchar(c);  // So you can type commands to FluidNC
    }
}

// Helpful for debugging touch development.
const char* M5TouchStateName(m5::touch_state_t state_num) {
    static constexpr const char* state_name[16] = { "none", "touch", "touch_end", "touch_begin", "___", "hold", "hold_end", "hold_begin",
                                                    "___",  "flick", "flick_end", "flick_begin", "___", "drag", "drag_end", "drag_begin" };

    return state_name[state_num];
}

void drawPngFile(const char* filename, int x, int y) {
    // When datum is middle_center, the origin is the center of the canvas and the
    // +Y direction is down.
    canvas.drawPngFile(LittleFS, filename, x, -y, 0, 0, 0, 0, 1.0f, 1.0f, datum_t::middle_center);
}

#define FORMAT_LITTLEFS_IF_FAILED true

void init_system() {
    auto cfg            = M5.config();
    cfg.serial_baudrate = 921600;
    // Don't enable the encoder because M5's encoder driver is flaky
    M5Dial.begin(cfg, false, false);

    // Turn on the power hold pin
    lgfx::gpio::command(lgfx::gpio::command_mode_output, GPIO_NUM_46);
    lgfx::gpio::command(lgfx::gpio::command_write_high, GPIO_NUM_46);

    // This must be done after M5Dial.begin which sets the PortA pins
    // to I2C mode.  We need to override that to use them for serial.
    USBSerial.begin(921600);
    Serial_FNC.begin(115200, SERIAL_8N1, FNC_RX_PIN, FNC_TX_PIN);

    // Setup external GPIOs as buttons
    lgfx::gpio::command(lgfx::gpio::command_mode_input_pullup, RED_BUTTON_PIN);
    lgfx::gpio::command(lgfx::gpio::command_mode_input_pullup, GREEN_BUTTON_PIN);

    greenButton.setDebounceThresh(5);
    redButton.setDebounceThresh(5);

    touch.setFlickThresh(30);

    init_encoder();  // Use our own encoder driver

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        dbg_println("LittleFS Mount Failed");
        return;
    }

    // Make an offscreen canvas that can be copied to the screen all at once
    canvas.createSprite(display.width(), display.height());

    // Draw the logo screen
    display.clear();
    display.drawPngFile(LittleFS, "/fluid_dial.png", 0, 0, display.width(), display.height(), 0, 0, 0.0f, 0.0f, datum_t::middle_center);

    speaker.setVolume(255);
}

void ackBeep() {
    speaker.tone(1800, 50);
}

void dbg_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void dbg_write(uint8_t c) {
#ifdef DEBUG_TO_USB
    if (debugPort.availableForWrite() > 1) {
        debugPort.write(c);
    }
#endif
}

void dbg_print(const std::string& s) {
    dbg_print(s.c_str());
}
void dbg_print(const char* s) {
#ifdef DEBUG_TO_USB
    if (debugPort.availableForWrite() > strlen(s)) {
        debugPort.print(s);
    }
#endif
}

void dbg_println(const std::string& s) {
    dbg_println(s.c_str());
}

void dbg_println(const char* s) {
    dbg_print(s);
    dbg_print("\r\n");
}

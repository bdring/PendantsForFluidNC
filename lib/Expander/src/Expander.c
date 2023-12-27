// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Expander.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "pin.h"

#ifdef __cplusplus
extern "C" {
#endif

// The integer value for
// pin low reports is 0x100 to 0x13f, and for
// pin high reports is 0x140 to 0x17f
// In UTF8, 0x100 to 0x13f encodes to 0xC4 0x80+N
// and 0x140 to 0x17f encodes to 0xC5 0x80+N
const uint8_t PinLowUTF8Prefix  = 0xC4;
const uint8_t PinHighUTF8Prefix = 0xC5;

// With no arguments, return an ACK for okay
void expander_ack() {
    fnc_realtime(ACK);
}

void expander_nak(const char* msg) {
    debug_println(msg);
    fnc_realtime(NAK);
}

void expander_pin_msg(uint8_t pin_num, bool active) {
    // UTF8 encoding
    fnc_putchar(active ? PinHighUTF8Prefix : PinLowUTF8Prefix);
    fnc_putchar(0x80 + pin_num);
}

pin_mode_t parse_io_mode(const char* params) {
    pin_mode_t mode = 0;
    if (strstr(params, "low")) {
        mode |= PIN_ACTIVELOW;
    }
    if (strstr(params, "out")) {
        mode |= PIN_OUTPUT;
    }
    if (strstr(params, "in")) {
        mode |= PIN_INPUT;
    }
    if (strstr(params, "pwm")) {
        mode |= PIN_PWM;
    }
    if (strstr(params, "pu")) {
        mode |= PIN_PULLUP;
    }
    if (strstr(params, "pd")) {
        mode |= PIN_PULLDOWN;
    }
    return mode;
}

static void trim(char** str) {
    char* s = *str;
    while (isspace(*s)) {
        ++s;
    }
    char* p = s + strlen(s);
    while (p != s && isspace(p[-1])) {
        --p;
    }
    *p   = '\0';
    *str = s;
}

bool expander_handle_msg(char* command, char* pinspecs) {
    if (strcmp(command, "RST") == 0) {
        expander_rst();
        expander_ack();
        return true;
    }
    if ((strcmp(command, "INI") == 0) || (strcmp(command, "GET") == 0) || (strcmp(command, "SET") == 0)) {
        // IO operation examples:
        //   INI: io.N=out,low
        //   INI: io.N=inp,pu
        //   INI: io.N=pwm
        //   GET: io.*
        //   GET: io.N
        //   SET: io.N=0.5
        uint8_t pin_num = 0;

        trim(&pinspecs);

        size_t prefixlen = strlen("io.");
        if (strncmp(pinspecs, "io.", prefixlen) != 0) {
            expander_nak("Missing io. specifier");
            return true;
        }

        char* pin_str = pinspecs + prefixlen;
        char* params;

        if (strcmp(command, "GET") == 0) {
            split(pin_str, &params, '=');

            if (*pin_str == '*') {
                expander_ack();
                expander_get_all();
                return true;
            }
            pin_num = atoi(pin_str);

            if (expander_get(pin_num)) {
                expander_ack();
            } else {
                expander_nak("Invalid pin_str number");
            }
            return true;
        }

        if (strcmp(command, "INI") == 0) {
            split(pin_str, &params, '=');
            pin_num = atoi(pin_str);

            if (expander_ini(pin_num, parse_io_mode(params))) {
                expander_ack();
            } else {
                expander_nak("INI Error");
            }
            return true;
        }

        if (strcmp(command, "SET") == 0) {
            split(pin_str, &params, '=');
            pin_num = atoi(pin_str);
            if (*params == '\0') {
                expander_nak("Missing value for SET");
            } else {
                int32_t  numerator;
                uint32_t denominator;
                if (atofraction(params, &numerator, &denominator)) {
                    if (expander_set(pin_num, numerator, denominator)) {
                        expander_ack();
                    } else {
                        expander_nak("Set Error");
                    }
                } else {
                    expander_nak("Bad set value");
                }
            }
            return true;
        }
    }
    return false;  // Not handled
}

// Implement these to handle expander IO messages
bool __attribute__((weak)) expander_rst() {
    deinit_all_pins();
    return true;
}
bool __attribute__((weak)) expander_ini(uint8_t pin_num, pin_mode_t pinmode) {
    return set_pin_mode(pin_num, pinmode) == fail_none;
}
bool __attribute__((weak)) expander_get_all() {
    update_all_pins();
    read_all_pins(expander_pin_msg);
    return true;
}
bool __attribute__((weak)) expander_get(uint8_t pin_num) {
    read_pin(expander_pin_msg, pin_num);
    return true;
}
bool __attribute__((weak)) expander_set(uint8_t pin_num, int32_t numerator, uint32_t denominator) {
    return set_output(pin_num, numerator, denominator) == fail_none;
}

void __attribute__((weak)) expander_poll() {
    read_all_pins(expander_pin_msg);
}

#ifdef __cplusplus
}
#endif

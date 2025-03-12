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

void expander_start() {
    fnc_realtime(RST);
}

// With no arguments, return an ACK for okay
void expander_ack() {
    fnc_realtime(ACK);
}

void expander_nak(const char* msg) {
    debug_print("[MSG:ERR: ");
    debug_print(msg);
    debug_println("]");
    fnc_realtime(NAK);
}

void expander_pin_msg(uint8_t pin_num, bool active) {
    // UTF8 encoding
    fnc_putchar(active ? PinHighUTF8Prefix : PinLowUTF8Prefix);
    fnc_putchar(0x80 + pin_num);
}

pin_mode_t parse_io_mode(char* params) {
    pin_mode_t mode = 0;
    for (char* rest; *params; params = rest) {
        split(params, &rest, ',');
        if (strcasecmp(params, "low") == 0) {
            mode |= PIN_ACTIVELOW;
            continue;
        }
        if (strcasecmp(params, "out") == 0) {
            mode |= PIN_OUTPUT;
            continue;
        }
        if (strcasecmp(params, "in") == 0) {
            mode |= PIN_INPUT;
            continue;
        }
        if (strcasecmp(params, "pwm") == 0) {
            mode |= PIN_PWM;
            continue;
        }
        if (strcasecmp(params, "pu") == 0) {
            mode |= PIN_PULLUP;
            continue;
        }
        if (strcasecmp(params, "pd") == 0) {
            mode |= PIN_PULLDOWN;
            continue;
        }
        if (strncasecmp(params, "frequency=", strlen("frequency=")) == 0) {
            int freq = atoi(params + strlen("frequency="));
            mode |= freq << PIN_FREQ_SHIFT;
            continue;
        }
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

static void expander_ack_nak(bool okay, const char* errmsg) {
    if (okay) {
        expander_ack();
    } else {
        expander_nak(errmsg);
    }
}

#define PinLowFirst 0x100
#define PinLowLast 0x13f
#define PinHighFirst 0x140
#define PinHighLast 0x17f
#define SetPWM 0x10000
void handle_extended_command(uint32_t cmd) {
    uint32_t value;
    int      pin_num;
    if (cmd >= SetPWM) {
        value   = cmd - SetPWM;
        pin_num = value >> 10;
        value   = value & 0x3ff;
    } else if (cmd >= PinLowFirst && cmd < PinLowLast) {
        value   = 0;
        pin_num = cmd - PinLowFirst;
    } else if (cmd >= PinHighFirst && cmd < PinHighLast) {
        value   = 1;
        pin_num = cmd - PinHighFirst;
    } else {
        return;
    }
    int fail = set_output(pin_num, value, 1000);
    if (fail) {
        expander_nak("Cannot set output");
    }
}

bool expander_handle_command(char* command) {
    size_t len = strlen(command);
    if (!len) {
        return false;
    }
    if (strcmp(command, "[MSG:RST]") == 0) {
        expander_rst();
        return false;  // This message is not specific to the expander
    }

    if (command[len - 1] != ']') {
        return false;
    }

    // IO operation examples:
    //   [INI: io.N=out,low]
    //   [INI: io.N=inp,pu]
    //   [INI: io.N=pwm]
    //   [GET: io.*]
    //   [GET: io.N]
    //   [SET: io.N=0.5]
    bool is_set = false, is_get = false, is_ini = false;
    is_set = strncmp(command, "[SET", 5) == 0;
    if (!is_set) {
        is_get = strncmp(command, "[GET", 5) == 0;
        if (!is_get) {
            is_ini = strncmp(command, "[INI", 5) == 0;
        }
        if (!is_ini) {
            return false;
        }
    }
    char* pinspec;
    split(command, &pinspec, ':');

    // Now we know that the command is for the expander so it is okay to modify the string
    command[len - 1] = '\0';
    char* params;
    split(pinspec, &params, '=');

    trim(&pinspec);  // Remove leading and trailing blanks
    trim(&params);   // Remove leading and trailing blanks

    size_t prefixlen = strlen("io.");
    if (strncmp(pinspec, "io.", prefixlen) != 0) {
        expander_nak("Missing io. specifier");
        return true;
    }

    char* pin_str = pinspec + prefixlen;
    int   pin_num = atoi(pin_str);  // Will be 0 if pin_str is "*"

    if (is_set) {
        if (*params == '\0') {
            expander_nak("Missing value for SET");
        } else {
            int32_t  numerator;
            uint32_t denominator;
            if (atofraction(params, &numerator, &denominator)) {
                expander_ack_nak(expander_set(pin_num, numerator, denominator), "Set Error");
            } else {
                expander_nak("Bad set value");
            }
        }
        return true;
    }

    if (is_get) {
        if (*pin_str == '*') {
            expander_ack();
            expander_get_all();
            return true;
        }
        expander_ack_nak(expander_get(pin_num), "Invalid pin_str number");
        return true;
    }

    if (is_ini) {
        bool res = expander_ini(pin_num, parse_io_mode(params));
        expander_ack_nak(res, "INI Error");
        if (res) {
            expander_get(pin_num);
        }
        return true;
    }

    // Can't happen because one of is_set, is_get, is_ini must be true
    return true;  // Not handled
}

// Implement these to handle expander IO messages
bool __attribute__((weak)) expander_rst() {
    deinit_all_pins();
    return true;
}
bool __attribute__((weak)) expander_ini(uint8_t pin_num, pin_mode_t pinmode) {
    int fail = set_pin_mode(pin_num, pinmode);
    return fail == fail_none;
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

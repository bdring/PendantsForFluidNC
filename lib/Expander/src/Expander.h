// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "GrblParserC.h"
#include "pinmode.h"

// IO expander API

// expander_handle_command() handles IO Expander MSG: messages.
// The app must call it from the implementation of handle_msg()
extern bool expander_handle_command(char* command);

// expander_poll() checks for GPIO input pin changes
// The app must call it from poll_extra() unless GPIO changes
// are handled via interrupts
extern void expander_poll();

// The following are called when IO Expander messages are parsed.
// Normally these are automatically implemented to refer to pin functions,
// but they are weak definitions so the app can override them if necessary.

// MSG:RST
extern bool expander_rst();

// MSG:EXP io.n=mode
extern bool expander_ini(uint8_t pin_num, pin_mode_t pinmode);

// Automatic
extern bool expander_get(uint8_t pin_num);

// Invoked by Realtime code
extern bool expander_set(uint8_t pin_num, int32_t numerator, uint32_t denominator);

extern void expander_start();

extern void expander_report_info();

extern const char* fw_version;
extern const char* board_name;

#ifdef __cplusplus
}
#endif

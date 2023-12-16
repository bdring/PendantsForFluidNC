// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_N_AXIS 6
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2
#define A_AXIS 3
#define B_AXIS 4
#define C_AXIS 5

typedef float    pos_t;
typedef int32_t  feedrate_t;
typedef uint32_t override_percent_t;
typedef int32_t  file_percent_t;

typedef enum {
    None                  = 0,
    Reset                 = 0x18,  // Ctrl-X
    StatusReport          = '?',
    CycleStart            = '~',
    FeedHold              = '!',
    SafetyDoorEvent       = 0x84,
    JogCancel             = 0x85,
    DebugReport           = 0x86,  // Only when DEBUG_REPORT_REALTIME enabled, sends debug report in '{}' braces.
    Macro0                = 0x87,
    Macro1                = 0x88,
    Macro2                = 0x89,
    Macro3                = 0x8a,
    FeedOvrReset          = 0x90,  // Restores feed override value to 100%.
    FeedOvrCoarsePlus     = 0x91,
    FeedOvrCoarseMinus    = 0x92,
    FeedOvrFinePlus       = 0x93,
    FeedOvrFineMinus      = 0x94,
    RapidOvrReset         = 0x95,  // Restores rapid override value to 100%.
    RapidOvrMedium        = 0x96,
    RapidOvrLow           = 0x97,
    RapidOvrExtraLow      = 0x98,  // *NOT SUPPORTED*
    SpindleOvrReset       = 0x99,  // Restores spindle override value to 100%.
    SpindleOvrCoarsePlus  = 0x9A,  // 154
    SpindleOvrCoarseMinus = 0x9B,
    SpindleOvrFinePlus    = 0x9C,
    SpindleOvrFineMinus   = 0x9D,
    SpindleOvrStop        = 0x9E,
    CoolantFloodOvrToggle = 0xA0,
    CoolantMistOvrToggle  = 0xA1,
    // UART Extender
    PinLow  = 0xB0,  // Start of two-character sequence; second is event number
    PinHigh = 0xB1,  // Start of two-character sequence; second is event number
    ACK     = 0xB2,  // IO Expander accepted command
    NAK     = 0xB3,  // IO Expander rejected command
} realtime_cmd_t;

struct gcode_modes {
    const char* modal;
    const char* wcs;
    const char* plane;
    const char* units;
    const char* distance;
    const char* program;
    const char* spindle;
    const char* coolant;
    const char* parking;
    int         tool;
    uint32_t    spindle_speed;
    feedrate_t  feed;
};

// Wait until FluidNC is ready
void fnc_wait_ready();

// This is called from loop(), and also from send_line() while
// waiting for an ok from FluidNC.  You probably do not need to call
// it explicitly.
void fnc_poll();

// Call this to send a line-oriented command to FluidNC
void fnc_send_line(const char* line, int timeout_ms);

bool split(char* input, char** right, char delim);
bool atofraction(const char* p, int32_t* pnumerator, uint32_t* pdenominator);

// Inject a byte into the parser.  Normally this happens automatically
// via fnc_poll(), but it can also be called explicitly for debugging
void collect(uint8_t data);

// Implement these to send and receive characters from FluidNC
// Receive a character from FluidNC
extern int fnc_getchar();  // Must implement
// Send a character to FluidNC
extern void fnc_putchar(uint8_t ch);  // Must implement
// Get the time in milliseconds
extern int milliseconds();  // Must implement

extern void fnc_realtime(realtime_cmd_t c);

// Optional debug port routines
extern void debug_putchar(char c);
extern void debug_print(const char* msg);
extern void debug_println(const char* msg);
extern int  debug_getchar();

// The following can be implemented to do whatever your app
// wants to do.  Any that you do not implement will be automatically
// handled as no-ops.

// Implement this to do anything that must be done while waiting for characters
extern void poll_extra();

// Implement these to handle specific kinds of messages from FluidNC
extern void show_error(int error);
extern void show_alarm(int alarm);
extern void show_ok();
extern void show_timeout();

extern void expander_ack();
extern void expander_nak(const char* msg);

extern void expander_pin_msg(uint8_t pin_num, bool active);

// [MSG: messages
// General handler that can be ovverridden
extern void handle_msg(char* command, char* arguments);

extern void handle_grbl(char* line);

// Handle signon messages like "Grbl 3.4 [stuff]"
extern void handle_signon(char* version, char* extra);

// Handle anything not otherwise recognized
extern void handle_other(char* line);

extern uint32_t parse_io_mode(const char* params);

// Data parsed from <...> status reports
extern void show_limits(bool probe, const bool* limits, size_t n_axis);
extern void show_state(const char* state);
extern void show_dro(const pos_t* axes, const pos_t* wcos, bool isMpos, bool* limits, size_t n_axis);
extern void show_file(const char* filename, file_percent_t percent);
extern void show_linenum(int linenum);
extern void show_spindle_coolant(int spindle, bool flood, bool mist);
extern void show_feed_spindle(uint32_t feedrate, uint32_t spindle_speed);
extern void show_overrides(override_percent_t feed_ovr, override_percent_t rapid_ovr, override_percent_t spindle_ovr);

// [GC: messages
extern void show_gcode_modes(struct gcode_modes* modes);

// Called before and after parsing a status report; useful for
// clearing and updating display screens
extern void begin_status_report();
extern void end_status_report();

#ifdef __cplusplus
}
#endif

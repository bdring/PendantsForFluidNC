// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "GrblParserC.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static size_t _report_len = 0;
static char   _report[128];

static bool _ackwait = false;
static int  _ack_time_limit;

static int _n_axis;

static pos_t wcos[MAX_N_AXIS] = { 0 };

static struct gcode_modes old_gcode_modes;
static struct gcode_modes new_gcode_modes;

static void dbg(const char* line) {
    char msg[100];
    strcpy(msg, "$Msg/Uart0=");
    strcat(msg, line);
    fnc_send_line(msg, 1000);
}

bool split(char* input, char** next, char delim) {
    char* pos = strchr(input, delim);
    if (pos) {
        *pos  = '\0';
        *next = pos + 1;
        return true;
    }
    *next = input + strlen(input);  // End of string
    return false;
}

bool atofraction(const char* p, int32_t* pnumerator, uint32_t* pdenominator) {
    int32_t  numerator   = 0;
    uint32_t denominator = 1;
    bool     negate      = false;
    char     c;

    if (*p == '-') {
        ++p;
        negate = true;
    }

    while (isdigit((int)(c = *p++))) {
        numerator = numerator * 10 + (c - '0');
    }
    if (c == '.') {
        while (isdigit((int)(c = *p++))) {
            numerator = numerator * 10 + (c - '0');
            denominator *= 10;
        }
        if (c == '%') {
            denominator *= 100;
            c = *p++;
        }
    } else if (c == '/') {
        while (isdigit((int)(c = *p++))) {
            denominator = denominator * 10 + (c - '0');
        }
    } else if (c == '%') {
        denominator *= 100;
        c = *p++;
    }
    if (negate) {
        numerator = -numerator;
    }
    *pnumerator   = numerator;
    *pdenominator = denominator;

    return c == '\0';
}

static bool is_report_type(char* report, char** body, const char* prefix, const char* suffix) {
    size_t report_len = strlen(report);
    size_t prefix_len = strlen(prefix);
    if ((prefix_len <= report_len) && (strncmp(report, prefix, prefix_len) == 0)) {
        size_t suffix_len = strlen(suffix);
        if (suffix_len && (suffix_len < report_len) && (strcmp(report + report_len - suffix_len, suffix) == 0)) {
            report[report_len - suffix_len] = '\0';
        }
        *body = report + prefix_len;
        return true;
    }
    return false;
}

static void parse_msg(char* command) {
    // The report wrapper, already removed, is [MSG:...]
    // The body is, for example, INFO: data
    // The part before the optional : is the command.
    // If : is present, everything after it is the arguments to the command

    // Split the string into the command and arguments
    char* arguments;
    split(command, &arguments, ':');
    while (isspace((int)(*command))) {
        ++command;
    }
    char* end = command + strlen(command);
    while (end != command && isspace((int)(end[-1]))) {
        --end;
    }
    *end = '\0';

    handle_msg(command, arguments);
}

static void parse_error(const char* body) {
    // The report wrapper, already removed, is error:...
    show_error(atoi(body));
}

static void parse_alarm(const char* body) {
    // The report wrapper, already removed, is ALARM:...
    show_alarm(atoi(body));
}

static void parse_signon(char* body) {
    char* arguments;
    split(body, &arguments, ' ');
    handle_signon(body, arguments);
}

static pos_t atopos(const char* s) {
    int32_t  numerator;
    uint32_t denominator;
    atofraction(s, &numerator, &denominator);
    return (pos_t)numerator / denominator;
}

static size_t parse_axes(char* s, pos_t* axes) {
    char*  next;
    size_t n_axis = 0;
    do {
        split(s, &next, ',');
        if (_n_axis < MAX_N_AXIS) {
            axes[n_axis++] = atopos(s);
        }
        s = next;
    } while (*s);
    return n_axis;
}

static void parse_integers(char* s, uint32_t* nums, int maxnums) {
    char*  next;
    size_t i = 0;
    do {
        if (i >= maxnums) {
            return;
        }
        split(s, &next, ',');
        nums[i++] = atoi(s);

        s = next;
    } while (*s);
}

static void parse_status_report(char* field) {
    // The report wrapper, already removed, is <...>
    // The body is, for example,
    //   Idle|MPos:151.000,149.000,-1.000|Pn:XP|FS:0,0|WCO:12.000,28.000,78.000
    // i.e. a sequence of field|field|field

    bool has_linenum = false;
    int  linenum     = 0;
    int  spindle     = 0;
    bool has_a_field = false;
    bool flood       = false;
    bool mist        = false;

    char* next;
    split(field, &next, '|');
    if (*next == '\0') {
        return;  // Malformed report
    }

    char* state = field;

    bool probe              = false;
    bool limits[MAX_N_AXIS] = { false };

    pos_t axes[MAX_N_AXIS];
    bool  isMpos = false;

    bool has_override = false;

    bool           has_filename = false;
    char*          filename     = '\0';
    file_percent_t file_percent = 0;

    uint32_t           fs[2];
    override_percent_t frs[3];

    size_t n_axis = 0;

    // ... handle it
    while (*next) {
        field = next;
        split(field, &next, '|');

        // MPos:, WPos:, Bf:, Ln:, FS:, Pn:, WCO:, Ov:, A:, SD: (ISRs:, Heap:)
        char* value;
        split(field, &value, ':');

        if (strcmp(field, "MPos") == 0) {
            // x,y,z,...
            n_axis = parse_axes(value, axes);
            isMpos = true;
            continue;
        }
        if (strcmp(field, "WPos") == 0) {
            // x,y,z...
            n_axis = parse_axes(value, axes);
            isMpos = false;
            continue;
        }
        if (strcmp(field, "Bf") == 0) {
            // buf_avail,rx_avail
            continue;
        }
        if (strcmp(field, "Ln") == 0) {
            // n
            linenum = atoi(value);
            continue;
        }
        if (strcmp(field, "FS") == 0) {
            // feedrate,spindle_speed
            parse_integers(value, fs, 2);  // feed in [0], spindle in [1]
            continue;
        }
        if (strcmp(field, "Pn") == 0) {
            // PXxYy etc
            char c;
            while ((c = *value++) != '\0') {
                switch (c) {
                    case 'P':
                        probe = true;
                        break;
                    case 'X':
                        limits[X_AXIS] = true;
                        break;
                    case 'Y':
                        limits[Y_AXIS] = true;
                        break;
                    case 'Z':
                        limits[Z_AXIS] = true;
                        break;
                    case 'A':
                        limits[A_AXIS] = true;
                        break;
                    case 'B':
                        limits[B_AXIS] = true;
                        break;
                    case 'C':
                        limits[C_AXIS] = true;
                        break;
                }
                continue;
            }
        }
        if (strcmp(field, "WCO") == 0) {
            // x,y,z,...
            // We do not use the WCO values because the DROs show whichever
            // position is in the status report
            parse_axes(value, wcos);
            continue;
        }
        if (strcmp(field, "Ov") == 0) {
            has_override = true;
            // feed_ovr,rapid_ovr,spindle_ovr
            parse_integers(value, frs, 3);  // feed in [0], rapid in [1], spindle in [2]
            continue;
        }
        if (strcmp(field, "A") == 0) {
            // SCFM
            has_a_field = true;
            spindle     = 0;
            flood       = false;
            mist        = false;
            char c;
            while ((c = *value++) != '\0') {
                switch (c) {
                    case 'S':
                        spindle = 1;
                        break;
                    case 'C':
                        spindle = 2;
                        break;
                    case 'F':
                        flood = true;
                        break;
                    case 'M':
                        mist = true;
                        break;
                }
            }
            continue;
        }
        if (strcmp(field, "SD") == 0) {
            has_filename = true;
            char* comma  = strchr(value, ',');
            if (comma) {
                *comma   = '\0';
                filename = comma + 1;
            }
            int32_t  numerator;
            uint32_t denominator;
            atofraction(value, &numerator, &denominator);
            file_percent = numerator / denominator;
            continue;
        }
    }

    // Callbacks to handle the data extracted from the report
    begin_status_report();
    show_state(state);
    if (has_filename) {
        show_file(filename, file_percent);
    }
    if (n_axis) {
        show_limits(probe, limits, n_axis);
        show_dro(axes, wcos, isMpos, limits, n_axis);
    }
    if (has_linenum) {
        show_linenum(linenum);
    }
    if (has_a_field) {
        show_spindle_coolant(spindle, flood, mist);
    }
    show_feed_spindle(fs[0], fs[1]);
    if (has_override) {
        show_overrides(frs[0], frs[1], frs[2]);
    }

    end_status_report();
}

static struct GCodeMode {
    const char*  tag;
    const char** variable;
    const char*  value;
} modes_map[] = { { "G0", &new_gcode_modes.modal, "G0" },
                  { "G1", &new_gcode_modes.modal, "G1" },
                  { "G2", &new_gcode_modes.modal, "G2" },
                  { "G3", &new_gcode_modes.modal, "G3" },
                  { "G38.2", &new_gcode_modes.modal, "G38.2" },
                  { "G38.3", &new_gcode_modes.modal, "G38.3" },
                  { "G38.4", &new_gcode_modes.modal, "G38.4" },
                  { "G38.5", &new_gcode_modes.modal, "G38.5" },
                  { "G54", &new_gcode_modes.wcs, "G54" },
                  { "G55", &new_gcode_modes.wcs, "G55" },
                  { "G56", &new_gcode_modes.wcs, "G56" },
                  { "G57", &new_gcode_modes.wcs, "G57" },
                  { "G58", &new_gcode_modes.wcs, "G58" },
                  { "G59", &new_gcode_modes.wcs, "G59" },
                  { "G17", &new_gcode_modes.plane, "XY" },
                  { "G18", &new_gcode_modes.plane, "YZ" },
                  { "G19", &new_gcode_modes.plane, "ZX" },
                  { "G20", &new_gcode_modes.units, "In" },
                  { "G21", &new_gcode_modes.units, "mm" },
                  { "G90", &new_gcode_modes.distance, "Abs" },
                  { "G91", &new_gcode_modes.distance, "Rel" },
                  { "M0", &new_gcode_modes.program, "Pause" },
                  { "M1", &new_gcode_modes.program, "?Pause" },
                  { "M2", &new_gcode_modes.program, "Rew" },
                  { "M30", &new_gcode_modes.program, "End" },
                  { "M3", &new_gcode_modes.spindle, "CW" },
                  { "M4", &new_gcode_modes.spindle, "CCW" },
                  { "M5", &new_gcode_modes.spindle, "Off" },
                  { "M7", &new_gcode_modes.coolant, "Mist" },
                  { "M8", &new_gcode_modes.coolant, "Flood" },
                  { "M9", &new_gcode_modes.coolant, "Off" },
                  { "M56", &new_gcode_modes.parking, "Ovr" },
                  { NULL, NULL, NULL } };

static void lookup_mode(const char* tag) {
    for (struct GCodeMode* p = modes_map; p->tag; p++) {
        if (strcmp(tag, p->tag) == 0) {
            *p->variable = p->value;
            return;
        }
    }
}

static void parse_gcode_report(char* tag) {
    // Wrapper, already removed, is [GC: ...]
    // Body is, for example, G0 G54 G17 G21 G90 G94 M5 M9 T0 F0.0 S0
    char* next;
    do {
        split(tag, &next, ' ');
        if (strlen(tag) > 1) {
            switch (*tag) {
                case 'T':
                    new_gcode_modes.tool = atoi(tag + 1);
                    break;
                case 'F':
                    //Fnew_gcode_modes.feed = simple_atof(tag + 1);
                    break;
                case 'S':
                    new_gcode_modes.spindle_speed = atoi(tag + 1);
                    break;
                case 'G':
                case 'M':
                    lookup_mode(tag);
                    break;
            }
        }

        // G80 G0 G1 G2 G3  G38.2 G38.3 G38.4 G38.5
        // G54 .. G59
        // G17 G18 G19
        // G20 G21
        // G90 G91
        // G94 G93
        // M0 M1 M2 M30
        // M3 M4 M5
        // M7 M8 M9
        // M56
        // Tn
        // Fn
        // Sn
        //        if (tag == "G0") {
        //            continue;
        //        }
        tag = next;
    } while (*tag);
    if (memcmp(&new_gcode_modes, &old_gcode_modes, sizeof(new_gcode_modes)) == 0) {
        memcpy(&old_gcode_modes, &new_gcode_modes, sizeof(struct gcode_modes));
    }
    show_gcode_modes(&new_gcode_modes);
}

void fnc_send_line(const char* line, int timeout_ms) {
    while (_ackwait) {
        if ((milliseconds() - _ack_time_limit) >= 0) {
            show_timeout();
            _ackwait = false;
        } else {
            fnc_poll();
        }
    }
    char c;
    while ((c = *line++) != '\0') {
        fnc_putchar(c);
    }
    fnc_putchar('\n');
    _ack_time_limit = milliseconds() + timeout_ms;
    _ackwait        = true;
}

void fnc_realtime(realtime_cmd_t c) {
    fnc_putchar((uint8_t)c);
}

static void parse_report() {
    if (*_report == '\0') {
        return;
    }

    if (strcmp(_report, "ok") == 0) {
        _ackwait = false;
        show_ok();
        return;
    }

    char* body;
    if (is_report_type(_report, &body, "<", ">")) {
        parse_status_report(body);
        return;
    }
    if (is_report_type(_report, &body, "[GC:", "]")) {
        parse_gcode_report(body);
        return;
    }
    if (is_report_type(_report, &body, "[MSG:", "]")) {
        parse_msg(body);
        return;
    }
    if (is_report_type(_report, &body, "error:", "")) {
        _ackwait = false;
        parse_error(body);
        return;
    }
    if (is_report_type(_report, &body, "ALARM:", "")) {
        parse_alarm(body);
        return;
    }

    if (is_report_type(_report, &body, "Grbl ", "")) {
        parse_signon(body);
        return;
    }

    handle_other(_report);
}
// Receive an incoming byte
void collect(uint8_t data) {
    char c = data;
    if (c == '\r') {
        return;
    }
    if (c == '\n') {
        parse_report();
        _report[0]  = '\0';
        _report_len = 0;
        return;
    }
    _report[_report_len++] = c;
    _report[_report_len]   = '\0';
}

void fnc_poll() {
    int c;
    if ((c = fnc_getchar()) >= 0) {
        collect(c);
    }
    poll_extra();
}

void fnc_wait_ready() {
    // XXX we need to figure out how to do this.  The pendant
    // typically starts faster than FluidNC
}

// Implement this to do anything that must be done while waiting for characters
void __attribute__((weak)) poll_extra() {};

// Implement these to handle specific kinds of messages from FluidNC
void __attribute__((weak)) show_alarm(int alarm) {}
void __attribute__((weak)) show_error(int error) {}
void __attribute__((weak)) show_ok() {}
void __attribute__((weak)) show_timeout() {}

// Handle [MSG: messages
// If you do not override it, it will handle IO expander messages.
// You can override it to handle whatever you want.  The override
// can first call handle_expander_msg(), which will return true if an
// expander message was handled.
void __attribute__((weak)) handle_msg(char* command, char* arguments) {}

void __attribute__((weak)) handle_signon(char* version, char* extra) {}
void __attribute__((weak)) handle_other(char* line) {}

// Data parsed from <...> status reports
void __attribute__((weak)) show_limits(bool probe, const bool* limits, size_t n_axis) {};
void __attribute__((weak)) show_state(const char* state) {};
void __attribute__((weak)) show_dro(const pos_t* axes, const pos_t* wcos, bool isMpos, bool* limits, size_t n_axis) {}
void __attribute__((weak)) show_file(const char* filename, file_percent_t percent) {}
void __attribute__((weak)) show_spindle_coolant(int spindle, bool flood, bool mist) {}
void __attribute__((weak)) show_feed_spindle(uint32_t feedrate, uint32_t spindle_speed) {}
void __attribute__((weak)) show_overrides(override_percent_t feed_ovr, override_percent_t rapid_ovr, override_percent_t spindle_ovr) {}
// [GC: messages
void __attribute__((weak)) show_gcode_modes(struct gcode_modes* modes) {}

// Called before and after parsing a status report; useful for
// clearing and updating display screens
void __attribute__((weak)) begin_status_report() {}
void __attribute__((weak)) end_status_report() {}

// used for optional debugging or pendants and smart displays
void __attribute__((weak)) debug_putchar(char c) {}
void __attribute__((weak)) debug_print(const char* msg) {}
void __attribute__((weak)) debug_println(const char* msg) {}
int __attribute__((weak)) debug_getchar() {
    return -1;
}

#ifdef __cplusplus
}
#endif

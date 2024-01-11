// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "FluidNCModel.h"
#include <map>
#include "System.h"
#include "Scene.h"

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
int                lastError          = 0;
uint32_t           errorExpire;

// clang-format off
// Maps the state strings in status reports to internal state enum values
std::map<String, state_t> state_map = {
    { "Idle", Idle },
    { "Alarm", Alarm },
    { "Hold:0", Hold },
    { "Hold:1", Hold },
    { "Run", Cycle },
    { "Jog", Jog },
    { "Home", Homing },
    { "Door:0", SafetyDoor },
    { "Door:1", SafetyDoor },
    { "Check", CheckMode },
    { "Sleep", Sleep },
};
// clang-format on

state_t decode_state_string(const char* state_string) {
    if (stateString != state_string) {
        stateString       = state_string;
        state_t new_state = state_map[stateString];
        return new_state;
    }
    return state;
}

void set_disconnected_state() {
    state       = Disconnected;
    stateString = "N/C";
}

// clang-format off
std::map<int, String> error_map = {  // Do here so abreviations are right for the dial
    { 0, "None"},
    { 1, "GCode letter"},
    { 2, "GCode format"},
    { 3, "Bad $ command"},
    { 4, "Negative value"},
    { 5, "Setting Diabled"},
    { 10, "Soft limit error"},
    { 13, "Check door"},
    { 18, "No Homing Cycles"},
    { 20, "Unsupported GCode"},
    { 22, "Undefined feedrate"},
    { 19, "No single axis"},
    { 34, "Arc radius error"},
    { 39, "P Param Exceeded"},
};
// clang-format on

String decode_error_number(int error_num) {
    if (error_map.find(error_num) != error_map.end()) {
        return error_map[error_num];
    }
    return String(error_num);
}

extern "C" void begin_status_report() {
    myPercent = 0;
}

extern "C" void show_file(const char* filename, file_percent_t percent) {
    myPercent = percent;
}

extern "C" void show_overrides(override_percent_t feed_ovr, override_percent_t rapid_ovr, override_percent_t spindle_ovr) {
    myFro = feed_ovr;
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

void send_line(const String& s, int timeout) {
    send_line(s.c_str(), timeout);
}
void send_line(const char* s, int timeout) {
    fnc_send_line(s, timeout);
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

String modeString() {
    return myModeString;
}

extern "C" void show_state(const char* state_string) {
    state_t new_state = decode_state_string(state_string);
    if (state != new_state) {
        state = new_state;
        current_scene->onStateChange(state);
    }
}

extern "C" void show_error(int error) {
    errorExpire = millis() + 1000;
    lastError   = error;
    current_scene->reDisplay();
}

extern "C" void show_timeout() {}

extern "C" void end_status_report() {
    current_scene->onDROChange();
}

extern "C" void show_alarm(int alarm) {
    lastAlarm = alarm;
    current_scene->reDisplay();
}

extern "C" void show_gcode_modes(struct gcode_modes* modes) {
    myModeString = String(modes->wcs);
    myModeString += "|" + String(modes->units);
    myModeString += "|" + String(modes->distance);
    myModeString += "|" + String(modes->spindle);
    myModeString += "|" + String(modes->coolant);
    myModeString += "|T" + String(modes->tool);
    current_scene->reDisplay();
}

int disconnect_ms = 0;
int next_ping_ms  = 0;

// If we haven't heard from FluidNC in 4 seconds for some other reason,
// send a status report request.
const int ping_interval_ms = 4000;

// If we haven't heard from FluidNC in 6 seconds for any reason, declare
// FluidNC unresponsive.  After a ping, FluidNC has 2 seconds to respond.
const int disconnect_interval_ms = 6000;

bool starting = true;

void request_status_report() {
    fnc_realtime(StatusReport);  // Request fresh status
    next_ping_ms = milliseconds() + ping_interval_ms;
}

bool fnc_is_connected() {
    int now = milliseconds();
    if (starting) {
        starting      = false;
        disconnect_ms = now + (disconnect_interval_ms - ping_interval_ms);
        request_status_report();  // sets next_ping_ms
        return false;             // Do we need a value for "unknown"?
    }
    if ((now - disconnect_ms) >= 0) {
        next_ping_ms  = now + ping_interval_ms;
        disconnect_ms = now + disconnect_interval_ms;
        return false;
    }
    if ((now - next_ping_ms) >= 0) {
        request_status_report();
    }
    return true;
}

void update_rx_time() {
    int now       = milliseconds();
    next_ping_ms  = now + ping_interval_ms;
    disconnect_ms = now + disconnect_interval_ms;
}

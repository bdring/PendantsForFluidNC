// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "FluidNCModel.h"
#include <map>

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
uint32_t           errorExpire        = 0;
int                lastError          = 0;
gcode_modes        myGcModes;

// clang-format off
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

void decode_state_string(const char* state_string) {
    if (stateString != state_string) {
        stateString = state_string;
        state       = state_map[stateString];
    }
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
void log_msg(const String& s) {
#ifdef DEBUG_TO_FNC
    send_line("$Msg/Uart0=" + s);
#endif
#ifdef DEBUG_TO_USB
    USBSerial.println(s);
#endif
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

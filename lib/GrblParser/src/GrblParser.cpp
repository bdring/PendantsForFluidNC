#include <Arduino.h>
#include "GrblParser.h"

void GrblParser::parse_report()
{
    if (_report.length() == 0)
    {
        return;
    }
    if (_report.startsWith("<"))
    {
        parse_status_report();
        return;
    }
    if (_report.startsWith("[GC:"))
    {
        parse_gcode_report();
        return;
    }
    if (_report.startsWith("[MSG:"))
    {
        parse_message(_report);
    }
    if (_report.startsWith("error:"))
    {
        parse_error();
    }
    if (_report == "ok")
    {
        show_ok();
    }
}

void GrblParser::parse_error()
{
    _last_error = _report.substring(strlen("error:")).toInt();
    show_error(_last_error);
}
void GrblParser::parse_status_report()
{
    // example <Idle|MPos:151.000,149.000,-1.000|Pn:XP|FS:0,0|WCO:12.000,28.000,78.000>
    if (_report.endsWith(">"))
    {
        _report.remove(_report.length() - 1);
    }
    // Now the string is a sequence of field|field|field
    size_t pos = 0;
    auto nextpos = _report.indexOf("|", pos);
    if (nextpos == -1)
    {
        return;
    }
    _state = _report.substring(pos + 1, nextpos);

    bool probe = false;
    bool limits[MAX_N_AXIS] = {false};

    float axes[MAX_N_AXIS];
    bool isMpos = false;
    _filename = "";

    // ... handle it
    while (nextpos != -1)
    {
        pos = nextpos + 1;
        nextpos = _report.indexOf("|", pos);
        auto field = _report.substring(pos, nextpos);
        // MPos:, WPos:, Bf:, Ln:, FS:, Pn:, WCO:, Ov:, A:, SD: (ISRs:, Heap:)
        auto colon = field.indexOf(":");
        String tag, value;
        if (colon == -1)
        {
            tag = field;
            value = "";
        }
        else
        {
            tag = field.substring(0, colon);
            value = field.substring(colon + 1);
        }
        if (tag == "MPos")
        {
            // x,y,z,...
            parse_axes(value, axes);
            isMpos = true;
            continue;
        }
        if (tag == "WPos")
        {
            // x,y,z...
            parse_axes(value, axes);
            isMpos = false;
            continue;
        }
        if (tag == "Bf")
        {
            // buf_avail,rx_avail
            continue;
        }
        if (tag == "Ln")
        {
            // n
            _linenum = value.toInt();
            continue;
        }
        if (tag == "FS")
        {
            // feedrate,spindle_speed
            float fs[2];
            parse_numbers(value, fs, 2); // feed in [0], spindle in [1]
            continue;
        }
        if (tag == "Pn")
        {
            // PXxYy etc
            for (char const &c : value)
            {
                switch (c)
                {
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
        if (tag == "WCO")
        {
            // x,y,z,...
            // We do not use the WCO values because the DROs show whichever
            // position is in the status report
            // float wcos[MAX_N_AXIS];
            // auto  wcos = parse_axes(value, wcos);
            continue;
        }
        if (tag == "Ov")
        {
            // feed_ovr,rapid_ovr,spindle_ovr
            float frs[3];
            parse_numbers(value, frs, 3); // feed in [0], rapid in [1], spindle in [2]
            continue;
        }
        if (tag == "A")
        {
            // SCFM
            _spindle = 0;
            _flood = false;
            _mist = false;
            for (char const &c : value)
            {
                switch (c)
                {
                case 'S':
                    _spindle = 1;
                    break;
                case 'C':
                    _spindle = 2;
                    break;
                case 'F':
                    _flood = true;
                    break;
                case 'M':
                    _mist = true;
                    break;
                }
            }
            continue;
        }
        if (tag == "SD")
        {
            auto commaPos = value.indexOf(",");
            if (commaPos == -1)
            {
                _percent = value.toFloat();
                _filename = "";
            }
            else
            {
                _percent = value.substring(0, commaPos).toFloat();
                _filename = value.substring(commaPos + 1);
            }
            continue;
        }
    }

    begin_status_report();
    show_state(_state);
    show_file();
    show_limits(probe, limits);
    show_dro(axes, isMpos, limits);
    end_status_report();
}

static struct GGodeMode
{
    const char *tag;
    const char *GrblParser::gcode_modes::*variable;
    const char *value;
} modes_map[] = {
    {"G0", &GrblParser::gcode_modes::modal, "G0"},
    {"G1", &GrblParser::gcode_modes::modal, "G1"},
    {"G2", &GrblParser::gcode_modes::modal, "G2"},
    {"G3", &GrblParser::gcode_modes::modal, "G3"},
    {"G38.2", &GrblParser::gcode_modes::modal, "G38.2"},
    {"G38.3", &GrblParser::gcode_modes::modal, "G38.3"},
    {"G38.4", &GrblParser::gcode_modes::modal, "G38.4"},
    {"G38.5", &GrblParser::gcode_modes::modal, "G38.5"},
    {"G54", &GrblParser::gcode_modes::wcs, "G54"},
    {"G55", &GrblParser::gcode_modes::wcs, "G55"},
    {"G56", &GrblParser::gcode_modes::wcs, "G56"},
    {"G57", &GrblParser::gcode_modes::wcs, "G57"},
    {"G58", &GrblParser::gcode_modes::wcs, "G58"},
    {"G59", &GrblParser::gcode_modes::wcs, "G59"},
    {"G17", &GrblParser::gcode_modes::plane, "XY"},
    {"G18", &GrblParser::gcode_modes::plane, "YZ"},
    {"G19", &GrblParser::gcode_modes::plane, "ZX"},
    {"G20", &GrblParser::gcode_modes::units, "In"},
    {"G21", &GrblParser::gcode_modes::units, "mm"},
    {"G90", &GrblParser::gcode_modes::distance, "Abs"},
    {"G91", &GrblParser::gcode_modes::distance, "Rel"},
    {"M0", &GrblParser::gcode_modes::program, "Pause"},
    {"M1", &GrblParser::gcode_modes::program, "?Pause"},
    {"M2", &GrblParser::gcode_modes::program, "Rew"},
    {"M30", &GrblParser::gcode_modes::program, "End"},
    {"M3", &GrblParser::gcode_modes::spindle, "CW"},
    {"M4", &GrblParser::gcode_modes::spindle, "CCW"},
    {"M5", &GrblParser::gcode_modes::spindle, "Off"},
    {"M7", &GrblParser::gcode_modes::coolant, "Mist"},
    {"M8", &GrblParser::gcode_modes::coolant, "Flood"},
    {"M9", &GrblParser::gcode_modes::coolant, "Off"},
    {"M56", &GrblParser::gcode_modes::parking, "Ovr"},
};
void GrblParser::lookup_mode(const String &tag, gcode_modes &modes)
{
    for (auto m : modes_map)
    {
        if (tag == m.tag)
        {
            modes.*m.variable = m.value;
            return;
        }
    }
}

void GrblParser::parse_gcode_report()
{
    // [GC:G0 G54 G17 G21 G90 G94 M5 M9 T0 F0.0 S0]
    int pos = 0;
    int nextpos = _report.indexOf(":", pos);
    if (nextpos == -1)
    {
        return;
    }
    auto name = _report.substring(pos, nextpos);
    if (name != "[GC")
    {
        return;
    }
    _report.remove(_report.length() - 1); // Remove trailing ]
    _report = _report.substring(nextpos + 1); // remove leadin
    pos = nextpos + 1;
    do
    {
        nextpos = _report.indexOf(" ", pos);
        String tag;
        if (nextpos == -1)
        {
            tag = _report.substring(pos);
        }
        else
        {
            tag = _report.substring(pos, nextpos);
        }
        if (tag.length() > 1)
        {
            switch (tag[0])
            {
            case 'T':
                new_gcode_modes.tool = tag.substring(1).toInt();
                break;
            case 'F':
                new_gcode_modes.feed = tag.substring(1).toFloat();
                break;
            case 'S':
                new_gcode_modes.spindle_speed = tag.substring(1).toInt();
                break;
            case 'G':
            case 'M':
                lookup_mode(tag, new_gcode_modes);
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
        pos = nextpos + 1;
    } while (nextpos != -1);
    if (memcmp(&new_gcode_modes, &old_gcode_modes, sizeof(new_gcode_modes)) == 0)
    {
        old_gcode_modes = new_gcode_modes;
        //show_gcode_modes(new_gcode_modes);
    }
    show_gcode_modes(new_gcode_modes);
}

void GrblParser::parse_axes(String s, float *axes)
{
    int pos = 0;
    int nextpos = -1;
    _n_axis = 0;
    do
    {
        nextpos = s.indexOf(",", pos);
        String num;
        if (nextpos == -1)
        {
            num = s.substring(pos);
        }
        else
        {
            num = s.substring(pos, nextpos);
        }
        if (_n_axis < MAX_N_AXIS)
        {
            axes[_n_axis++] = num.toFloat();
        }
        pos = nextpos + 1;
    } while (nextpos != -1);
}

void GrblParser::parse_numbers(String s, float *nums, int maxnums)
{
    int pos = 0;
    int nextpos = -1;
    int i = 0;
    do
    {
        if (i >= maxnums)
        {
            return;
        }
        nextpos = s.indexOf(",", pos);
        String num;
        if (nextpos == -1)
        {
            num = s.substring(pos);
        }
        else
        {
            num = s.substring(pos, nextpos);
        }
        nums[i++] = num.toFloat();
        pos = nextpos + 1;
    } while (nextpos != -1);
}

// Receive an incoming byte
size_t GrblParser::write(uint8_t data)
{
    char c = data;
    if (c == '\r')
    {
        return 1;
    }
    if (c == '\n')
    {
        parse_report();
        _report = "";
        return 1;
    }
    _report += c;
    return 1;
}
size_t GrblParser::write(const String &str)
{
    for (auto c : str)
    {
        write((uint8_t)c);
    }
    return str.length();
}

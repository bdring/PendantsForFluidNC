#pragma once

constexpr static const int MAX_N_AXIS = 6;
constexpr static const int X_AXIS     = 0;
constexpr static const int Y_AXIS     = 1;
constexpr static const int Z_AXIS     = 2;
constexpr static const int A_AXIS     = 3;
constexpr static const int B_AXIS     = 4;
constexpr static const int C_AXIS     = 5;

class GrblParser {
public:
    struct gcode_modes {
        const char* modal    = "";
        const char* wcs      = "";
        const char* plane    = "";
        const char* units    = "";
        const char* distance = "";
        const char* program  = "";
        const char* spindle  = "";
        const char* coolant  = "";
        const char* parking  = "";
        int         tool;
        uint32_t    spindle_speed;
        float       feed;
    };

private:
    String _report;

    bool is_report_type(const String& report, String& body, const char* prefix, const char* suffix);
    void parse_report();

    void parse_status_report(const String& body);
    void parse_gcode_report(const String& body);
    void parse_error(const String& body);
    void parse_msg(const String& body);

    void parse_axes(String s, float* axes);
    void parse_numbers(String s, float* nums, int maxnums);

    void lookup_mode(const String& tag, gcode_modes& modes);

    gcode_modes old_gcode_modes;
    gcode_modes new_gcode_modes;

public:
    int _n_axis;

    int  _linenum;
    int  _spindle;
    bool _flood;
    bool _mist;

    int _last_error = 0;

    String _state;
    String _filename;
    float  _percent;

    size_t write(uint8_t data);
    size_t write(const String& str);

    virtual void show_limits(bool probe, const bool* limits) {};
    virtual void show_state(const String& state) {};
    virtual void show_dro(const float* axes, bool isMpos, bool* limits) {}
    virtual void show_file(const String& filename) {}
    virtual void show_gcode_modes(const gcode_modes& modes) {}
    virtual void begin_status_report() {}
    virtual void end_status_report() {}
    virtual void show_error(int error) {}
    virtual void show_ok() {}

    virtual void handle_msg(const String& command, const String& arguments) {};
};

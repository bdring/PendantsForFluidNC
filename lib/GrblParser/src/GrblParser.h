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

    bool _ackwait = false;
    int  _ack_time_limit;

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

    // Send received characters to the parser.  Typically you do not
    // need to call these explicitly, because poll() does it for you
    size_t collect(uint8_t data);
    size_t collect(const String& str);

    // Wait until FluidNC is ready
    void wait_ready();

    // This is called from Arduino loop(), and also from send_line() while
    // waiting for an ok from FluidNC.  You probably do not need to call
    // it explicitly.
    void poll();

    // Call this to send a line-oriented command to FluidNC
    void send_line(const String& line, int timeout_ms = 2000);

    // Implement these to send and receive characters from FluidNC
    virtual int  getchar()           = 0;
    virtual void putchar(uint8_t ch) = 0;

    // Implement this to get the time in milliseconds
    virtual int milliseconds() = 0;

    // Implement this to do anything that must be done while waiting for characters
    virtual void poll_extra() {};

    // Implement these to handle specific kinds of messages from FluidNC
    virtual void show_error(int error) {}
    virtual void show_ok() {}
    virtual void show_timeout() {}

    // [MSG: messages
    virtual void handle_msg(const String& command, const String& arguments) {};

    // Data parsed from <...> status reports
    virtual void show_limits(bool probe, const bool* limits) {};
    virtual void show_state(const String& state) {};
    virtual void show_dro(const float* axes, bool isMpos, bool* limits) {}
    virtual void show_file(const String& filename) {}

    // [GC: messages
    virtual void show_gcode_modes(const gcode_modes& modes) {}

    // Called before and after parsing a status report; useful for
    // clearing and updating display screens
    virtual void begin_status_report() {}
    virtual void end_status_report() {}
};

#include <Arduino.h>
#include "GrblParser.h"

class Displayer : public GrblParser
{
    void show_state(const String &state)
    {
        Serial.print(state);
    }
    void show_dro(const float *axes, bool isMpos, bool *limits)
    {
        char delim = ' ';
        for (int i = 0; i < _n_axis; i++)
        {
            Serial.print(delim);
            delim = ',';
            Serial.print(axes[i]);
        }
    }
    void end_status_report()
    {
        Serial.println();
    }
} displayer;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial1.print("?");    // Initial status report
    Serial1.print("$G\n"); // Initial modes report
}
// #define ECHO_RX_DATA
// #define SEND_CONSOLE_DATA
void loop()
{
    while (Serial1.available())
    {
        char c = Serial1.read();
        if (c != '\r')
        {
#ifdef ECHO_RX_DATA
            Serial.print(c);
#endif
            displayer.write(c);
        }
    }
#ifdef SEND_CONSOLE_DATA
    while (Serial.available())
    {
        char c = Serial.read();
        if (c != '\r')
        {
            Serial1.write(c);
        }
    }
#endif
}

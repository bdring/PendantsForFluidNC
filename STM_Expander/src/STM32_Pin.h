#include <Arduino.h>
#include "main.h"
// This is a class to store some information on STM32 pins

class STM32_Pin
{

public:
    enum class Mode : uint8_t
    {
        None = 0,
        Input = 1,
        Output = 2,
        PWM = 3,
    };

    uint32_t stm_pin_num;
    bool pwm_capable = false;
    bool initialized = false;
    Mode pin_mode = Mode::None;
    int last_value = -1; // unknown
    int debounce_ms = 100; // default
    uint32_t last_change_millis = 0;

    int init(String mode);
    int set_output(float val);

    bool read_pin();

private:
};
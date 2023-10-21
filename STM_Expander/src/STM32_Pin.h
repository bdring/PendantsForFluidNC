#include <Arduino.h>
// This is a class to store some information on STM32 pins

class STM32_Pin
{

    enum class Mode : uint8_t
    {
        None = 0,
        Input = 1, 
        Output = 2,
        PWM  = 3,
    };

public:
    uint32_t stm_pin_num;
    bool pwm_capable = false;
    bool initialized = false;
    Mode pin_mode = Mode::None; 

    int init(String mode);
    int set_output(float val);

private:
};
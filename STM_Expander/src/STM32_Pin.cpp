#include "STM32_Pin.h"

int STM32_Pin::set_output(float val)
{
    if (!initialized)
    {
        return -1;
    }

    if (pin_mode == Mode::Output)
    {
        if (val > 0)
        {
            val = 1.0;
        }
        digitalWrite(stm_pin_num, (uint32_t)val);
    }
    else
    {
        uint32_t pwm_val = map((uint32_t)(val * 10.0), 0, 1000, 0, 255); // map 0.0-100.0 to 0 to 255
        analogWrite(stm_pin_num, pwm_val);
    }

    return 0;
}

int STM32_Pin::init(String params)
{
    if (params.indexOf("out") != -1)
    {
        pinMode(stm_pin_num, OUTPUT);
        pin_mode = Mode::Output;
        initialized = true;
        return 0;
    }
    if (params.indexOf("pwm") != -1)
    {
        if (!pwm_capable)
        {
            return -1;
        }
        pinMode(stm_pin_num, OUTPUT);
        pin_mode = Mode::PWM;
        initialized = true;
        return 0;
    }
    if (params.indexOf("inp") != -1)
    {
        if (params.indexOf("pu"))
        {
            pinMode(stm_pin_num, INPUT_PULLUP);
        }
        else if (params.indexOf("pd"))
        {
            pinMode(stm_pin_num, INPUT_PULLDOWN);
        }
        else
        {
            pinMode(stm_pin_num, INPUT);
        }

        //attachInterrupt(digitalPinToInterrupt(stm_pin_num), pin_change, CHANGE);

        pin_mode = Mode::Input;
        initialized = true;
        return 0;
    }

    return -1;
}

void STM32_Pin::pin_change() {

}

#include <io_controller.h>

STM32_Pin pins[PIN_COUNT];

void io_init()
{

    // map the pins to the FNC numbers
    // This order can be any way, but this matches my extender
    pins[0].stm_pin_num = PA4;
    pins[1].stm_pin_num = PA5;
    pins[2].stm_pin_num = PA8;
    pins[3].stm_pin_num = PA11;
    pins[4].stm_pin_num = PA12;
    pins[5].stm_pin_num = PB6;
    pins[6].stm_pin_num = PB7;
    pins[7].stm_pin_num = PB8;
    pins[8].stm_pin_num = PB9;
    pins[9].stm_pin_num = PB10;
    pins[10].stm_pin_num = PB11;
    pins[11].stm_pin_num = PB14;
    pins[12].stm_pin_num = PB15;
    pins[13].stm_pin_num = PC13;
    pins[14].stm_pin_num = PA0;
    pins[15].stm_pin_num = PA1;
    pins[16].stm_pin_num = PA6;
    pins[17].stm_pin_num = PA7;
    pins[18].stm_pin_num = PB0;
    pins[19].stm_pin_num = PB1;

    // Which ones are pwm capable
    pins[2].pwm_capable = true;
    pins[5].pwm_capable = true;
    pins[6].pwm_capable = true;
    pins[7].pwm_capable = true;
    pins[8].pwm_capable = true;
    pins[14].pwm_capable = true;
    pins[15].pwm_capable = true;
    pins[16].pwm_capable = true;
    pins[17].pwm_capable = true;
    pins[18].pwm_capable = true;
    pins[19].pwm_capable = true;
}

void read_all_pins(bool forceUpdate)
{
    for (uint8_t i = 0; i < PIN_COUNT; i++)
    {
        if (pins[i].pin_mode == STM32_Pin::Mode::Input)
        {
            if (pins[i].read_pin(forceUpdate)) // returns true for a value change
            {
                if (pins[i].last_value == 1) {
                    Serial_FNC.write(PinHigh);
                } else {
                    Serial_FNC.write(PinLow);
                }
                Serial_FNC.write(i);
                //Serial_Pendant.printf("Change on pin:%d to %d\r\n", i, pins[i].last_value);          
            }
        }
    }
}

#include <io_controller.h>

static struct PinMapper
{
    uint32_t fnc_pin_num;
    uint32_t stm_pin_num;
} pin_map[] = {
    {0, PA4},
    {1, PA5},
    {2, PA8},
    {3, PA11},
    {5, PA12},
    {5, PB6},
    {6, PB7},
    {7, PB8},
    {8, PB9},
    {9, PB10},
    {10, PB11},
    {11, PB14},
    {12, PB15},
    {13, PC13},
    {14, PA0},
    {15, PA1},
    {16, PA6},
    {17, PA7},
    {18, PB0},
    {19, PB1},
};

STM32_Pin pins[64];

void io_init() {

    // map the pins to the FNC numbers
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
    pins[14].pwm_capable = true;
    pins[15].pwm_capable = true;
    pins[16].pwm_capable = true;
    pins[17].pwm_capable = true;
    pins[18].pwm_capable = true;
    pins[19].pwm_capable = true;

}

uint32_t get_STM_pin(uint32_t fnc_pin)
{
    for (auto m : pin_map)
    {
        if (fnc_pin == m.fnc_pin_num)
        {
            return m.stm_pin_num;
        }
    }
    // TO DO return fail code
    return 0;  
}

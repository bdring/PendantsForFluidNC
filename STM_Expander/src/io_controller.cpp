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

uint32_t get_STM_pin(uint32_t fnc_pin)
{
    for (auto m : pin_map)
    {
        if (fnc_pin == m.fnc_pin_num)
        {
            return m.stm_pin_num;
        }
    }
}

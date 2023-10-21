#include <Arduino.h>
#include "STM32_Pin.h"


extern STM32_Pin pins[64];;

void io_init();

uint32_t get_STM_pin(uint32_t fnc_pin);

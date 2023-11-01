#include <Arduino.h>
#include "STM32_Pin.h"

#define PIN_COUNT 20

const uint8_t PinLow = 0xB0;
const uint8_t PinHigh = 0xB1;

extern STM32_Pin pins[PIN_COUNT];

void io_init();

void read_all_pins();

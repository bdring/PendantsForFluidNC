#include <Arduino.h>
#include "STM32_Pin.h"

#define PIN_COUNT 20

const uint8_t PinLow = 0xB0;
const uint8_t PinHigh = 0xB1;
const uint8_t NAK = 0xB2;
const uint8_t ACK = 0xB3;

extern STM32_Pin pins[PIN_COUNT];

void io_init();

void deinit_all_pins();
void read_pin(size_t pin_num, bool forceUpdate);
void read_all_pins(bool forceUpdate);
void protocolRespond(bool ok);
bool valid_pin_number(int pin_num);
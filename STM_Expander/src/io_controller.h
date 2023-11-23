#include <Arduino.h>
#include "STM32_Pin.h"

#define PIN_COUNT 20

// The integer value for
// pin low reports is 0x100 to 0x13f, and for
// pin high reports is 0x140 to 0x17f
// In UTF8, 0x100 to 0x13f encodes to 0xC4 0x80+N
// and 0x140 to 0x17f encodes to 0xC5 0x80+N
const uint8_t PinLowUTF8Prefix  = 0xC4;
const uint8_t PinHighUTF8Prefix = 0xC5;

const uint8_t ACK = 0xB2;
const uint8_t NAK = 0xB3;

extern STM32_Pin pins[PIN_COUNT];

void io_init();

typedef void (*pin_msg_t)(int pin_num, bool active);

void update_all_pins();
void deinit_all_pins();
void read_pin(pin_msg_t, size_t pin_num);
void read_all_pins(pin_msg_t);
void protocolRespond(bool ok);
bool valid_pin_number(int pin_num);

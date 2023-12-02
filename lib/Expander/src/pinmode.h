#pragma once
#include <stdint.h>

typedef uint32_t pin_mode_t;

#define PIN_INPUT     (1 << 0)
#define PIN_OUTPUT    (1 << 1)
#define PIN_PWM       (1 << 2)
#define PIN_PULLUP    (1 << 3)
#define PIN_PULLDOWN  (1 << 4)
#define PIN_ACTIVELOW (1 << 5)

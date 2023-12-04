// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"
#include "gpiomap.h"

// The pins table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

#ifdef __cplusplus
extern "C" {
#endif

pin_t gpios[NUM_DIGITAL_PINS];

#ifndef pinIsAnalogInput
#    define pinIsAnalogInput(i) false
#endif

void init_gpiomap() {
    for (size_t i = 0; i < NUM_DIGITAL_PINS; i++) {
        gpio_pin_t gpio = { i, pinIsAnalogInput(i) };
        gpios[i].gpio   = gpio;
    }
}

#ifdef __cplusplus
}
#endif

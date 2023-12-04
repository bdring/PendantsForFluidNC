// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "pin.h"
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

extern pin_t gpios[];

extern void init_gpiomap();

#define n_pins NUM_DIGITAL_PINS

#ifdef __cplusplus
}
#endif

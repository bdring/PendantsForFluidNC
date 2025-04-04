// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t pin_mode_t;

#define PIN_INPUT (1 << 0)
#define IN PIN_INPUT
#define PIN_OUTPUT (1 << 1)
#define OUT PIN_OUTPUT
#define PIN_PWM (1 << 2)
#define PWM PIN_PWM
#define PIN_PULLUP (1 << 3)
#define PU PIN_PULLUP
#define PIN_PULLDOWN (1 << 4)
#define PD PIN_PULLDOWN
#define PIN_ACTIVELOW (1 << 5)

#define PIN_FREQ_SHIFT 8

#ifdef __cplusplus
}
#endif

// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "pin.h"

extern pin_t gpios[];

extern const int n_pins;

extern const char* fw_version;
extern const char* board_name;

void ready();

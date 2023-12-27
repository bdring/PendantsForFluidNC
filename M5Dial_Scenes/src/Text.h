// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Simplified ways to display text, hiding details of the graphics layer

#pragma once
#include <Arduino.h>
#include "M5Dial.h"

const int           WIDTH           = 240;
const int           HEIGHT          = 240;
constexpr const int CENTER          = WIDTH / 2;
const int           VERTICAL_CENTER = HEIGHT / 2;

enum fontnum_t {
    TINY        = 0,
    SMALL       = 1,
    MEDIUM      = 2,
    LARGE       = 3,
    MEDIUM_MONO = 4,
};

extern M5Canvas canvas;

void text(const String& msg, int x, int y, int color, fontnum_t fontnum = TINY, int datum = middle_center);
void centered_text(const String& msg, int y, int color = WHITE, fontnum_t fontnum = TINY);

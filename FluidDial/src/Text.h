// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Simplified ways to display text, hiding details of the graphics layer

#pragma once
#include "System.h"
#include "Point.h"

enum fontnum_t {
    TINY        = 0,
    SMALL       = 1,
    MEDIUM      = 2,
    LARGE       = 3,
    MEDIUM_MONO = 4,
};

bool text_fits(const String& txt, fontnum_t fontnum, int w);
// adjusts text to fit in (w) display area. reduces font size until it. tryfonts::false just uses fontnum
void auto_text(const String& txt, int x, int y, int w, int color, fontnum_t fontnum = MEDIUM,
               int datum = middle_center, bool tryfonts = true, bool trimleft = false);

void text(const String& msg, int x, int y, int color, fontnum_t fontnum = TINY, int datum = middle_center);
void text(const String& msg, Point xy, int color, fontnum_t fontnum = TINY, int datum = middle_center);
void centered_text(const String& msg, int y, int color = WHITE, fontnum_t fontnum = TINY);

// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Text.h"
#include <map>

const GFXfont* font[] = {
    // lgfx::v1::IFont* font[] = {
    &fonts::FreeSansBold9pt7b,   // TINY
    &fonts::FreeSansBold12pt7b,  // SMALL
    &fonts::FreeSansBold18pt7b,  // MEDIUM
    &fonts::FreeSansBold24pt7b,  // LARGE
    &fonts::FreeMonoBold18pt7b,  // MEDIUM_MONO
};

void text(const String& msg, int x, int y, int color, fontnum_t fontnum, int datum) {
    canvas.setFont(font[fontnum]);
    canvas.setTextDatum(datum);
    canvas.setTextColor(color);
    canvas.drawString(msg, x, y);
}
void text(const String& msg, Point xy, int color, fontnum_t fontnum, int datum) {
    Point dispxy = xy.to_display();
    text(msg, dispxy.x, dispxy.y, color, fontnum, datum);
}
void centered_text(const String& msg, int y, int color, fontnum_t fontnum) {
    text(msg, display.width() / 2, y, color, fontnum);
}

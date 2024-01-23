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

bool text_fits(const String& txt, fontnum_t fontnum, int w) {
    canvas.setFont(font[fontnum]);
    if (canvas.textWidth(txt) <= w)
        return true;
    return false;
}

void auto_text(const String& txt, int x, int y, int w, int color, fontnum_t fontnum, int datum,
                                  bool tryfonts, bool trimleft) {
    bool doesnotfit = true;
    while (true) { // forever loop
        int f = fontnum;
        if (text_fits(txt, fontnum, w)) {
            doesnotfit = false;
            break;
        }
        if (fontnum && tryfonts)
            fontnum = (fontnum_t) (--f);
        else
            break;
    }
    
    String s(txt);

    if (doesnotfit) {
        int dotswidth = canvas.textWidth(" ...");

        while (s.length() > 4) {
            if (trimleft)
                s.remove(0,1);
            else
                s.remove(s.length()-1);
            if (canvas.textWidth(s)+dotswidth <= w) {
                if (trimleft) {
                    s = String("... " + s);
                    USBSerial.printf("trimleft: %s\r\n", s.c_str());
                } else {
                    s.concat(" ...");
                }
                break;
            }
        }
    }

    text(s, x, y, color, fontnum, datum);
    }

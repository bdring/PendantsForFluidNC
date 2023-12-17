// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Factors for drawing parts of the pendant display

#pragma once
#include <Arduino.h>
#include "M5Dial.h"
#include "FluidNCModel.h"
#include "Text.h"

// draw stuff
void drawBackground(int color);
void drawStatus();
void drawOutlinedRect(int x, int y, int width, int height, int bgcolor, int outlinecolor);
void drawButton(int x, int y, int width, int height, fontnum_t fontnum, const String& text, bool highlighted);
void drawLed(int x, int y, int radius, bool active);
void drawButtonLegends(const String& red, const String& green, const String& orange);
void drawMenuTitle(const String& name);
void refreshDisplay();

void drawDRO(int x, int y, int width, int axis, float value, bool highlighted);

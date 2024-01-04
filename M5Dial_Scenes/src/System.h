// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

// #define DEBUG_TO_FNC
#define DEBUG_TO_USB
#define ECHO_FNC_TO_DEBUG

#include <Arduino.h>
#include <LittleFS.h>
#include "M5Dial.h"
#include "Encoder.h"

extern M5Canvas           canvas;
extern M5GFX&             display;
extern m5::Speaker_Class& speaker;
extern m5::Touch_Class&   touch;
extern ENCODER&           encoder;
extern String             myModeString;

extern Stream& debugPort;

String M5TouchStateName(m5::touch_state_t state_num);

void init_system();

void log_write(uint8_t c);
void log_print(const String& s);
void log_println(const String& s);

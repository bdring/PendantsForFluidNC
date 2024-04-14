// Copyright (c) 2024 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Math on numbers with an assumed exponent of -4 base 10.  This lets
// us use integer arithmetic for numbers with 4 decimal places.  The
// precision is 1/10000 of the base unit value, which is a good choice
// for displaying numbers in a GCode CNC system.
// For 32-bit integers, the range is -214748.3648 .. 214748.3647 .

// If the base unit is millimeters, the max range is about +-200
// meters, which is more than enough for any ordinary CNC machine.
// The range could extended simply by using different units, like
// meters in one direction or microns in the other.  Either way, the
// range is about 2 * 10^9 times the precision.

typedef int e4_t;

// Ordinary integer arithmetic can be used for
// addition and subtraction of E4 numbers.

#ifdef __cplusplus
extern "C" {
#endif

e4_t e4_power10(int power);
e4_t e4_scale(e4_t n, int num, int denom);
e4_t e4_mm_to_inch(e4_t mm);
e4_t e4_inch_to_mm(e4_t inch);
e4_t e4_magnitude(e4_t n1, e4_t n2);
e4_t e4_from_int(int n);

const char* e4_to_cstr(e4_t n, int ndecimals);

#ifdef __cplusplus
}
#endif

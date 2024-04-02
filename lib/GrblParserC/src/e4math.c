// Copyright (c) 2024 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Math for numbers with 4 decimal places (exponent -4) using integer arithmetic
// See e4math.h for an explanation.

#include "e4math.h"
#include <stdio.h>

static e4_t e4_powers[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

e4_t e4_power10(int power) {
    if (power < -4) {
        power = -4;
    } else if (power > 5) {
        power = 5;
    }
    return e4_powers[power + 4];
}
e4_t e4_scale(e4_t n, int num, int denom) {
    return n * num / denom;
}
e4_t e4_mm_to_inch(e4_t mm) {
    // Using 5 / 127 instead of 10 / 254 gives extra
    // overflow margin for the intermediate product
    return mm * 5 / 127;
    // For a rounded result, use (((mm * 10) / 127) + 1) / 2;
}
e4_t e4_inch_to_mm(e4_t inch) {
    return inch * 127 / 5;
}
e4_t e4_magnitude(e4_t n1, e4_t n2) {
    long long n = ((long long)n1 * n1) + ((long long)n2 * n2);
    long long x = n;
    long long y = 1;
    // printf("X %lld Y %lld\n", x, y);
    while (x > y) {
        x = (x + y) >> 1;
        // printf("x %lld y %lld\n", x, y);
        y = n / x;
        // printf("y' %lld\n", y);
    }
    // printf("x %lld\n", x);
    return x;
}
const char* e4_to_cstr(e4_t n, int ndecimals) {
    static char buf[12];
    buf[11] = '\0';
    char* p = &buf[11];

    // clip ndecimals to the range 0..4
    if (ndecimals < 0) {
        ndecimals = 0;
    } else if (ndecimals > 4) {
        ndecimals = 4;
    }

    // Convert to absolute value before rounding
    int isneg = n < 0;
    if (isneg) {
        n = -n;
    }

    // Discard non-displayed digits with rounding
    int omitted = 4 - ndecimals;
    // omitted ranges from 0 to 4
    if (omitted) {
        n += 5 * e4_powers[omitted - 1];  // Round
        n /= e4_powers[omitted];
    }

    // If it rounds to 0, we don't want a minus sign
    if (n == 0) {
        isneg = 0;
    }

    // Postdecimal digits
    for (int i = 0; i < ndecimals; ++i) {
        *--p = "0123456789"[n % 10];
        n /= 10;
    }

    // Decimal point unless ndecimals is 0
    if (ndecimals) {
        *--p = '.';
    }

    // Predecimal digits; at least one
    do {
        *--p = "0123456789"[n % 10];
        n /= 10;
    } while (n);

    // Minus sign
    if (isneg) {
        *--p = '-';
    }

    return p;
}

e4_t e4_from_int(int n) {
    return 10000 * n;
}
#if 0
void e4_test()
    for (int i = 0; i <= 4; i++) {
        printf("%s\n", e4ToCStr(912345, i));
    }
    for (int i = 0; i <= 4; i++) {
        printf("%s\n", e4ToCStr(-912345, i));
    }
    for (int i = 0; i <= 4; i++) {
        printf("%s\n", e4ToCStr(-912344, i));
    }
    for (int i = 0; i <= 4; i++) {
        printf("%s\n", e4ToCStr(-912346, i));
    }
    for (int i = 0; i <= 4; i++) {
        printf("%s\n", e4ToCStr(-002346, i));
    }
    for (int i = -5; i <= 9; i++) {
        printf("%s\n", e4ToCStr(e4Pow10(i), 4));
    }
}
#endif

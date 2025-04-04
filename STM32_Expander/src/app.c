// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Expander.h"
#include "stm32f1xx_hal.h"
#include "string.h"
#include "pwm_pin.h"
#include "gpio_pin.h"
#include "dma_uart.h"
#include "system.h"
#include "gpiomap.h"
#ifdef STARTUP_DEBUG
#    define DEBUG_PIN 8
#    define GH set_output(DEBUG_PIN, 1, 0);
#    define GL set_output(DEBUG_PIN, 0, 0);
#endif

int pass_getchar() {
    return dma_getchar(1);
}

void pass_print(const char* msg) {
    dma_print(1, msg);
}

void pass_println(const char* msg) {
    pass_print(msg);
    pass_print("\n");
}

// Interface routines for GrblParser

// Receive a byte from the serial port connected to FluidNC
int fnc_getchar() {
    return dma_getchar(0);
}

// Send a byte to the serial port connected to FluidNC
void fnc_putchar(uint8_t c) {
    dma_putchar(0, c);
}

// Return a value that increments every millisecond
int milliseconds() {
    return HAL_GetTick();
}

// Perform extra operations after the normal polling for input from FluidNC
void poll_extra() {
    int c;
    while ((c = pass_getchar()) != -1) {
        if (c != '\0') {  // Suppress nulls that can be caused by pendant startup messages at the wrong baud rate
            fnc_putchar(c);
        }
    }

    expander_poll();
}

// Handle IO Expander messages
void handle_report(char* report) {
    if (!expander_handle_command(report)) {
        pass_println(report);
    }
}

// void handle_signon(char* version, char* arguments) { }

extern TIM_HandleTypeDef htim5;

void delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
// Application initialization, called from main() in CubeMX/Core/Src/main.c after
// the basic driver setup code that CubeMX generated has finished.
void setup() {
    HAL_MspInit();
    SystemClock_Config();
    init_from_gpiomap();
    init_dma_uart(0, FNC_BAUD, GPIOA, GPIO_PIN_9, GPIOA, GPIO_PIN_10);
    while (dma_getchar(0) != -1) {
        // Drain the FluidNC buffer
    }
    init_dma_uart(1, PASSTHROUGH_BAUD, GPIOA, GPIO_PIN_2, GPIOA, GPIO_PIN_3);
    while (dma_getchar(1) != -1) {
        // Drain the pass-through buffer
    }

#ifdef STARTUP_DEBUG
    set_pin_mode(DEBUG_PIN, PIN_OUTPUT);
    GH;
    delay_ms(500);
    GL;
    delay_ms(500);
    GH;
    delay_ms(500);
    GL;
#endif
    expander_start();
    ready();
    fnc_wait_ready();
}

int main() {
    setup();
    while (1) {
        fnc_poll();
    }
    return 0;
}

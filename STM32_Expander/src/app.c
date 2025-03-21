// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "usart.h"
#include "Expander.h"
#include "stm32f1xx_hal.h"
#include "string.h"
#include "pwm_pin.h"

#include "gpio_pin.h"
#include "gpiomap.h"
#ifdef STARTUP_DEBUG
#    define DEBUG_PIN 8
#    define GH set_output(DEBUG_PIN, 1, 0);
#    define GL set_output(DEBUG_PIN, 0, 0);
#endif

UART_HandleTypeDef* FNCSerial   = &huart1;  // connects STM32 to ESP32 and FNC
UART_HandleTypeDef* DebugSerial = &huart2;  // connects STM32 to Debug terminal

// DMA ring buffer for the serial port connected to FluidNC
#define UART_DMA_LEN 8192
static int     last_dma_count = 0;
static uint8_t dma_buf[UART_DMA_LEN];

// Interface routines for interfacing with the pendant UART

bool pass_getchar(uint8_t* c) {
    return HAL_UART_Receive(DebugSerial, c, 1, 0) == HAL_OK;
}

void pass_print(const char* msg) {
    HAL_UART_Transmit(DebugSerial, (uint8_t*)msg, strlen(msg), 1000);
}

void pass_println(const char* msg) {
    pass_print(msg);
    pass_print("\n");
}

// Interface routines for GrblParser

// Receive a byte from the serial port connected to FluidNC
int fnc_getchar() {
    // The DMA-mode HAL UART driver receives data to a ring buffer.
    // We chase the buffer pointer and pull out the data.
    int count = last_dma_count - __HAL_DMA_GET_COUNTER(FNCSerial->hdmarx);
    if (count < 0) {
        count += UART_DMA_LEN;
    }
    if (count) {
        uint8_t c = dma_buf[UART_DMA_LEN - last_dma_count];
        --last_dma_count;
        if (last_dma_count < 0) {
            last_dma_count += UART_DMA_LEN;
        }
        return c;
    }
    return -1;
}

// Send a byte to the serial port connected to FluidNC
void fnc_putchar(uint8_t c) {
    HAL_UART_Transmit(FNCSerial, &c, 1, HAL_MAX_DELAY);
}

// Return a value that increments every millisecond
int milliseconds() {
    return HAL_GetTick();
}

// Perform extra operations after the normal polling for input from FluidNC
void poll_extra() {
    uint8_t c;
    while (pass_getchar(&c)) {
        fnc_putchar(c);
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
    HAL_UART_Receive_DMA(FNCSerial, dma_buf, UART_DMA_LEN);
    last_dma_count = UART_DMA_LEN;

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
    fnc_wait_ready();
}

// Application execution, called from the while(1) loop()
// in CubeMX/Core/Src/main.c
void loop() {
    for (int i = 100; i; --i) {
        fnc_poll();
    }
}

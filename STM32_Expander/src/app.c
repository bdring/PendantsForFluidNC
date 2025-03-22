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

// DMA UART driver
// DMA ring buffers
#define UART1_DMA_LEN 4096
uint8_t uart1_dma_buf[UART1_DMA_LEN];

#define UART2_DMA_LEN 4096
uint8_t uart2_dma_buf[UART2_DMA_LEN];

typedef struct {
    UART_HandleTypeDef* huart;
    int                 last_dma_count;
    int                 dma_len;
    uint8_t*            dma_buf;
} dma_uart_t;

dma_uart_t dma_uarts[2] = {
    { &huart1, UART1_DMA_LEN, UART1_DMA_LEN, uart1_dma_buf },
    { &huart2, UART2_DMA_LEN, UART2_DMA_LEN, uart2_dma_buf },
};

void init_dma_uart(int uart_num) {
    dma_uart_t* dma_uart     = &dma_uarts[uart_num];
    dma_uart->last_dma_count = dma_uart->dma_len;
    HAL_UART_Receive_DMA(dma_uart->huart, dma_uart->dma_buf, dma_uart->dma_len);
}

void dma_print(int uart_num, const char* msg) {
    HAL_UART_Transmit(dma_uarts[uart_num].huart, (uint8_t*)msg, strlen(msg), 1000);
}
void dma_putchar(int uart_num, uint8_t c) {
    HAL_UART_Transmit(dma_uarts[uart_num].huart, &c, 1, HAL_MAX_DELAY);
}
int dma_getchar(int uart_num) {
    dma_uart_t* dma_uart = &dma_uarts[uart_num];
    // The DMA-mode HAL UART driver receives data to a ring buffer.
    // We chase the buffer pointer and pull out the data.
    int count = dma_uart->last_dma_count - __HAL_DMA_GET_COUNTER(dma_uart->huart->hdmarx);
    if (count < 0) {
        count += dma_uart->dma_len;
    }
    if (count) {
        uint8_t c = dma_uart->dma_buf[dma_uart->dma_len - dma_uart->last_dma_count];
        --dma_uart->last_dma_count;
        if (dma_uart->last_dma_count < 0) {
            dma_uart->last_dma_count += dma_uart->dma_len;
        }
        return c;
    }
    return -1;
}

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
    init_dma_uart(0);
    init_dma_uart(1);

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

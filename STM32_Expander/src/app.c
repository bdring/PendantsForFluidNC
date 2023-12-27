// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "usart.h"
#include "Expander.h"
#include "stm32f1xx_hal.h"
#include "string.h"

UART_HandleTypeDef* FNCSerial   = &huart1;  // connects STM32 to ESP32 and FNC
UART_HandleTypeDef* DebugSerial = &huart2;  // connects STM32 to Debug terminal

// DMA ring buffer for the serial port connected to FluidNC
#define UART_DMA_LEN 8192
static int     last_dma_count = 0;
static uint8_t dma_buf[UART_DMA_LEN];

// Interface routines for interfacing with the pendant UART

void debug_print(const char* msg) {
    HAL_UART_Transmit(DebugSerial, (uint8_t*)msg, strlen(msg), 1000);
}

void debug_println(const char* msg) {
    debug_print(msg);
    debug_print("\r\n");
}

void debug_putchar(char c) {
    HAL_UART_Transmit(DebugSerial, (uint8_t*)&c, 1, 1000);
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
    while (HAL_UART_Receive(DebugSerial, &c, 1, 0) == HAL_OK) {
        fnc_putchar(c);
        collect(c);  // for testing from pendant terminal
    }

    expander_poll();
}

// Send MSG: messages to the IO Expander code for processing
void handle_msg(char* command, char* arguments) {
    if (!expander_handle_msg(command, arguments)) {
        debug_print("[MSG:");
        debug_print(command);
        debug_putchar(':');
        debug_print(arguments);
        debug_println("]");
    }
}

void handle_signon(char* version, char* arguments) {
    debug_print("Grbl ");
    debug_print(version);
    debug_print(" ");
    debug_println(arguments);
}
void handle_other(char* line) {
    debug_println(line);
}

// Application initialization, called from main() in CubeMX/Core/Src/main.c after
// the basic driver setup code that CubeMX generated has finished.
void setup() {
    HAL_UART_Receive_DMA(FNCSerial, dma_buf, UART_DMA_LEN);
    last_dma_count = UART_DMA_LEN;

    debug_println("[MSG:INFO: Hello from STM32_Expander]");
    fnc_wait_ready();
    // XXX we need some sort of message to tell FluidNC that the
    // expander has been reset.  At startup, that would be okay, but
    // if it happens later, it is probably an alarm condition because
    // the pins are now invalid.  Maybe the message should be a realtime
    // character to avoid the need to ack with an ok, since we cannot
    // depend on FluidNC to be ready when the expander starts.
}

// Application execution, called from the while(1) loop()
// in CubeMX/Core/Src/main.c
void loop() {
    fnc_poll();
}

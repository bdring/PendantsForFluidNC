#include "usart.h"
#include "Expander.h"
#include "stm32f1xx_hal.h"

UART_HandleTypeDef* FNCSerial   = &huart1;  // connects STM32 to ESP32 and FNC
UART_HandleTypeDef* DebugSerial = &huart2;  // connects STM32 to Debug terminal

// DMA ring buffer for the serial port connected to FluidNC
#define UART_DMA_LEN 256
static int     last_dma_count = 0;
static uint8_t dma_buf[UART_DMA_LEN];

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
    expander_handle_msg(command, arguments);
}

// Application initialization, called from main() in CubeMX/Core/Src/main.c after
// the basic driver setup code that CubeMX generated has finished.
void setup() {
    HAL_UART_Receive_DMA(FNCSerial, dma_buf, UART_DMA_LEN);
    last_dma_count = UART_DMA_LEN;

    HAL_UART_Transmit(DebugSerial, "Hello from STM32_Expander\r\n", 7, 1000);
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

// This makes the linker happy.  It won't be called because the
// while(1) loop in main.c never exits.
void _exit(int foo) {
    while (1) {}
}

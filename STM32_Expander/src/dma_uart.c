// Copyright (c) 2025 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// STM32 UART driver using DMA for reception

#include "gpio_pin.h"  // gpio_clock_enable()
#include "dma_uart.h"
#include <string.h>

// DMA ring buffers
#define UART1_DMA_LEN 4096
uint8_t uart1_dma_buf[UART1_DMA_LEN];

#define UART2_DMA_LEN 4096
uint8_t uart2_dma_buf[UART2_DMA_LEN];

typedef struct {
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef  hdma;
    int                last_dma_count;
    int                dma_len;
    uint8_t*           dma_buf;
} dma_uart_t;

dma_uart_t dma_uarts[2] = {
    { .huart = { .Instance = USART1 }, .last_dma_count = UART1_DMA_LEN, .dma_len = UART1_DMA_LEN, .dma_buf = uart1_dma_buf },
    { .huart = { .Instance = USART2 }, .last_dma_count = UART2_DMA_LEN, .dma_len = UART2_DMA_LEN, .dma_buf = uart2_dma_buf },
};

void init_dma_uart(int uart_num, int baud, GPIO_TypeDef* tx_port, uint16_t tx_pin, GPIO_TypeDef* rx_port, uint16_t rx_pin) {
    dma_uart_t* dma_uart = &dma_uarts[uart_num];

    UART_HandleTypeDef* huart = &(dma_uart->huart);

    __HAL_RCC_DMA1_CLK_ENABLE();

    DMA_HandleTypeDef* hdma = &(dma_uart->hdma);

    switch (uart_num) {
        case 0:
            __HAL_RCC_USART1_CLK_ENABLE();
            hdma->Instance = DMA1_Channel5;
            HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
            break;
        case 1:
            __HAL_RCC_USART2_CLK_ENABLE();
            hdma->Instance = DMA1_Channel6;
            HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
            break;
        default:
            return;
    }

    gpio_clock_enable(tx_port);
    gpio_clock_enable(rx_port);

    GPIO_InitTypeDef gpiomode;

    gpiomode.Pin   = tx_pin;
    gpiomode.Mode  = GPIO_MODE_AF_PP;
    gpiomode.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(tx_port, &gpiomode);

    gpiomode.Pin  = rx_pin;
    gpiomode.Mode = GPIO_MODE_INPUT;
    gpiomode.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(rx_port, &gpiomode);

    huart->Init.BaudRate     = baud;
    huart->Init.WordLength   = UART_WORDLENGTH_8B;
    huart->Init.StopBits     = UART_STOPBITS_1;
    huart->Init.Parity       = UART_PARITY_NONE;
    huart->Init.Mode         = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(huart) != HAL_OK) {
        return;
    }

    hdma->Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma->Init.MemInc              = DMA_MINC_ENABLE;
    hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma->Init.Mode                = DMA_CIRCULAR;
    hdma->Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(hdma) != HAL_OK) {
        return;
    }

    // This is an expansion of __HAL_LINKDMA that takes a dma handle pointer
    huart->hdmarx = hdma;
    hdma->Parent  = huart;
    // __HAL_LINKDMA(huart, hdmarx, hdma);

    dma_uart->last_dma_count = dma_uart->dma_len;
    HAL_UART_Receive_DMA(huart, dma_uart->dma_buf, dma_uart->dma_len);
}

void dma_print(int uart_num, const char* msg) {
    HAL_UART_Transmit(&(dma_uarts[uart_num].huart), (uint8_t*)msg, strlen(msg), 1000);
}
void dma_putchar(int uart_num, uint8_t c) {
    HAL_UART_Transmit(&(dma_uarts[uart_num].huart), &c, 1, HAL_MAX_DELAY);
}
int dma_getchar(int uart_num) {
    dma_uart_t* dma_uart = &dma_uarts[uart_num];
    // The DMA-mode HAL UART driver receives data to a ring buffer.
    // We chase the buffer pointer and pull out the data.
    int last  = dma_uart->last_dma_count;
    int this  = __HAL_DMA_GET_COUNTER(dma_uart->huart.hdmarx);
    int count = last - this;
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

void DMA1_Channel5_IRQHandler(void) {
    HAL_DMA_IRQHandler(&(dma_uarts[0].hdma));
}

void DMA1_Channel6_IRQHandler(void) {
    HAL_DMA_IRQHandler(&(dma_uarts[1].hdma));
}

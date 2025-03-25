#include "stm32f1xx_hal.h"
#include <stdint.h>

void init_dma_uart(int uart_num, int baud, GPIO_TypeDef* tx_port, uint16_t tx_pin, GPIO_TypeDef* rx_port, uint16_t rx_pin);
void dma_print(int uart_num, const char* msg);
void dma_putchar(int uart_num, uint8_t c);
int  dma_getchar(int uart_num);

#include "pin.h"
#include "gpiomap.h"

#define MAX_TIMER_NUM 4
TIM_HandleTypeDef timer_handles[MAX_TIMER_NUM + 1];
TIM_TypeDef*      timers[]                          = { 0, TIM1, TIM2, TIM3, TIM4 };
uint32_t          timer_channels[]                  = { 0, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };
uint16_t          timer_divisors[MAX_TIMER_NUM + 1] = { 0 };

#define TIMER_CLOCK 60000000
#define TIMER_RESOLUTION 999

bool Timer_Init(int timer_num, int frequency) {
    if (timer_num < 1 || timer_num > 4) {
        return false;
    }

    switch (timer_num) {
        case 1:
            __HAL_RCC_TIM1_CLK_ENABLE();
            break;
        case 2:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;
        case 3:
            __HAL_RCC_TIM3_CLK_ENABLE();
            break;
        case 4:
            __HAL_RCC_TIM4_CLK_ENABLE();
            break;
        default:
            return false;
    }

    TIM_ClockConfigTypeDef  sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig      = { 0 };

    uint32_t max_frequency = TIMER_CLOCK / TIMER_RESOLUTION;
    uint32_t divisor       = max_frequency / frequency;

    if (divisor == 0) {
        divisor = 1;
    }
    uint32_t existing_divisor = timer_divisors[timer_num];
    if (existing_divisor != 0) {  // This TIM is already configured
        // If the requested divisor is the same as the existing one,
        // we can just use the existing setup for this timer.
        // Otherwise we fail due to conflicting divisors for
        // different channels
        return existing_divisor == divisor;
    }

    TIM_HandleTypeDef* handle = &timer_handles[timer_num];

    handle->Instance               = timers[timer_num];
    handle->Init.Prescaler         = divisor - 1;
    handle->Init.CounterMode       = TIM_COUNTERMODE_UP;
    handle->Init.Period            = TIMER_RESOLUTION;
    handle->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(handle) != HAL_OK) {
        return false;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(handle, &sClockSourceConfig) != HAL_OK) {
        return false;
    }
    if (HAL_TIM_PWM_Init(handle) != HAL_OK) {
        return false;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(handle, &sMasterConfig) != HAL_OK) {
        return false;
    }
    timer_divisors[timer_num] = divisor;
    return true;
}
bool PWM_Init(gpio_pin_t* gpio, uint32_t frequency, bool invert) {
    uint8_t timer_num = gpio->timer_num;
    if (!Timer_Init(timer_num, frequency)) {
        return false;
    }
    uint32_t           channel = timer_channels[gpio->timer_channel];
    TIM_HandleTypeDef* handle  = &timer_handles[timer_num];

    HAL_TIM_PWM_Stop(handle, channel);

    TIM_OC_InitTypeDef sConfigOC = { 0 };

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = invert ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(handle, &sConfigOC, channel) != HAL_OK) {
        return false;
    }

    GPIO_TypeDef* port = gpio->port;
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else {
        return false;
    }

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin              = gpio->pin_num;
    GPIO_InitStruct.Mode             = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    return HAL_TIM_PWM_Start(handle, channel) == HAL_OK;
}
void deinit_pwm(gpio_pin_t* gpio) {
    uint8_t timer_num          = gpio->timer_num;
    timer_divisors[timer_num]  = 0;
    uint32_t           channel = timer_channels[gpio->timer_channel];
    TIM_HandleTypeDef* handle  = &timer_handles[timer_num];
    HAL_TIM_PWM_Stop(handle, channel);
}

void PWM_Duty(gpio_pin_t* gpio, uint32_t duty) {
    uint8_t            timer_num = gpio->timer_num;
    TIM_HandleTypeDef* handle    = &timer_handles[timer_num];
    switch (gpio->timer_channel) {
        case 1:
            handle->Instance->CCR1 = duty;
            break;
        case 2:
            handle->Instance->CCR2 = duty;
            break;
        case 3:
            handle->Instance->CCR3 = duty;
            break;
        case 4:
            handle->Instance->CCR4 = duty;
            break;
    }
}

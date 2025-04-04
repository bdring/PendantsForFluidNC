#include "pin.h"
bool PWM_Init(gpio_pin_t* gpio, uint32_t frequency, bool invert);
void PWM_Duty(gpio_pin_t* gpio, uint32_t duty);

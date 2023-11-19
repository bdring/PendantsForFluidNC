#include "STM32_Pin.h"

STM32_Pin::FailCodes STM32_Pin::set_output(float val) {
    //Serial_Pendant.printf("Set out pin:%d to:", stm_pin_num);
    //Serial_Pendant.println(val);

    if (!initialized) {
        return FailCodes::NotInitialized;
    }

    if (pin_mode == Mode::Output) {
        bool bitValue = (val != 0.0);
        if (activeLow) {
            //Serial_Pendant.printf("active low");
            bitValue = !bitValue;
        }
        //Serial_Pendant.printf("Set out pin:%d to:%d", stm_pin_num, bitValue);
        digitalWrite(stm_pin_num, bitValue);
    } else {
        uint32_t pwm_val = map((uint32_t)(val * 10.0), 0, 1000, 0, 255);  // map 0.0-100.0 to 0 to 255
        analogWrite(stm_pin_num, pwm_val);
    }

    return FailCodes::None;
}

bool STM32_Pin::read_pin(bool forceUpdate) {  // return true if value has changed
    int new_value;
    if (pin_mode == Mode::Input) {
        new_value = digitalRead(stm_pin_num);

        if (activeLow) {
            new_value = !new_value;
        }

        if (forceUpdate | new_value != last_value) {
            if (millis() - last_change_millis > debounce_ms) {
                last_value         = new_value;
                last_change_millis = millis();  // maybe use for debouncing
                return true;
            }
        }
    }
    return false;
}
void STM32_Pin::deinit() {
    if (initialized) {
        initialized        = false;
        activeLow          = false;
        pin_mode           = Mode::None;
        last_value         = -1;   // unknown
        debounce_ms        = 100;  // default
        last_change_millis = 0;
        pinMode(stm_pin_num, INPUT);
    }
}

STM32_Pin::FailCodes STM32_Pin::init(String params) {
    // for now we assume all pins can input and output. Some can do PWM

    activeLow = (params.indexOf("low") != -1);

    if (params.indexOf("out") != -1) {
        pinMode(stm_pin_num, OUTPUT);
        pin_mode    = Mode::Output;
        initialized = true;
        if (activeLow) {
            digitalWrite(stm_pin_num, true);
        }
        return FailCodes::None;
    }
    if (params.indexOf("pwm") != -1) {
        if (!pwm_capable) {
            return FailCodes::NotCapable;
        }
        pinMode(stm_pin_num, OUTPUT);
        pin_mode    = Mode::PWM;
        initialized = true;
        return FailCodes::None;
    }
    if (params.indexOf("inp") != -1) {
        if (params.indexOf("pu")) {
            pinMode(stm_pin_num, INPUT_PULLUP);
        } else if (params.indexOf("pd")) {
            pinMode(stm_pin_num, INPUT_PULLDOWN);
        } else {
            pinMode(stm_pin_num, INPUT);
        }

        //activeLow = (params.indexOf("low") != -1);

        last_value = -1;  // reset to unknown value

        // attachInterrupt(digitalPinToInterrupt(stm_pin_num), pin_interrupt, CHANGE);

        pin_mode    = Mode::Input;
        initialized = true;
        return FailCodes::None;
    }

    return FailCodes::UnknownParameter;
}

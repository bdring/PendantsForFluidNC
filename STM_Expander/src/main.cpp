#include <Arduino.h>
#include "GrblParser.h"
#include <io_controller.h>

// for use on STM32 Only !!!! for now

// https://github.com/stm32duino/Arduino_Core_STM32/wiki/

// STM Pin Reference
// HardwareSerial Serial1(); PA10, PA9
// HardwareSerial Serial2(PA3, PA2);
// HardwareSerial Serial3(PB11, PB10);

HardwareSerial Serial_FNC(PA10, PA9);    // connects from STM32 to ESP32 and FNC
HardwareSerial Serial_Pendant(PA3, PA2); // connects from STM32 to Display

class Displayer : public GrblParser
{

    void debug_message(String message)
    {
        Serial_Pendant.print(message);
    }

    void parse_message(String message)
    {
        String level;
        String body;

        level = message.substring(5); // // trim [MSG:

        int pos = level.indexOf(" ");
        if (pos == -1)
        {
            return;
        }

        level = level.substring(0, pos);

        body = message.substring(6 + level.length());
        body.remove(body.length() - 1);

        // if this is an IO op then get the pin number.
        // [MSG:INI io.1=inp,low,pu]
        if (level == "INI" || level == "GET" || level == "SET")
        {
            int pin_num;
            int STM_pin_num;
            String param_list;
            Serial_Pendant.println(body);
            if (!body.startsWith("io."))
            {
                return;
            }
            pos = body.indexOf(".");
            int nextpos = body.indexOf("=");
            if (pos == -1 or nextpos == -1)
            {
                return;
            }

            pin_num = body.substring(pos + 1, nextpos).toInt();
            param_list = body.substring(nextpos + 1);

            STM_pin_num = get_STM_pin(pin_num);

            Serial_Pendant.println(pin_num);
            Serial_Pendant.println(param_list);

            if (level == "INI")
            {
                if (param_list.indexOf("out")) {
                    // TO DO Initial value
                    pinMode(STM_pin_num, OUTPUT);
                }
                else if (param_list.indexOf("inp")) {

                } else {
                    // fail
                    return;
                }

            }

            if (level == "SET") {
                // [MSG:SET io.1=1]
                // TO DO 
                //    Some basic validation that this is an output pin, pwm, etc
                //    Allow for other params
                //    check that a val exists
                int val = param_list.toInt();
                digitalWrite(STM_pin_num, val);
                return;
            }

                return;
        }

        Serial_Pendant.println(message);
    }

    void show_state(const String &state)
    {
        Serial_Pendant.print(state);
    }

    void show_gcode_modes(const gcode_modes &modes)
    {
        Serial_Pendant.print("Got modes");
    }

    void show_info_message(String message)
    {
        Serial_Pendant.println("Got message");
        Serial_Pendant.println(message);
    }

    void process_set_message(String message)
    {
    }

    //  io.1=inp,low,pu]
    void process_ini_message(String message)
    {
        String pin_id;
        String params;

        auto eq = message.indexOf("=");
        if (eq == -1)
        {
            pin_id = message.substring(0, eq);
            params = message.substring(eq + 1);
        }
        uint32_t pin_num = get_STM_pin(pin_id.substring(2).toInt());

        // setup pin type
        if (params.indexOf("out") != -1)
        {
            pinMode(pin_num, OUTPUT);
        }
        else if (params.indexOf("pwm") != -1)
        {
            pinMode(pin_num, OUTPUT);
        }
        else // assume input
        {
            pinMode(pin_num, INPUT);
        }
    }

} displayer;

void setup()
{

    Serial_FNC.begin(115200);     // PA10, PA9
    Serial_Pendant.begin(115200); // PA3, PA2

    pinMode(PC13, OUTPUT); // for rx/tx activity LED
    Serial_Pendant.println("\r\nHello pendant");
    Serial_FNC.println("Hello FNC");

    pinMode(PA7, OUTPUT);
    digitalWrite(PA7, HIGH);
    delay(100);
    digitalWrite(PA7, LOW);
}

void loop()
{
    while (Serial_FNC.available()) // From Terminal
    {
        char c = Serial_FNC.read();
        Serial.write(c);
        // displayer.write(c);  // for production
        digitalToggle(PC13);
    }

    while (Serial_Pendant.available()) // From FNC
    {
        char c = Serial_Pendant.read();
        Serial_FNC.write(c);
        displayer.write(c); // for testing from terminal
        digitalToggle(PC13);
    }

    delay(5);
    digitalWrite(PC13, HIGH);
}

extern "C" void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1)
            ;
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        while (1)
            ;
    }
}

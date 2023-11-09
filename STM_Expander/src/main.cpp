#include <Arduino.h>
#include "GrblParser.h"
#include <io_controller.h>

// https://github.com/stm32duino/Arduino_Core_STM32/wiki/

// STM UART Reference
// HardwareSerial Serial1(); PA10, PA9
// HardwareSerial Serial2(PA3, PA2);
// HardwareSerial Serial3(PB11, PB10);

HardwareSerial Serial_FNC(PA10, PA9);    // connects from STM32 to ESP32 and FNC
HardwareSerial Serial_Pendant(PA3, PA2); // connects from STM32 to Display

class Displayer : public GrblParser
{

    void debug_message(String message)
    {
    }

    void parse_message(String message)
    {
        String level; // message level
        String body;

        level = message.substring(5); // // trim [MSG:

        int pos = level.indexOf(" ");
        if (pos == -1)
        {
            return;
        }

        level = level.substring(0, pos - 1); // remove the colon

        body = message.substring(6 + level.length() + 1);
        body.remove(body.length() - 1);

        // if this is an IO op then get the pin number.
        if (level == "INI" || level == "GET" || level == "SET")
        {
            int pin_num;
            String pin, param_list;

            pos = body.indexOf(".");
            int nextpos = body.indexOf("=");
            if (pos == -1 or nextpos == -1)
            {
                return;
            }
            pin = body.substring(pos + 1, nextpos);
            pin_num = pin.toInt();
            param_list = body.substring(nextpos + 1);

            if (!body.startsWith("io."))
            {
                return;
            }

            if (level == "GET")
            {
                if (pin == "*")
                {
                    read_all_pins(true);
                }
                return;
            }

            if (level == "INI")
            {
                if (pins[pin_num].init(param_list) != STM32_Pin::FailCodes::None)
                {
                    debug_message("INI Error");
                    protocolRespond(false);
                    return;
                }
            }

            if (level == "SET")
            {
                float val = param_list.toFloat();
                if (pins[pin_num].set_output(val) != STM32_Pin::FailCodes::None)
                {
                    debug_message("Set Error");
                }
                return;
            }

            return;
        }
    }

    void show_state(const String &state)
    {
    }

    void show_gcode_modes(const gcode_modes &modes)
    {
    }

    void show_info_message(String message)
    {
    }

    void process_set_message(String message)
    {
    }

} displayer;

void setup()
{

    io_init();

    Serial_FNC.begin(115200);     // PA10, PA9
    Serial_Pendant.begin(115200); // PA3, PA2

    pinMode(PC13, OUTPUT); // for rx/tx activity LED
    Serial_Pendant.printf("\r\n[MSG:INFO: Hello pendant. Clock %dHz]", F_CPU);
}

void loop()
{
    while (Serial_FNC.available()) // From Terminal
    {
        char c = Serial_FNC.read();
        Serial_Pendant.write(c); // just for debuging
        displayer.write(c);      // for production
    }

    while (Serial_Pendant.available()) // From FNC
    {
        char c = Serial_Pendant.read();
        Serial_FNC.write(c);
        // displayer.write(c); // for testing from pendant terminal
    }

    read_all_pins(false);
}

void debug_message(String message)
{
    Serial_Pendant.printf("[MSG:INFO: Controller debug:%s", message);    
}



// 8 MHz external Crystal with 2x PLL = 16MHz
// Built from the Clock Configurator in STMCubeIDE
extern "C" void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
    
}

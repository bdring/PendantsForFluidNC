#include <Arduino.h>
#include "GrblParser.h"
#include <io_controller.h>

// https://github.com/stm32duino/Arduino_Core_STM32/wiki/

// STM UART Reference
// HardwareSerial Serial1(); PA10, PA9
// HardwareSerial Serial2(PA3, PA2);
// HardwareSerial Serial3(PB11, PB10);

HardwareSerial FNCSerial(PA10, PA9);   // connects from STM32 to ESP32 and FNC
HardwareSerial DebugSerial(PA3, PA2);  // connects from STM32 to Display

extern void send_pin_msg(int pin_num, bool active);

class Displayer : public GrblParser {
    // With no arguments, return an ACK for okay
    void protocolRespond() { putchar(ACK); }

    // If an error message is supplied, send it followed by a NAK
    void protocolRespond(const String& msg) {
        String prefix("$Log/Debug=*");
        send_line(prefix + msg);
        putchar(NAK);
    }

    void handle_msg(const String& command, const String& arguments) {
        if (command == "RST") {
            deinit_all_pins();
            protocolRespond();
            return;
        }
        if (command == "INI" || command == "GET" || command == "SET") {
            // IO operation examples:
            //   INI: io.N=out,low
            //   INI: io.N=inp,pu
            //   INI: io.N=pwm
            //   GET: io.*
            //   GET: io.N
            //   SET: io.N=0.5
            int    pin_num;
            String pinspecs, pin, param_list;

            pinspecs = arguments;  // Mutable copy for trim
            pinspecs.trim();

            if (!pinspecs.startsWith("io.")) {
                protocolRespond("Missing pin specifier");
                return;
            }

            int pinnumpos = strlen("io.");
            int equalspos = pinspecs.indexOf("=");
            if (equalspos != -1) {
                pin        = pinspecs.substring(pinnumpos, equalspos);
                param_list = pinspecs.substring(equalspos + 1);
            } else {
                pin = pinspecs.substring(pinnumpos + 1);
            }

            if (command == "GET") {
                if (pin == "*") {
                    protocolRespond();
                    read_all_pins(send_pin_msg, true);
                    return;
                }
                pin_num = pin.toInt();
                if (valid_pin_number(pin_num)) {
                    protocolRespond();
                    read_pin(send_pin_msg, pin_num, true);
                } else {
                    protocolRespond("Invalid pin number");
                }
                return;
            }

            if (command == "INI") {
                pin_num = pin.toInt();
                if (pins[pin_num].init(param_list) != STM32_Pin::FailCodes::None) {
                    protocolRespond("INI Error");
                } else {
                    protocolRespond();
                }
                return;
            }

            if (command == "SET") {
                pin_num = pin.toInt();
                if (param_list == "") {
                    protocolRespond("Missing value for SET");
                } else {
                    float val = param_list.toFloat();
                    if (pins[pin_num].set_output(val) != STM32_Pin::FailCodes::None) {
                        protocolRespond("Set Error");
                    }
                }
                return;
            }
        }
    }
    int getchar() {
        if (FNCSerial.available()) {
            return FNCSerial.read();
        }
        return -1;
    }
    int  milliseconds() { return millis(); }
    void poll_extra() {
        while (DebugSerial.available()) {  // From debug UART
            char c = DebugSerial.read();
            putchar(c);
            collect(c);  // for testing from pendant terminal
        }

        read_all_pins(send_pin_msg, false);
    }

public:
    void putchar(uint8_t c) { FNCSerial.write(c); }
} displayer;

void setup() {
    io_init();

    FNCSerial.begin(115200);  // PA10, PA9

    DebugSerial.begin(115200);  // PA3, PA2
    DebugSerial.printf("\r\n[MSG:INFO: Hello pendant. Clock %dHz]", F_CPU);

    pinMode(PC13, OUTPUT);  // for rx/tx activity LED

    displayer.wait_ready();
    // XXX we need some sort of message to tell FluidNC that the
    // expander has been reset.  At startup, that would be okay, but
    // if it happens later, it is probably an alarm condition because
    // the pins are now invalid.  Maybe the message should be a realtime
    // character to avoid the need to ack with an ok, since we cannot
    // depend on FluidNC to be ready when the expander starts.
}

void loop() {
    displayer.poll();
}

void send_pin_msg(int pin_num, bool active) {
    displayer.putchar(active ? PinHigh : PinLow);
    displayer.putchar(pin_num);
}

// 8 MHz external Crystal with 2x PLL = 16MHz
// Built from the Clock Configurator in STMCubeIDE
extern "C" void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

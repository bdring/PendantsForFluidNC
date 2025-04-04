# Display to Terminal Pendant

This pendant requires a microcontroller with one UART and a
single LED identified by the Arduino LED_BUILTIN macro.  The
UART connects to a FluidNC controller and the LED lights when
the FluidNC controller is in Cycle state (moving).

This example pendant could be extended to show additional
states by adding more LEDs or by using a multicolor LED.

## Wiring

Connect a secondary UART on the FluidNC controller to the primary UART
on the MCU.  Connect MCU Tx to FluidNC Rx and vice versa.  If the MCU
has 5V IO, you will need level conversion on the MCU Tx to FluidNC Rx
line.  It could be just a resistive voltage divider like this:

  MCU Tx  ---XXXX------ FluidNC Rx
             2.2K  |
         resistor  X
                   X 3.3K resistor
                   X
                   |
  MCU GND --------------FluidNC GND


If the MCU has 3.3V IO, you can connect directly with no level shifting.

The FluidNC Tx to MCU Rx line typically can be directly connected,
since a 3.3V output can drive a 5V input safely and effectively.

## Example FluidNC Config Section

```yaml
uart1:
  txd_pin: gpio.17
  rxd_pin: gpio.16
  baud: 115200
  mode: 8N1
uart_channel1:
  uart_num: 1
  report_interval_ms: 400
```

## Assigning UART Pins

On the FluidNC controller you can assign the UART pins in the config file; just change gpio.17 and gpio.16 in the example above.

On the MCU, the Arduino framework assigns IO pins to the primary UART according to a board definition file.  platformio.ini has "env:" sections for various MCUs.  Each such env: section has a "board =" line that selects a particular board that has that MCU.  Set that line to the name of a board that is compatible with the one you are using.  Ask the internet for help if you don't understand that.

On some MCUs, the primary UART is also connected to a USB-Serial chip so it can be connected to a PC.  It is often possible to connect that primary UART directly to another computer, if the USB-serial lines are isolated with resistors to permit the other computer to overdrive the MCU's Rx line.

## Compiling

* Install PlatformIO according to instructions on the Web.  You can use any IDE/editing environment that you like.  Many people use PlatformIO under VSCode, but it also works with many other editors.
* Edit platformio.ini and set "default_envs" to the name of the MCU you want to use.  The name should match one of the "env:" values in platformio.ini
* Click the IDE's "Upload" button, or execute "pio run -t upload" if you are running PlatformIO from the command line.
* Connect a serial monitor program to the primary UART of the MCU.  It should display a message whenever the FluidNC state changes (Idle, Cycle, Hold, etc).

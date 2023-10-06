# PendantsForFluidNC
Pendants for controlling FluidNC CNC firmware

This is a collection of example "pendant" programs showing how to
program a auxiliary microcontroller or other computer to interact
with a FluidNC CNC controller. FluidNC uses the [GRBL serial
line protocol](https://github.com/gnea/grbl/wiki/Grbl-v1.1-Interface)
for control and status.  Unlike classic GRBL controllers which
speak that serial protocol on only one serial port,
FluidNC can have multiple "channels" simultaneously speaking
the serial protocol.  The additional channels can include
extra serial ports, TCP/Telnet connections, and TCP/WebSocket
connections.

Another serial port (UART) is an excellent way to connect an auxiliary
microcontroller (MCU).  Every MCU has at least one UART, and most have
two or more.  A UART needs only two signal lines and in some use
cases only one.  Most MCU UARTs can operate at least as fast as
115200 baud (10K characters per second) and some can go up to
1Mbaud (100K characters per second) or more.  That is typically
fast enough for a lot of interesting use cases.

## What is a Pendant?

In addition to the main operator's console, traditional CNC machines
often included an auxiliary control box that hung from a cable near
the machine.  Those boxes were called "pendants" because they were
hanging or suspended from overhead. Typically pendants could perform a
subset of the most common operations like jogging and job
start/pause/stop.  Thus the word "pendant" has come to mean any small
auxilary control for a CNC machine.

A FluidNC Pendant that is implemented on a small MCU can be very
inexpensive, since suitable MCUs can cost in the range of $1 to a few
dollars.  The packaging and user-facing I/O devices (small displays,
pushbuttons, etc) are likely to dominate the cost.

## What Can FluidNC Pendants Do?

In principle, a FluidNC Pendant could do anything that a traditional
GCode Sender program can do, limited only by the resources of the
MCU on which it is implemented.  Typically, a given pendant will
only do a targeted subset of the possibilities, because otherwise
it would be just another sender requiring a full computer.  Some
examples of small-pendant use cases include:

* Running "playlists" for art machines
* Jog controllers
* Input expansion
* Extra displays like light towers or OLEDs, to show machine state

## How Do I Use This Code?

This code is a collection of examples to help you develop your own
custom pendants.  Many of the examples can be compiled for several
different MCUs, including various ESP32 variants, AVR Arduinos and
some ARM MCUs.  We expect that you will start with an example that
is similar to your desired gadget and modify/extend it for your needs.
We do not provide free consultation for doing that.

One of the basic components that is useful for display pendants is
the GrblParser class.  GrblParser decodes standard GRBL protocol
report messages and calls user-supplied code to display the information
therein.

## Support

This code is not supported.  It is provided as a courtesy to help
you get started.  If you have ideas for new features, please implement
them yourself or contract with someone to do so.

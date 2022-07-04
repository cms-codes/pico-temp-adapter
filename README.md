# pico-temp-adapter

### IoT project using Raspberry Pi Pico and ESP.

Uses PlatformIO IDE, with the wizio-pico platform library for the Raspberry Pi Pico.
SPI peripheral mode to allow for ESP controller to set target value and read current values.

TO-DO: 
 - create SPI class with non-blocking transfers
 - move temperature reading to main loop to minimize up isr running time

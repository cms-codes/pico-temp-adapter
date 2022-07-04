#ifndef SPI_H
#define SPI_H
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#define _SPI_RX_PIN  20
#define _SPI_SCK_PIN 18
#define _SPI_TX_PIN  19
#define _SPI_NSS_PIN 17

void _spi_init(void);

void spi_float_to_bytes(const volatile float *float_in, uint8_t *bytes_out, const int len);

#endif
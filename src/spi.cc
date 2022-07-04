#include "spi.h"

void _spi_init(void) {
  spi_init(spi0, 2000 * 1000);
  spi_set_slave(spi0, true);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
  gpio_set_function(_SPI_RX_PIN, GPIO_FUNC_SPI);
  gpio_set_function(_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(_SPI_TX_PIN, GPIO_FUNC_SPI);

}

void spi_float_to_bytes(const volatile float *float_in, uint8_t *bytes_out, const int len) {
  uint8_t *byte = (uint8_t *)*(uint32_t *)(&float_in);

  for (int i = 0; i < len; i++, byte++) {
    bytes_out[i] = *byte;
  }

  return;
}
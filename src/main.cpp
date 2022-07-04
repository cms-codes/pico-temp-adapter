#include <stdio.h>
#include "irq.h"
#include "i2c.h"
#include "spi.h"
#include "BMP180_Pico.h"

#define _TEMP_SAMPLING_DELTA_MS 1000

bool temp_reading_callback(struct repeating_timer *t);

// create the driver object
BMP180_Pico sensor1 = BMP180_Pico();
volatile float temp_reading = 0.0f;

uint8_t spi_in[5] = {0};
uint8_t spi_out[5] = {0};

struct repeating_timer temp_reading_timer;

int main(void) {
  stdio_init_all();
  sleep_ms(2000);
  _irq_init(); // set up CS pin to trigger SPI
  _i2c_init(); // temperature sensor bus
  _spi_init(); // SPI peripheral mode
  if (!sensor1.Begin()) {
    while (true)
      printf("Did not find BMP180 sensor!\n");
  }
  printf("--- init complete\n");

  // negative delay value guarantees deadline regardless of preceding run time
  add_repeating_timer_ms(_TEMP_SAMPLING_DELTA_MS * -1, temp_reading_callback, NULL, &temp_reading_timer);
  
  while (true) {
    if (SPI_flag) {
      // serialize temperature reading
      spi_float_to_bytes(&temp_reading, spi_out, sizeof(temp_reading));
      if (spi_is_readable(spi0)) {
        spi_read_blocking(spi0, 0, spi_in, 1);
        if (spi_in[0] == 0xAC) {
          // command for sending temperature reading
          spi_write_read_blocking(spi0, spi_out, spi_in, sizeof(temp_reading));    
        }
      }
      // transaction complete, reset flag
      SPI_flag = 0;
    }
    tight_loop_contents();
  }

  return 0;
}

bool temp_reading_callback(struct repeating_timer *t) {
  // update blocked by SPI transaction
  // TO-DO: remove sensor method call from this isr
  if (SPI_flag == false) temp_reading = sensor1.GetTemperature();
  return true;
}
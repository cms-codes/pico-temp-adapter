#include <stdio.h>
#include "i2c.h"
#include "BMP180_Pico.h"

// create the driver object
BMP180_Pico sensor1 = BMP180_Pico();

int main(void) {
  stdio_init_all();
  sleep_ms(2000);
  _i2c_init();
  if (!sensor1.Begin()) {
    while (true)
      printf("Did not find BMP180 sensor!\n");
  }
  
  while (true) {
    printf("%4.3f C\n", sensor1.GetTemperature());
    sleep_ms(2000);
  }

  return 0;
}

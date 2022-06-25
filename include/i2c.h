#ifndef I2C_H
#define I2C_H
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define _I2C_SDA_PIN 4
#define _I2C_SCL_PIN 5

void _i2c_init(void);

#endif
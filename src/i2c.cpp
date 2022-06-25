#include "i2c.h"

void _i2c_init(void) {
    i2c_init(&i2c0_inst, 100 * 1000);
    gpio_set_function(_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(_I2C_SCL_PIN, GPIO_FUNC_I2C);
}
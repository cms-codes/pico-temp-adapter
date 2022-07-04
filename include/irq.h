#ifndef IRQ_H
#define IRQ_H

#include "hardware/gpio.h"
#include "hardware/irq.h"

extern volatile bool SPI_flag;

void _irq_init(void);
void gpio_irq_handler(void);

#endif
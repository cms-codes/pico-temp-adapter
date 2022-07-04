#include "irq.h"
#include "spi.h"

volatile bool SPI_flag = false;

void _irq_init(void) {
  gpio_set_irq_enabled(_SPI_NSS_PIN, GPIO_IRQ_EDGE_FALL, true);
  irq_set_exclusive_handler(IO_IRQ_BANK0, gpio_irq_handler);
  irq_set_enabled(IO_IRQ_BANK0, true);
}

void gpio_irq_handler(void) {
  // TO-DO: mask IRQ to know which pin is being signaled which is
  //        needed to accommodate multiple GPIO by this handler
  gpio_acknowledge_irq(_SPI_NSS_PIN, GPIO_IRQ_EDGE_FALL);
  SPI_flag = true;
}
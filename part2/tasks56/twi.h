#ifndef TWI_H
#define TWI_H

#include <avr/io.h>
#include <avr/interrupt.h>

extern void (*twi_callback)(uint8_t res);

void twi_init();
void twi_write(uint8_t dev, uint8_t len, uint8_t *ptr, void (*cb)(uint8_t));
void twi_read(uint8_t dev, uint8_t len, uint8_t *ptr, void (*cb)(uint8_t));

#endif

#ifndef MEM_H
#define MEM_H

#include <avr/io.h>

#define MEM_CODE 0b1010000

void mem_init();
void mem_read(uint8_t dev, uint16_t addr, uint8_t len, uint8_t *data, void (*cb)(uint8_t));
void mem_write(uint8_t dev, uint16_t addr, uint8_t len, uint8_t *data, void (*cb)(uint8_t));

#endif

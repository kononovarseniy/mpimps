// BIO - Basic Input Output
#ifndef BIO_H
#define BIO_H

#include <avr/io.h>

#define COL_SHIFT 1
#define COL_MASK (0b111 << COL_SHIFT)
#define ROW_SHIFT 4
#define ROW_MASK (0b1111 << ROW_SHIFT)
#define BTN(C, R) (1 << ((2 - C) * 4 + R))

void kbd_init();
uint16_t kbd_scan();

void adc_init();

#endif

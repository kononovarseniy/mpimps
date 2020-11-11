#ifndef LED_H
#define LED_H

#include <avr/io.h>

#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define MOSI_PIN PB5
#define MISO_PIN PB6
#define SCK_PIN PB7
#define SS_PIN PB4

extern uint8_t display_tick;

void led_init();
void led_show(uint32_t *display);

void led_clear(uint32_t *display);
void led_set(uint32_t *display, uint8_t x, uint8_t y, uint8_t color);
void led_fill_rect(uint32_t *display, uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color);
void led_draw_rect(uint32_t *display, uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color);

#endif

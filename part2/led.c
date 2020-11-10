#include "led.h"

uint8_t display_tick = 0;

void spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
}

void send_all(uint8_t reg, uint8_t val) {
    SPI_PORT &= ~(1 << SS_PIN);
    for (int i = 0; i < 4; i++){
        spi_transfer(reg);
        spi_transfer(val);
    }
    SPI_PORT |= (1 << SS_PIN);
}

void led_init() {
    SPI_PORT |= (1 << SS_PIN);
    SPI_DDR |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << SS_PIN);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (0 << SPR1); // clk/16, master, MSB first

    send_all(0xF, 0); // test off
    send_all(0x9, 0); // no decode
    send_all(0xA, 0); // lowiest intensity
    send_all(0xB, 7); // all lines
    send_all(0xC, 1); // turn on
}

void led_show(uint32_t *disp) {
    for (int i = 0; i < 8; i++) {
        uint8_t *ptr = (uint8_t *)(disp++)+3;
        SPI_PORT &= ~(1 << SS_PIN);
        for (int j = 0; j < 4; j++) {
            spi_transfer(i + 1);
            spi_transfer(*ptr--);
        }
        SPI_PORT |= (1 << SS_PIN);
    }
}

void led_clear(uint32_t *display) {
    uint8_t *ptr = (uint8_t *)display;
    for (uint8_t i = 0; i < 32; i++)
        *ptr++ = 0;
}

void led_set(uint32_t *display, uint8_t x, uint8_t y, uint8_t color) {
    uint8_t *line = (uint8_t *)display + 4 * y;
    uint8_t *ptr = line + 3 - x / 8;
    uint8_t mask = 0x80 >> (x % 8);
    if (color == 2)
        color = (x ^ y ^ display_tick) & 1;
    if (color)
        *ptr |= mask;
    else
        *ptr &= ~mask;
}

void led_fill_rect(uint32_t *display, uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color) {
    for (uint8_t x = x0; x < x0 + w; x++)
        for (uint8_t y = y0; y < y0 + h; y++)
            led_set(display, x, y, color);
}

void led_draw_rect(uint32_t *display, uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color) {
    for (uint8_t x = x0; x < x0 + w; x++) {
        led_set(display, x, y0, color);
        led_set(display, x, y0 + h - 1, color);
    }
    for (uint8_t y = y0 + 1; y < y0 + h; y++) {
        led_set(display, x0, y, color);
        led_set(display, x0 + w - 1, y, color);
    }
}


#include "lcd.h"
#include <avr/io.h>

#define DATA_PORT PORTA
#define DATA_DDR DDRA
#define DATA_IN PINA

#define CTL_PORT PORTB
#define CTL_DDR DDRB
#define RS_PIN PB0
#define RW_PIN PB1
#define E_PIN PB2

void hd_wait() {
    do {
        DATA_DDR = 0x00;
        DATA_PORT = 0xff;
        CTL_PORT &= ~(1 << RS_PIN);
        CTL_PORT |= (1 << RW_PIN);
        CTL_PORT |= (1 << E_PIN);
        //hd_delay();
        CTL_PORT &= ~(1 << E_PIN);
    } while (DATA_IN & 0x80);
}

void hd_write(uint8_t data, uint8_t is_data) {
    hd_wait();
    if (is_data)
        CTL_PORT |= (1 << RS_PIN);
    else
        CTL_PORT &= ~(1 << RS_PIN);
    CTL_PORT &= ~(1 << RW_PIN);
    CTL_PORT |= (1 << E_PIN);
    DATA_DDR = 0xff;
    DATA_PORT = data;
    //hd_delay();
    CTL_PORT &= ~(1 << E_PIN);
    DATA_DDR = 0x00;
}

uint8_t hd_read(uint8_t is_data) {
    hd_wait();
    DATA_DDR = 0x00;
    DATA_PORT = 0xff;
    if (is_data)
        CTL_PORT |= (1 << RS_PIN);
    else
        CTL_PORT &= ~(1 << RS_PIN);
    CTL_PORT |= (1 << RW_PIN);
    CTL_PORT |= (1 << E_PIN);
    //hd_delay();
    uint8_t res = DATA_IN;
    CTL_PORT &= ~(1 << E_PIN);
    return res;
}

void hd_cmd(uint8_t cmd) {
    hd_write(cmd, 0);
}

void hd_data(uint8_t data) {
    hd_write(data, 1);
}

void lcd_goto(uint8_t y, uint8_t x) {
    if (y == 1)
        lcd_ddram(0x40 + x);
    else
        lcd_ddram(x);
}

void lcd_put(char ch) {
    hd_data(ch);
}

void lcd_print(char *str) {
    while (*str) hd_data(*str++);
}

void lcd_init() {
    CTL_DDR |= (1 << RS_PIN) | (1 << RW_PIN) | (1 << E_PIN);
    lcd_function(1, 1, 0); // 8-bit data, 2 lines, default font
    lcd_clear();
    lcd_entry_mode(1, 0); // inc addr on write, fixed display position

    lcd_on_off_ctl(1, 1, 1); // display: on, cursor: on, block_cursor: off
}


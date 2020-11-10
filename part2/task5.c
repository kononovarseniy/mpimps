#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "led.h"

uint8_t scaler = 0;
uint8_t counter = 0;

uint32_t display[8];
uint8_t table[32];


ISR(TIMER0_OVF_vect) {
    led_clear(display);
    led_fill_rect(display, 2, 2, 4, 4, 2);
    led_draw_rect(display, 8, 0, 8, 8, 1);
    led_draw_rect(display, 10, 2, 4, 4, 2);
    led_fill_rect(display, 19, 3, 10, 2, 1);
    for (int i = 0; i < 32; i++) {
        led_set(display, i, table[(i + display_tick/2) % 32], 1);
    }
    led_show(display);
    display_tick++;

    if (++scaler < 30) return;
    scaler = 0;

    if (++counter == 10) counter = 0;

    lcd_goto(0, 15);
    lcd_put('0' + counter);
    lcd_goto(1, 12);
    lcd_shift(0, 0);
}

void main() {
    TCCR0 = (1 << CS02) | (0 << CS01) | (1 << CS00);
    TIMSK = (1 << TOIE0);

    led_init();
    led_clear(display);
    for (int i = 0; i < 32; i++)
        table[i] = i % 8;

    lcd_init();

    sei();

    lcd_goto(0, 5);
    lcd_print("Hello");
    lcd_goto(1, 4);
    lcd_print("world!!!");

    lcd_shift(0, 0);

    for (;;);
}

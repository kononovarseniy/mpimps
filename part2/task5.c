#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "led.h"
#include "bio.h"

uint8_t scaler = 0;
uint8_t counter = 0;

uint32_t display[8];
uint8_t table[32];
uint8_t pos = 0;
uint8_t dot_x = 30;
uint8_t dot_y = 1;

#define BTN_L BTN(0, 3)
#define BTN_D BTN(1, 3)
#define BTN_R BTN(2, 3)
#define BTN_U BTN(1, 2)

ISR(TIMER0_OVF_vect) {
    uint16_t kbs = kbd_scan();

    if ((kbs & BTN_L) && dot_x > 0) dot_x--;
    if ((kbs & BTN_R) && dot_x < 31) dot_x++;
    if ((kbs & BTN_U) && dot_y > 0) dot_y--;
    if ((kbs & BTN_D) && dot_y < 7) dot_y++;

    led_clear(display);
    led_draw_rect(display, 8, 0, 8, 8, 1);
    led_fill_rect(display, 10, 2, 4, 4, 2);
    led_fill_rect(display, pos, 0, 1, 8, 1);
    led_set(display, dot_x, dot_y, 1);
    for (int i = 0; i < 8; i++) {
        led_set(display, i, table[(i + display_tick/2) % 32], 1);
    }

    for (uint8_t r = 0; r < 4; r++) {
        for (uint8_t c = 0; c < 3; c++) {
            led_set(display, c + 29, r + 4, !(kbs & BTN(c, r)));
        }
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

ISR(ADC_vect) {
    pos = ADCH / 8;
}

void main() {
    led_clear(display);
    for (int i = 0; i < 32; i++)
        table[i] = i % 8;

    TCCR0 = (1 << CS02) | (0 << CS01) | (1 << CS00);
    TIMSK = (1 << TOIE0);

    lcd_init();
    led_init();
    adc_init();
    kbd_init();

    sei();

    lcd_goto(0, 5);
    lcd_print("Hello");
    lcd_goto(1, 4);
    lcd_print("world!!!");

    lcd_shift(0, 0);

    for (;;);
}

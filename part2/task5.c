#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "led.h"
#include "bio.h"
#include "twi.h"
#include "mem.h"

uint8_t scaler = 0;
uint8_t counter = 0;

uint32_t display[8];
uint8_t table[32];
uint8_t line_pos = 0;
uint8_t dot_pos[] = { 30, 1 };

#define BTN_L BTN(0, 3)
#define BTN_D BTN(1, 3)
#define BTN_R BTN(2, 3)
#define BTN_U BTN(1, 2)

#define BTN_SAVE BTN(0, 2)
#define BTN_LOAD BTN(2, 2)

uint16_t kbs;

#define MEMORY_DEVICE 0b111
#define MEMORY_POS_ADDRESS 0x0000
uint8_t memory_busy = 0;

void save_callback(uint8_t res) {
    memory_busy = 0;
}

void load_callback(uint8_t res) {
    memory_busy = 0;
}

ISR(TIMER0_OVF_vect) {
    uint8_t prev_kbs = kbs;
    kbs = kbd_scan();

    if ((kbs & BTN_L) && dot_pos[0] > 0) dot_pos[0]--;
    if ((kbs & BTN_R) && dot_pos[0] < 31) dot_pos[0]++;
    if ((kbs & BTN_U) && dot_pos[1] > 0) dot_pos[1]--;
    if ((kbs & BTN_D) && dot_pos[1] < 7) dot_pos[1]++;

    if ((kbs & BTN_SAVE) && ~(prev_kbs & BTN_SAVE)) {
        if (!memory_busy) {
            memory_busy = 1;
            mem_write(MEMORY_DEVICE, MEMORY_POS_ADDRESS, 2, dot_pos, save_callback);
        }
    }
    if ((kbs & BTN_LOAD) && ~(prev_kbs & BTN_LOAD)) {
        if (!memory_busy) {
            memory_busy = 1;
            mem_read(MEMORY_DEVICE, MEMORY_POS_ADDRESS, 2, dot_pos, load_callback);
        }
    }

    led_clear(display);
    led_draw_rect(display, 8, 0, 8, 8, 1);
    led_fill_rect(display, 10, 2, 4, 4, 2);
    led_fill_rect(display, line_pos, 0, 1, 8, 1);
    led_set(display, dot_pos[0], dot_pos[1], 1);
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
    line_pos = ADCH / 8;
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
    twi_init();
    mem_init();

    sei();

    lcd_goto(0, 5);
    lcd_print("Hello");
    lcd_goto(1, 4);
    lcd_print("world!!!");

    lcd_shift(0, 0);

    for (;;);
}

#include <avr/io.h>
#include <avr/interrupt.h>

#define DATA_PORT PORTA
#define DATA_DDR DDRA
#define DATA_IN PINA

#define CTL_PORT PORTB
#define CTL_DDR DDRB
#define RS_PIN PB0
#define RW_PIN PB1
#define E_PIN PB2

#include "lcd.h"

uint8_t scaler = 0;
uint8_t counter = 0;

ISR(TIMER0_OVF_vect) {
    if (++scaler < 30) return;
    scaler = 0;

    if (++counter == 10) counter = 0;
    lcd_goto(0, 15);
    lcd_put('0' + counter);
    lcd_goto(1, 12);
    lcd_shift(0, 0);
    DDRB |= (1 << PB3);
    PORTB |= (1 << PB3);
}
void main() {
    TCCR0 = (1 << CS02) | (0 << CS01) | (1 << CS00);
    TIMSK = (1 << TOIE0);

    sei();

    lcd_init();
    
    lcd_goto(0, 5);
    lcd_print("Hello");
    lcd_goto(1, 4);
    lcd_print("world!!!");

    lcd_shift(0, 0);

    for (;;);
}

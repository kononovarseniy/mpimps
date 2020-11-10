#include "bio.h"

void kbd_init() {
    DDRA &= ~(COL_MASK | ROW_MASK);
    PORTA |= (COL_MASK | ROW_MASK); 
}

uint16_t kbd_scan() {
    uint16_t res = 0;
    for (uint8_t col = 0; col < 3; col++) {
        PORTA &= ~(1 << (col + COL_SHIFT)); 
        DDRA |= 1 << (col + COL_SHIFT);

        res <<= 4;
        res |= (~PINA & ROW_MASK) >> ROW_SHIFT;

        DDRA &= ~(COL_MASK);
        PORTA |= COL_MASK;
    }
    return res;
}

void adc_init() {
    // Setup ADC with internal AREF, left adgusted ADC value, on pin PORTA[0]
    ADMUX = (0 << REFS1) | (1 << REFS0) | (1 << ADLAR) + (0b00000 << MUX0);
    // Enable ADC, start conversion, auto trigger, prescaler 128
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}


#include <avr/io.h>

interrupts:
.org 0x00
    rjmp main
.org 0x1C*2
    rjmp adc_int
.org 0x20*2

main:
    ; init stack
    ldi r16, lo8(RAMEND)
    sts SPL, r16
    ldi r16, hi8(RAMEND)
    sts SPH, r16

    ; init ports
    ldi r16, (1 << PB3)
    sts DDRB, r16

    ; init timer
    ldi r16, 0b01100001 ; Non-inverting fast PWM mode without prescaler
    ldi r17, (1 << TOIE0)
    sts TCCR0, r16
    sts TIMSK, r17

    ; ADC setup
    ldi r16, (0b011 << 5)+(0) ; Setup ADC with internal AREF, left adgusted ADC value on pin PORTA[0]
    sts ADMUX, r16

    ldi r16, (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
    ; 0b11101111 Enable ADC, start conversion, auto trigger, prescaler 128
    sts ADCSRA, r16

    lds r16, SFIOR ; free running mode
    andi r16, ~0b11100000
    sts SFIOR, r16

    sei

loop:
    rjmp loop

adc_int:
    lds r16, ADCH
    sts OCR0, r16
    reti



#include <avr/io.h>

interrupts:
.org 0x0000
    rjmp main
.org 0x0054
    rjmp adc_int
.org 0x0068

main:
    ; use arduino pin 6 for PWM
    ; it is PORTD[6]
    ldi r16, (1<<6)
    sts DDRD, r16
    clr r16
    sts PORTD, r16

    ldi r16, 0b10000011 ; Non-inverting fast PWM mode
    ldi r17, 0b00000001 ; no prescaling
    sts TCCR0A, r16
    sts TCCR0B, r17

; ADC setup
    ldi r16, (0b0110 << 4)+(0) ; Setup ADC with internal AREF, left adgusted ADC value on pin PORTC[0]
    sts ADMUX, r16

    ldi r16, 0b00000001 ; Disable input buffer on analog pin
    sts DIDR0, r16

    ldi r16, 0b11101111 ; Enable ADC, start conversion, auto trigger, prescaler 128
    sts ADCSRA, r16

    ldi r16, 0b00000000 ; free running mode
    sts ADCSRB, r16

    sei

loop:
    rjmp loop

adc_int:
    lds r16, ADCH
    sts OCR0A, r16
    reti



#include <avr/io.h>

interrupts:
.org 0x0000
    rjmp main
.org 0x0040         ; TIMER0 OVF interrupt (WHY it is 0x40? Not 0x20)
    rjmp timer_overflow
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

    ldi r16, 0b00000001 ; Enable TC0_OVF interrupt
    sts TIMSK0, r16

    ldi r17, 0
    sts OCR0A, r17 ; analogWrite
    ldi r18, 0 ; counter
    ldi r17, 1 ; direction

    sei ; Enable interrupts

loop:
    sleep
    rjmp loop

timer_overflow:
    inc r18
    cpi r18, 100 ; rize time is 0.4s
    brne timer_overflow_end
        lds r16, OCR0A
        add r16, r17
        sts OCR0A, r16

        cpi r16, 255
        brne .+2
            ldi r17, -1

        cpi r16, 0
        brne .+2
            ldi r17, 1
timer_overflow_end:
    reti



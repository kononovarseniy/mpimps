#include <avr/io.h>

interrupts:
.org 0x00
    rjmp main
.org 0x12*2
    rjmp timer_overflow
.org 0x30*2

#define CNT r18
#define DIR r17

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

    ; init variables
    clr CNT
    ldi DIR, 1

    sei ; Enable interrupts

loop:
    rjmp loop

timer_overflow:
    inc CNT
    cpi CNT, 30 ; rize time is 0.5s
    brne timer_overflow_end
        clr CNT
        lds r16, OCR0
        add r16, DIR
        sts OCR0, r16

        cpi r16, 255
        brne l1
            ldi DIR, -1
        l1:

        cpi r16, 0
        brne l2
            ldi DIR, 1
        l2:
    timer_overflow_end:
    reti



#include <avr/io.h>

interrupts:
.org 0x00
    rjmp main
.org 0x12*2
    rjmp timer_overflow
.org 0x30*2

main:
    ; init stack
    ldi r16, lo8(RAMEND)
    sts SPL, r16
    ldi r16, hi8(RAMEND)
    sts SPH, r16

    ldi r16, (1 << PB3)
    sts DDRB, r16

    ldi r16, 0b00000101 ; Setup timer source clk/1024
    sts TCCR0, r16

    ldi r16, (1 << TOIE0)
    sts TIMSK, r16

    sei ; Enable interrupts
    
    clr r16
    sts TCNT0, r16

blink:
    sbi PORTB-0x20, PB3
    rcall delay
    cbi PORTB-0x20, PB3
    rcall delay
    rjmp blink

timer_overflow: ; on 8-bit TC0 overflow (clk/265/1024 = clk/2^18)
                ; i.e. timeout is 32.768 ms
    inc r17     ; cnt++;
    cpi r17, 16 ; if (cnt == 16)
    brne .+2    ;
    clr r17     ;     cnt = 0
    reti

delay:
    clr r17
    wait:
        cpi r17, 15 ; 32.768 * 15 = 491.52 ms
        brne wait
    ret


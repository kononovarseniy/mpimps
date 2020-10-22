#include <avr/io.h>

interrupts:
.org 0x0000
    rjmp main
.org 0x0040         ; TIMER0 OVF interrupt (WHY it is 0x40? Not 0x20)
    rjmp timer_overflow
.org 0x0068

main:
    ; init stack
    ldi r16, lo8(RAMEND)
    sts SPL, r16
    ldi r16, hi8(RAMEND)
    sts SPH, r16

    ldi r16, 0x20 ; arduino_pin13
    sts DDRB, r16

    clr r16
    sts TCCR0A, r16

    ldi r16, 0b00000101 ; Setup timer source clk/1024
    sts TCCR0B, r16

    ldi r16, 0b00000001 ; Enable TC0_OVF interrupt
    sts TIMSK0, r16

    sei ; Enable interrupts
    
    clr r16
    sts TCNT0, r16

blink:
    sbi 0x5, 5
    rcall delay
    cbi 0x5, 5
    rcall delay
    rjmp blink

timer_overflow: ; on 8-bit TC0 overflow (clk/265/1024 = clk/2^18)
                ; i.e. timeout is 16.384 ms
    inc r17     ; cnt++;
    cpi r17, 31 ; if (cnt == 31)
    brne .+2    ;
    clr r17     ;     cnt = 0
    reti

delay:
    clr r17
    wait:
        cpi r17, 30 ; 16.384 * 30 = 491.52 ms
        brne wait
    ret


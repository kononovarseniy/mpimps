#include <avr/io.h>
#define MILLIS_INC 1
#define MICROS_INC 24

#define INTERVAL 12

interrupts:
.org 0x00
    rjmp main
.org 0x12*2
    rjmp timer0_ovf
.org 0x30*2

#define MILLIS (RAMSTART + 0x0)
#define MICROS (RAMSTART + 0x2)

timer0_ovf:
    push r16
    push r17
    push r18
    push r19
    push r0
    lds r0, SREG

    lds r16, MILLIS
    lds r17, MILLIS+1
    lds r18, MICROS
    lds r19, MICROS+1

    subi r18, lo8(-MICROS_INC)
    sbci r19, hi8(-MICROS_INC)
    mov r1, r18
    mov r2, r19
    subi r18, lo8(1000)
    sbci r19, hi8(1000)
    brlt else
        subi r16, lo8(-MILLIS_INC-1)
        sbci r17, hi8(-MILLIS_INC-1)
        rjmp end_if
    else:
        mov r18, r1
        mov r19, r2
        subi r16, lo8(-MILLIS_INC)
        sbci r17, hi8(-MILLIS_INC)
    end_if:
   
    sts MILLIS,   r16
    sts MILLIS+1, r17
    sts MICROS,   r18
    sts MICROS+1, r19

    sts SREG, r0
    pop r0
    pop r19
    pop r18
    pop r17
    pop r16
    reti

main:
    ; init stack
    ldi r16, lo8(RAMEND)
    sts SPL, r16
    ldi r16, hi8(RAMEND)
    sts SPH, r16

    ; init timer
    clr r16
    sts MILLIS, r16
    sts MILLIS+1, r16
    sts MICROS, r16
    sts MICROS+1, r16
    
    ldi r16, 0b00000011 ; 
    ldi r17, (1 << TOIE0) ; enable interrupt
    sts TCCR0, r16
    sts TIMSK, r17
 
    ; init IO pins
    ldi r16, (1 << PB3)
    sts DDRB, r16

    clr r16
    sts DDRC, r16
    ldi r16, (1 << PC0)
    sts PORTC, r16

    ; init
#define TMPL r16
#define TMPH r17
#define STATE r18
#define PREV_MILLIS_L r19
#define PREV_MILLIS_H r20
    clr STATE
    clr PREV_MILLIS_L
    clr PREV_MILLIS_H

    sei

loop:
    lds TMPL, PINC
    eor TMPL, STATE  ;
    andi TMPL, 1   ; if ((state ^ PINB) & 1 != 0)
    brne loop_else ; if prev_state != cur_state
        cli
        lds PREV_MILLIS_L, MILLIS
        lds PREV_MILLIS_H, MILLIS+1
        sei
        ldi TMPL, 1
        eor STATE, TMPL ; toggle unstable state
        rjmp loop_end_if
    loop_else:
        cli
        lds TMPL, MILLIS
        lds TMPH, MILLIS+1
        sei
        sub TMPL, PREV_MILLIS_L
        sbc TMPH, PREV_MILLIS_H
        subi TMPL, lo8(INTERVAL)
        sbci TMPH, hi8(INTERVAL)
        brlt loop_end_if
            mov TMPL, STATE
            andi TMPL, 0b0011
            lsl TMPL
            andi STATE, ~0b0110
            or STATE, TMPL
    loop_end_if: 

    mov TMPL, STATE
    andi TMPL, 0b110
    cpi TMPL, 0b010
    brne if_pressed_end
        ldi TMPL, 1<<3
        eor STATE, TMPL
    if_pressed_end:

    lds TMPL, PORTB
    andi TMPL, ~(1 << PB3)
    sbrc STATE, 3
        ori TMPL, 1 << PB3
    sts PORTB, TMPL
rjmp loop


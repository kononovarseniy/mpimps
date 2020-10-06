#include <avr/io.h>

interrupts:
.org 0x00
    rjmp main
.org 0x12*2
    rjmp timer0_ovf
.org 0x30*2

digits:
    .byte 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F

get_code:
    push ZL
    push ZH
    push r17
    push r18
    push r19

    ldi r18, lo8(digits)
    ldi r19, hi8(digits)
    clr r17
    add r18, r16
    adc r19, r17

    mov ZH, r19
    mov ZL, r18
    lpm; r0, Z
    
    pop r19
    pop r18
    pop r17
    pop ZH
    pop ZL
    ret


timer0_ovf:
    push r0
    push r1
    lds r0, SREG
    push r0
    push r16
    
    mov r16, r18
    call get_code
    sts PORTA, r0
    
    inc r19
    cpi r19, 10
    brne t0o_output
        ldi r19, 0
        inc r18
        cpi r18, 10
        brne t0o_output
            ldi r18, 0
    t0o_output:

    pop r16
    pop r0
    sts SREG, r0
    pop r1
    pop r0
    reti

main:
    ; init stack
    ldi r16, lo8(RAMEND)
    sts SPL, r16
    ldi r16, hi8(RAMEND)
    sts SPH, r16

    ldi r16, (1<<CS02)+(1<<CS00); 
    ldi r17, (1<<TOIE0) ; enable interrupt
    sts TCCR0, r16
    sts TIMSK, r17
 
    ; init IO pins
    ldi r16, 0xff
    sts DDRA, r16

    ldi r18, 0        
    ldi r19, 0

    sei

loop:
rjmp loop


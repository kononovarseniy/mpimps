#include <avr/io.h>

interrupts:
.org 0x00
    rjmp main
.org 0x12*2
    rjmp timer0_ovf
.org 0x30*2


digits:
    .byte 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
    .byte 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71

#define DISP_DIGITS (RAMSTART + 0x00)


; Logical shifh left: val <<= n; n = 0
.macro shl val n
    cpi \n, 0
    breq shl_end\@
shl_loop\@:
    lsl \val
    dec \n
    brne shl_loop\@
shl_end\@:
.endm


inc_decimal:
    push r16
    push r17
    push r18
    push XH
    push XL
    
    ldi r17, 1
    ldi r18, 4
inc_decimal_loop:
    ld r16, X
    add r16, r17
    ldi r17, 0
    cpi r16, 10
    brne inc_decimal_store
        clr r16
        ldi r17, 1
    inc_decimal_store:
    st X+, r16
    dec r18
    brne inc_decimal_loop
    
    pop XL
    pop XH
    pop r18
    pop r17
    pop r16
    ret


get_code:
    push ZL
    push ZH
    push r17

    ldi ZL, lo8(digits)
    ldi ZH, hi8(digits)
    clr r17
    add ZL, r16
    adc ZH, r17

    lpm; r0, Z
    
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
    push r17

; PORTA = 0
    clr r16
    sts PORTA, r16

; PORTB = 1 << r19
    ldi r16, 1
    mov r17, r19
    shl r16, r17
    sts PORTB, r16

; PORTA = get_code(*(DISP_DIGITS + r19))
    ldi XH, hi8(DISP_DIGITS+3)
    ldi XL, lo8(DISP_DIGITS+3)
    clr r16
    sub XL, r19
    sbc XH, r16
    ld r16, X
    call get_code
    sts PORTA, r0
    
; r19 = (r19 + 1) % 4
    inc r19
    cpi r19, 4
    brne _L1
        ldi r19, 0
    _L1:

    inc r18
    cpi r18, 100
    brne t0o_output
        ldi r18, 0
        ldi XH, hi8(DISP_DIGITS)
        ldi XL, lo8(DISP_DIGITS)
        call inc_decimal
    t0o_output:

    pop r17
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

    ldi r16, (0<<CS02)+(1<<CS00)+(1<<CS00)
    ldi r17, (1<<TOIE0) ; enable interrupt
    sts TCCR0, r16
    sts TIMSK, r17
 
    ; init IO pins
    ldi r16, 0xff
    sts DDRA, r16

    ldi r16, 0x0f
    sts DDRB, r16
    sts PORTB, r16

    ldi XH, hi8(DISP_DIGITS)
    ldi XL, lo8(DISP_DIGITS)
    ldi r16, 4
    clr r17
clear_loop:
    st X+, r17
    dec r16
    brne clear_loop
    
    ldi r18, 0        
    ldi r19, 0

    sei

loop:
rjmp loop


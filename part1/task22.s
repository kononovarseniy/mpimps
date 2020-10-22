#include <avr/io.h>

interrupts:
.org 0x00
    rjmp main
.org 0x12*2
    rjmp timer0_ovf
.org 0x30*2


timer0_ovf:
    push r0
    push r1
    lds r0, SREG
    push r0
    push r16
    
    call scan_matrix
    mov r16, r0
    or r16, r1
    sts PORTA, r16
    
    pop r16
    pop r0
    sts SREG, r0
    pop r1
    pop r0
    reti

init_matrix:
    push r16

    # Cols
    ldi r16, 0b000
    sts DDRD, r16
    ldi r16, 0b111
    sts PORTD, r16
    # Rows
    ldi r16, 0x00
    sts DDRB, r16
    ldi r16, 0x0F
    sts PORTB, r16

    pop r16
    ret

scan_row:
    ; r16 - column mask
    ; r0  - result

    push r16
    push r17 

    ldi r17, 0b111
    eor r17, r16
    sts DDRD, r16
    sts PORTD, r17
 
    lds r0, PINB
    ldi r16, 0b1111
    eor r0, r16
    and r0, r16

    pop r17
    pop r16

    ret

scan_matrix:
    push r16
    push r18
    push r19

    ; Col 1
    ldi r16, (1 << 0)
    call scan_row 
    mov r18, r0

    ; Col 2
    ldi r16, (1 << 1)
    call scan_row 
    ldi r16, (1 << 4)
    mul r0, r16
    or r18, r0

    ; Col 3
    ldi r16, (1 << 2)
    call scan_row 
    mov r19, r0

    ; return
    mov r0, r18
    mov r1, r19

    pop r19
    pop r18
    pop r16
    ret    

main:
    ; init stack
    ldi r16, lo8(RAMEND)
    sts SPL, r16
    ldi r16, hi8(RAMEND)
    sts SPH, r16

    ldi r16, (1<<CS02)+(1<<CS00); 
    ldi r17, (1 << TOIE0) ; enable interrupt
    sts TCCR0, r16
    sts TIMSK, r17
 
    ; init IO pins
    ldi r16, 0xff
    sts DDRA, r16

    call init_matrix

    sei

loop:
rjmp loop


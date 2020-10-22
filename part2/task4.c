#include <avr/io.h>
#include <avr/interrupt.h>

#define ever (;;)
#define S0 (0)
#define S1 (1)

uint8_t send = (0 << S0) | (0 << S1);

ISR(USART_UDRE_vect) {
    uint8_t oldSREG = SREG;
    if (send & (1 << S0)) {
        send &= ~(1 << S0);
        UDR = '0';
        if (!send)
            UCSRB &= ~(1 << UDRIE);
        goto ret;
    }
    if (send & (1 << S1)) {
        send &= ~(1 << S1);
        UDR = '1';
        if (!send)
            UCSRB &= ~(1 << UDRIE);
        goto ret;
    }
ret:
    SREG = oldSREG;
}

ISR(USART_RXC_vect) {
    uint8_t oldSREG = SREG;
    uint8_t a = UDR;
    switch (a) {
        case '1':
            PORTB |= (1 << PB3);
            break;
        case '0':
            PORTB &= ~(1 << PB3);
            break;
    }
ret:
    SREG = oldSREG;
}

uint8_t prev_a = 1;
uint8_t prev_b = 1;

void send_signal(uint8_t val) {
    send |= (1 << val);
    UCSRB |= (1 << UDRIE);
}

void main() {
    #define FOSC 8000000
    #define BAUD 9600
    #define MYUBRR (FOSC/16/BAUD-1)

    UCSRA = 0; // Default
    UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
    /* Set frame format: 8data, 1stop bit */
    UCSRC = (1 << URSEL) | (0 << USBS) | (3 << UCSZ0);
    UBRRH = (MYUBRR >> 8);
    UBRRL = MYUBRR;

    DDRB = (0 << PB1) | (0 << PB2) | (1 << PB3);
    PORTB = (1 << PB1) | (1 << PB2);

    sei();

    for ever {
        uint8_t curr = PINB;
        uint8_t curr_a = curr & (1 << PB1);
        uint8_t curr_b = curr & (1 << PB2);
        if (curr_a != prev_a && !curr_a) {
            //PORTB |= (1 << PB3);
            send_signal(1);
        }
        if (curr_b != prev_b && !curr_b) {
            //PORTB &= ~(1 << PB3);
            send_signal(0);
        }
        prev_a = curr_a;
        prev_b = curr_b;
    }
}

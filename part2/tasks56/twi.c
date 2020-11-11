#include "twi.h"

uint8_t twi_slaw = 0;
uint8_t twi_remainder = 0;
uint8_t *twi_ptr = 0;
void (*twi_callback)(uint8_t res) = 0;

void twi_init() {
    TWBR = 123;
    TWSR = 1;
}

void twi_begin(uint8_t slaw, uint8_t len, uint8_t *ptr, void (*cb)(uint8_t)) {
    while (TWCR & (1 << TWSTO));
    twi_slaw = slaw;
    twi_remainder = len;
    twi_ptr = ptr;
    twi_callback = cb;
    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA);
}

void twi_write(uint8_t dev, uint8_t len, uint8_t *ptr, void (*cb)(uint8_t)) {
    twi_begin((dev << 1) | 0, len, ptr, cb);
}

void twi_read(uint8_t dev, uint8_t len, uint8_t *ptr, void (*cb)(uint8_t)) {
    twi_begin((dev << 1) | 1, len, ptr, cb);
}

ISR(TWI_vect) {
    uint8_t code = TWSR & 0xF8;
    switch (code) {
        case 0x08: // START transmitted
            TWDR = twi_slaw;
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            break;
        // Transmitter mode
        case 0x18: // SLA+W transmitted and ACK'ed
            TWDR = *twi_ptr;
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            break;
        case 0x20: // SLA+W transmitted and NOT ACK'ed
            TWCR = (1 << TWSTO) |  (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            twi_callback(1);
            break;
        case 0x28: // DATA transmitted and ACK'ed
            twi_ptr++;
            if (--twi_remainder > 0) {
                TWDR = *twi_ptr;
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            } else {
                TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
                twi_callback(0);
            }
            break;
        // Receiver mode
        case 0x40: // SLA+R transmitted and ACK'ed
            if (twi_remainder > 1)
                TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            else
                TWCR = (0 << TWEA) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            break;
        case 0x50: // DATA received and ACK'ed
            *twi_ptr++ = TWDR;
            if (--twi_remainder > 1)
                TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            else
                TWCR = (0 << TWEA) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            break;
        case 0x58: // DATA received and NOT ACK'ed
            *twi_ptr = TWDR;
            TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            twi_callback(0);
            break;
        // Exceptions ???
        default:
            TWCR = (1 << TWINT);
            twi_callback(code);
            break;
    }
}


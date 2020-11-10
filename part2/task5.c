#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "led.h"
#include "bio.h"

uint8_t scaler = 0;
uint8_t counter = 0;

uint32_t display[8];
uint8_t table[32];
uint8_t pos = 0;
uint8_t dot_x = 30;
uint8_t dot_y = 1;

#define BTN_L BTN(0, 3)
#define BTN_D BTN(1, 3)
#define BTN_R BTN(2, 3)
#define BTN_U BTN(1, 2)

/*uint8_t twi_write(uint8_t addr, uint8_t data) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xF8) != 0x08)
        return 0;
    TWDR = addr;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xf8) != 0x18)
        return 0;
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xf8) != 0x28)
        return 0;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    return 1;
}*/

/*uint8_t twi_read(uint8_t addr, uint8_t *data) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE);
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xF8) != 0x08)
        return 0;
    TWDR = (addr << 1) | 1;
    TWCR = (1 << TWINT) | (1 << TWEN)|(1<<TWIE);
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xf8) != 0x40)
    {
        /*lcd_put((TWSR & 0xf8) / 16 + '0');
        lcd_put((TWSR & 0xf8) % 16 + '0');*/
/*        return 0;
    }
    lcd_put('5');
    TWCR = (1 << TWINT) | (0 << TWEA) | (1 << TWEN)|(1<<TWIE);
    while (!(TWCR & (1 << TWINT)));
    lcd_put('6');
    *data = TWDR;
    //if ((TWSR & 0xf8) != 0x28)
    //    return 0;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO)|(1<<TWIE);
    return 1;
}*/

ISR(TIMER0_OVF_vect) {
    uint16_t kbs = kbd_scan();

    if ((kbs & BTN_L) && dot_x > 0) dot_x--;
    if ((kbs & BTN_R) && dot_x < 31) dot_x++;
    if ((kbs & BTN_U) && dot_y > 0) dot_y--;
    if ((kbs & BTN_D) && dot_y < 7) dot_y++;

    led_clear(display);
    led_draw_rect(display, 8, 0, 8, 8, 1);
    led_fill_rect(display, 10, 2, 4, 4, 2);
    led_fill_rect(display, pos, 0, 1, 8, 1);
    led_set(display, dot_x, dot_y, 1);
    for (int i = 0; i < 8; i++) {
        led_set(display, i, table[(i + display_tick/2) % 32], 1);
    }

    for (uint8_t r = 0; r < 4; r++) {
        for (uint8_t c = 0; c < 3; c++) {
            led_set(display, c + 29, r + 4, !(kbs & BTN(c, r)));
        }
    }

    led_show(display);
    display_tick++;

    if (++scaler < 30) return;
    scaler = 0;

    if (++counter == 10) counter = 0;

    lcd_goto(0, 15);
    lcd_put('0' + counter);
    lcd_goto(1, 12);
    lcd_shift(0, 0);
}

ISR(ADC_vect) {
    pos = ADCH / 8;
}

uint8_t twi_slaw = 0;
uint8_t twi_remainder = 0;
uint8_t *twi_ptr = 0;
void (*twi_callback)(uint8_t res) = 0;

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
            lcd_put(code / 16 + '0');
            lcd_put(code % 16 + '0');
            twi_callback(2);
            break;
    }
}
/*
ISR(TWI_vect) {
    switch (TWSR & 0xF8) {
        case 0x08: // START transmitted
        case 0x10: // RESTART transmitted
            TWCR |= (1 << TWINT);
            TWDR = twi_get_sla_rw(); // Send SLA+R/W
            break;
        case 0x18: // SLA+W transmitted and ACK'ed
            TWCR |= (1 << TWINT);
            TWDR = twi_get_byte(); // Send byte
            break;
        case 0x20: // SLW+W transmitted but NOT ACK'ed
            TWCR |= (1 << TWSTA) | (1 << TWINT); // Restart
            break;
        case 0x28: // DATA byte transmitted and ACK'ed
            TWCR |= (1 << TWSTA) | (1 << TWINT); // Complete transaction
            // TWCR |= (1 << STOP) | (1 << START) | (1 << TWINT); Complete and start new
            twi_byte_sent();
            break;
        case 0x30: // DATA byte transmitted but NOT ACK'ed
            TWCR |= (1 << TWSTA) | (1 << TWINT); // Restart
            break;
        case 0x40: // SLA+R transmitted and ACK'ed
            TWCR |= (1 << TWINT);
            break;
        case 0x48: // SLA+R transmitted but NOT ACK'ed
            TWCR |= (1 << TWSTA) | (1 << TWINT); // Restart
            break;
        case 0x50: // DATA byte received
            TWCR |= (0 << TWEA) | (1 << TWINT); // Stop receiving by sending NOT ACK
            twi_byte_received(TWDR);
        case 0x58: // DATA byte received and NOT ACK'ed
            TWCR |= (1 << TWSTO) | (1 << TWINT); // Stop transaction
        default: // Exceptional situation
            twi_error();
            break;
    }
}
*/

#define MEM_CODE 0b1010000

uint8_t mem_dev;
uint8_t mem_buf[3];
uint8_t mem_buf_size;

uint8_t *mem_ptr;
uint8_t mem_rem;
uint16_t mem_addr;

void (*mem_callback)(uint8_t res);
void (*mem_next)(uint8_t res);

void mem_write_buf_cb(uint8_t res) {
    if (res == 1) // Got NACK => write in progress
        twi_write(mem_dev, mem_buf_size, mem_buf, mem_write_buf_cb);
    else if (res)
        mem_callback(res);
    else
        mem_next(res);
}

void mem_read_cb(uint8_t res) {
    if (res) {
        mem_callback(res);
    } else {
        twi_read(mem_dev, mem_rem, mem_ptr, mem_callback); 
    }
}
void mem_read(uint8_t dev, uint16_t addr, uint8_t len, uint8_t *data, void (*cb)(uint8_t)) {
    mem_dev = MEM_CODE | dev;
    mem_callback = cb;

    mem_ptr = data;
    mem_rem = len;

    mem_buf_size = 2;
    mem_buf[0] = addr >> 8;
    mem_buf[1] = addr;
    mem_next = mem_read_cb;
    mem_write_buf_cb(1);
}

void mem_write_cb(uint8_t res) {
    if (res) {
        mem_callback(res);
    } else if (mem_rem == 0) {
        mem_callback(0);
    } else {
        mem_buf_size = 3;
        mem_buf[0] = mem_addr >> 8;
        mem_buf[1] = mem_addr;
        mem_buf[2] = *mem_ptr;
        mem_next = mem_write_cb;
        mem_write_buf_cb(1);
        mem_addr++;
        mem_ptr++;
        mem_rem--;
    }
}
void mem_write(uint8_t dev, uint16_t addr, uint8_t len, uint8_t *data, void (*cb)(uint8_t)) {
    mem_dev = MEM_CODE | dev;
    mem_callback = cb;

    mem_ptr = data;
    mem_rem = len;
    mem_addr = addr;

    mem_write_cb(0);
}

/*
uint8_t mem_dev;
uint16_t mem_addr;
uint8_t mem_tmp[3];
uint8_t *mem_ptr;
uint8_t mem_rem;
void (*mem_callback)(uint8_t res);
void (*mem_next)();

void mem_special_write()


void mem_write_addr_cb(uint8_t res) {
    if (res == 1)         mem_write_addr();
    else if (res)
        mem_callback(res);
    mem_next();
}
void mem_write_addr() {
    mem_tmp[0] = mem_addr >> 8;
    mem_tmp[1] = mem_addr;
    twi_write(mem_dev, 2, mem_tmp, mem_write_addr_cb);
}

void mem_read_cb(uint8_t res) {
    if (res) {
        mem_callback(res);
    } else {
        twi_read(mem_dev, mem_rem, mem_ptr, mem_callback); 
    }
}
void mem_read(uint8_t dev, uint16_t addr, uint8_t len, uint8_t *data, void (*cb)(uint8_t)) {
    mem_dev = MEM_CODE | dev;
    mem_ptr = data;
    mem_addr = addr;
    mem_rem = len;
    mem_callback = cb;
    mem_next = mem_read_cb;
    mem_special_write_cb(1);
}

void mem_write_callback(uint8_t res) {
    if (res) {
        mem_callback(res);
    } else {
        mem_tmp[0] = mem_addr >> 8;
        mem_tmp[1] = mem_addr;
        mem_tmp[2] = *mem_ptr++;
        mem_addr++;
        twi_write(mem_dev, 3, mem_tmp, --mem_rem ? mem_write_callback : mem_callback);
    }
}
void mem_write(uint8_t dev, uint16_t addr, uint8_t len, uint8_t *data, void (*cb)(uint8_t)) {
    mem_dev = MEM_CODE | dev;
    mem_addr = addr;
    mem_rem = len;
    mem_ptr = data;
    mem_callback = cb;
    mem_write_callback(0);
}*/

uint8_t tmp_twi_data[8];
void test_cb2(uint8_t res) {
    lcd_put('?');
    lcd_put(res + '0');
}

void test_cb(uint8_t res) {
    lcd_put('!');
    lcd_put(res + '0');

    twi_read(0b1010111, 2, tmp_twi_data, test_cb2);
}

void test_mem_write_cb(uint8_t res) {
    lcd_put('W');
    lcd_put(res + '0');
}

void test_mem_read_cb(uint8_t res) {
    lcd_put('R');
    lcd_put(res + '0');
    uint8_t b = tmp_twi_data[0];
    lcd_put(b / 10 + '0');
    lcd_put(b % 10 + '0');
    b = tmp_twi_data[1];
    lcd_put(b / 10 + '0');
    lcd_put(b % 10 + '0');
}

void main() {
    led_clear(display);
    for (int i = 0; i < 32; i++)
        table[i] = i % 8;

    TCCR0 = (1 << CS02) | (0 << CS01) | (1 << CS00);
    TIMSK = (1 << TOIE0);

    lcd_init();
    led_init();
    adc_init();
    kbd_init();

    TWBR = 123;
    TWSR = 1;

    sei();

    //tmp_twi_data[0] = 88;
    //tmp_twi_data[1] = 99;
    //mem_write(0b111, 0x0000, 2, tmp_twi_data, test_mem_write_cb);
    mem_read(0b111, 0x0000, 2, tmp_twi_data, test_mem_read_cb);
    /*uint8_t data[] = {0, 0};
    twi_write(0b1010111, 2, data, test_cb);*/

    //TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);
    /*uint8_t data;
    twi_read(0, &data);/*

    /*lcd_goto(0, 5);
    lcd_print("Hello");
    lcd_goto(1, 4);
    lcd_print("world!!!");

    lcd_shift(0, 0);*/

    for (;;);
}

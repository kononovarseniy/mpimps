#include "mem.h"

#include "twi.h"

uint8_t mem_dev;
uint8_t mem_buf[3];
uint8_t mem_buf_size;

uint8_t *mem_ptr;
uint8_t mem_rem;
uint16_t mem_addr;

void (*mem_callback)(uint8_t res);
void (*mem_next)(uint8_t res);

void mem_init() {
}

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


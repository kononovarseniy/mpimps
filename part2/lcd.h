#ifndef LCD_H
#define LCD_H

#include <avr/io.h>

void hd_write(uint8_t data, uint8_t is_data);
uint8_t hd_read(uint8_t is_data);
void hd_cmd(uint8_t cmd);
void hd_data(uint8_t data);

#define lcd_clear() \
    hd_cmd(0x01)
#define lcd_home() \
    hd_cmd(0x02)
#define lcd_entry_mode(DIR, SHIFT) \
    hd_cmd(0x04 | (DIR << 1) | (SHIFT << 0))
#define lcd_on_off_ctl(DISP, CUR, BLOCK) \
    hd_cmd(0x08 | (DISP << 2) | (CUR << 1) | (BLOCK << 0))
#define lcd_shift(SC, RL) \
    hd_cmd(0x10 | (SC << 3) | (RL << 2))
#define lcd_function(DATA, LINES, FONT) \
    hd_cmd(0x20 | (DATA << 4) | (LINES << 3) | (FONT << 2))
#define lcd_cgram(addr) \
    hd_cmd(0x40 | (addr & 0x3F))
#define lcd_ddram(addr) \
    hd_cmd(0x80 | (addr & 0x7F))

void lcd_goto(uint8_t y, uint8_t x);
void lcd_put(char ch);
void lcd_print(char *str);
void lcd_init();

#endif

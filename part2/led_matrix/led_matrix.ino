#include <SPI.h>

#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define MOSI_PIN PB3
#define MISO_PIN PB4
#define SCK_PIN PB5

const int ss_pin = 10;

void spi_transfer(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)));
}

void send_all(uint8_t reg, uint8_t val) {
  PORTB &= ~(1 << PB2);
  for (int i = 0; i < 4; i++) {
    spi_transfer(reg);
    spi_transfer(val);
  }
  PORTB |= (1 << PB2);
}

void led_init() {
  DDRB |= (1 << PB2);
  //SPI.begin();
  SPI_DDR |= (1 << MOSI_PIN) | (1 << SCK_PIN);
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (0 << SPR1); // clk/16, master, MSB first


  send_all(0xF, 1); // test on
  delay(200);
  send_all(0xF, 0); // test off
  send_all(0x9, 0); // no decode
  send_all(0xA, 0); // lowiest intensity
  send_all(0xB, 7); // all lines
  send_all(0xC, 1); // turn on
}

uint8_t display_tick = 0;
uint32_t display[8] = {0};

void led_clear(uint32_t *display) {
  uint8_t *ptr = (uint8_t *)display;
  for (uint8_t i = 0; i < 32; i++)
    *ptr++ = 0;
}

void led_set(uint32_t *display, uint8_t x, uint8_t y, uint8_t color) {
  uint8_t *line = (uint8_t *)display + 4 * y;
  uint8_t *ptr = line + 3 - x / 8;
  uint8_t mask = 0x80 >> (x % 8);
  if (color == 2)
    color = (x ^ y ^ display_tick) & 1;
  if (color)
    *ptr |= mask;
  else
    *ptr &= ~mask;
}

void led_shift(uint8_t col) {
  for (uint8_t i = 0; i < 8; i++) {
    display[i] <<= 1;
    display[i] |= col & 1;
    col >>= 1;
  }
}

void led_show(uint32_t *disp) {
  for (int i = 0; i < 8; i++) {
    uint8_t *ptr = (uint8_t *)(disp++) + 3;
    PORTB &= ~(1 << PB2);
    for (int j = 0; j < 4; j++) {
      spi_transfer(i + 1);
      spi_transfer(*ptr--);
    }
    PORTB |= (1 << PB2);
  }
}

void led_fill_rect(uint32_t *disp, uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color) {
  for (uint8_t x = x0; x < x0 + w; x++)
    for (uint8_t y = y0; y < y0 + h; y++)
      led_set(display, x, y, color);
}

void led_draw_rect(uint32_t *disp, uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color) {
  for (uint8_t x = x0; x < x0 + w; x++) {
    led_set(display, x, y0, color);
    led_set(display, x, y0 + h - 1, color);
  }
  for (uint8_t y = y0 + 1; y < y0 + h; y++) {
    led_set(display, x0, y, color);
    led_set(display, x0 + w - 1, y, color);
  }
}

uint8_t table[32];

void setup() {
  led_init();
  led_clear(display);
  for (int i = 0; i < 32; i++) {
    table[i] = i % 8;
  }
}

uint8_t cnt = 0;
void loop() {
  led_clear(display);
  led_fill_rect(display, 2, 2, 4, 4, 2);
  led_draw_rect(display, 8, 0, 8, 8, 1);
  led_draw_rect(display, 10, 2, 4, 4, 2);
  led_fill_rect(display, 19, 3, 10, 2, 1);
  for (int i = 0; i < 32; i++) {
    led_set(display, i, table[(i + display_tick / 2) % 32], 1);
  }
  led_show(display);
  display_tick++;
  delay(20);
  /*
    led_shift(cnt++);
    led_show(display1);
    delay(20);
    led_show(display2);
    delay(20);*/
}

PORT = /dev/ttyUSB0
CONF = /etc/avrdude.conf
GCC = avr-gcc -mmcu=$(PROC)

all: ;
clear:
	rm -r bin
bin:
	mkdir bin

# Here % is the name of program source file without extension
# $* is replaced by this name
make-program-%: %.c bin
	$(GCC) -I/usr/avr/include $*.c -o bin/$*.elf
	avr-objcopy -O ihex bin/$*.elf bin/$*.hex

avrdude-%: bin/%.hex
	avrdude -C $(CONF) -p $(PROC) -c $(PROG) -P $(PORT) -U "flash:w:bin/$*.hex:i"

upload-program-%: make-program-%
	avrdude -C $(CONF) -p $(PROC) -c $(PROG) -P $(PORT) -U "flash:w:bin/$*.hex:i"

make-%: PROC = atmega16
make-%: make-program-task% ;

upload-%: PROC = atmega16
upload-%: PROG = pinboard
upload-%: upload-program-task% ;

bin/lcd.elf: main.c lcd.c lcd.h led.c led.h bio.c bio.h twi.c twi.h mem.c mem.h bin
	$(GCC) -I/usr/avr/include main.c lcd.c led.c bio.c twi.c mem.c -o bin/lcd.elf

bin/lcd.hex: bin/lcd.elf
	avr-objcopy -O ihex bin/lcd.elf bin/lcd.hex

upload: PROC = atmega16
upload: PROG = pinboard
upload: avrdude-lcd ;


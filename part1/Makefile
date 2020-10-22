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
make-program-%: %.s bin
	$(GCC) -c -x assembler-with-cpp -I/usr/avr/include $*.s -o bin/$*.o
	$(GCC) -nostartfiles bin/$*.o -o bin/$*.elf
	avr-objcopy -O ihex bin/$*.elf bin/$*.hex

upload-program-%: make-program-%
	avrdude -C $(CONF) -p $(PROC) -c $(PROG) -P $(PORT) -U "flash:w:bin/$*.hex:i"

# Here % is the task number with 'a' suffix for arduino version
make-%a: PROC = atmega328p
make-%a: make-program-task%a ;

make-%: PROC = atmega16
make-%: make-program-task% ;

upload-%a: PROC = atmega328p
upload-%a: PROG = arduino
upload-%a: upload-program-task%a ;

upload-%: PROC = atmega16
upload-%: PROG = pinboard
upload-%: upload-program-task% ;


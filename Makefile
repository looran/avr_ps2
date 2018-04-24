CFLAGS = -Os -DF_CPU=16000000UL -mmcu=atmega328p

all: example

example:
	avr-gcc -Wall -Wextra $(CFLAGS) -o example example.c ps2.c
	avr-objcopy -O ihex -R .eeprom example example.hex

clean:
	rm -f example{,.hex}

.PHONY: example

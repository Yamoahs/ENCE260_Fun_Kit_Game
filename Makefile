# File:   Makefile
# Author: M. P. Hayes, UCECE
# Date:   12 Sep 2010
# Descr:  Makefile for GAME3

# Definitions.
CC = avr-gcc
CFLAGS = -mmcu=atmega32u2 -Os -Wall -Wstrict-prototypes -Wextra -g -I../../drivers/avr -I../../utils -I../../drivers
OBJCOPY = avr-objcopy
SIZE = avr-size
DEL = rm


# Default target.
all: GAME3.out


# Compile: create object files from C source files.
GAME3.o: GAME3.c ../../drivers/avr/pio.h ../../drivers/avr/system.h ../../utils/pacer.h
	$(CC) -c $(CFLAGS) $< -o $@

pio.o: ../../drivers/avr/pio.c ../../drivers/avr/pio.h ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

system.o: ../../drivers/avr/system.c ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

timer.o: ../../drivers/avr/timer.c ../../drivers/avr/system.h ../../drivers/avr/timer.h
	$(CC) -c $(CFLAGS) $< -o $@

pacer.o: ../../utils/pacer.c ../../drivers/avr/system.h ../../drivers/avr/timer.h ../../utils/pacer.h
	$(CC) -c $(CFLAGS) $< -o $@

navswitch.o: ../../drivers/navswitch.c ../../drivers/avr/delay.h ../../drivers/avr/pio.h ../../drivers/avr/system.h ../../drivers/navswitch.h
	$(CC) -c $(CFLAGS) $< -o $@

display.o: ../../drivers/display.c ../../drivers/avr/system.h ../../drivers/display.h ../../drivers/ledmat.h
	$(CC) -c $(CFLAGS) $< -o $@

ledmat.o: ../../drivers/ledmat.c ../../drivers/avr/pio.h ../../drivers/avr/system.h ../../drivers/ledmat.h
	$(CC) -c $(CFLAGS) $< -o $@



# Link: create ELF output file from object files.
GAME3.out: GAME3.o pio.o system.o timer.o pacer.o navswitch.o display.o ledmat.o
	$(CC) $(CFLAGS) $^ -o $@ -lm
	$(SIZE) $@


# Target: clean project.
.PHONY: clean
clean:
	-$(DEL) *.o *.out *.hex


# Target: program project.
.PHONY: program
program: GAME3.out
	$(OBJCOPY) -O ihex GAME3.out GAME3.hex
	dfu-programmer atmega32u2 erase; dfu-programmer atmega32u2 flash GAME3.hex; dfu-programmer atmega32u2 start
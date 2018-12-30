PREFIX = /usr/local
ARCH = h8300-elf
BINDIR = $(PREFIX)/bin
ADDNAME = $(ARCH)-

AR = $(BINDIR)/$(ADDNAME)ar
AS = $(BINDIR)/$(ADDNAME)as
CC = $(BINDIR)/$(ADDNAME)gcc
LD = $(BINDIR)/$(ADDNAME)ld
NM = $(BINDIR)/$(ADDNAME)nm
OBJCOPY = $(BINDIR)/$(ADDNAME)objcopy
OBJDUMP = $(BINDIR)/$(ADDNAME)objdump
RANLIB = $(BINDIR)/$(ADDNAME)ranlib
STRIP = $(BINDIR)/$(ADDNAME)strip

OBJS = serial.o lib.o xmodem.o elf.o interrupt.o

TARGET = libcommon.a

CFLAGS = -Wall -mh -nostdinc -nostdlib -fno-builtin
CFLAGS += -I.
CFLAGS += -Os

LFLAGS = -static -T ld.scr -L.

.SUFFIXES: .c .o
.SUFFIXES: .s .o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(AR) rcs $(TARGET) $(OBJS)

.c.o :	$<
	$(CC) -c $(CFLAGS) $<

.s.o :	$<
	$(CC) -c $(CFLAGS) $<

clean :
	rm -f $(OBJS) $(TARGET)
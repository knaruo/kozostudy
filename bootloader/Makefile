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

# H8WRITE = $(BINDIR)/h8write
# H8WRITE = ../h8write
H8WRITE = ../kz_h8write/bin/kz_h8write.exe

H8WRITE_SERDEV = /dev/ttyS0

OBJS = vector.o startup.o main.o intr.o
LIBS = ../lib/libcommon.a

TARGET = kzload

CFLAGS = -Wall -mh -nostdinc -nostdlib -fno-builtin
CFLAGS += -I.
CFLAGS += -I../lib
CFLAGS += -Os
CFLAGS += -DKZLOAD

LFLAGS = -static -T ld.scr -L.

.SUFFIXES: .c .o
.SUFFIXES: .s .o
.SUFFIXES: .S .o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(TARGET) $(CFLAGS) $(LFLAGS)
	cp $(TARGET) $(TARGET).elf
	$(STRIP) $(TARGET)

.c.o :	$<
	$(CC) -c $(CFLAGS) $<

.s.o :	$<
	$(CC) -c $(CFLAGS) $<

.S.o :	$<
	$(CC) -c $(CFLAGS) $<

$(TARGET).mot : $(TARGET)
	$(OBJCOPY) -O srec $(TARGET) $(TARGET).mot

image : $(TARGET).mot

write : $(TARGET).mot
	$(H8WRITE) -3069 -f20 $(TARGET).mot $(H8WRITE_SERDEV)

clean :
	rm -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).mot

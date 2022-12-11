SHELL   = /bin/sh
CC      = gcc
CFLAGS  = -fPIC -g -Iinclude -Wall -Wextra -I ../../include
LDFLAGS = -shared

TARGET  = decode_packets
SOURCES = $(shell echo *.c ../../src/*.c)
HEADERS = $(shell echo *.h ../../include/*.h)
OBJECTS = $(SOURCES:.c=.o)

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

clean:
	rm -rf src/*.o

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(OBJECTS)
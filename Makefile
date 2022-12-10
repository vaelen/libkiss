SHELL   = /bin/sh
CC      = gcc
CFLAGS  = -fPIC -g -Iinclude -Wall -Wextra -shared
LDFLAGS = -shared

TARGET  = libkiss.so
SOURCES = $(shell echo src/*.c)
HEADERS = $(shell echo include/*.h)
OBJECTS = $(SOURCES:.c=.o)

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

clean:
	rm -rf src/*.o

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(OBJECTS)
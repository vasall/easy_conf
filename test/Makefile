TARGET = test

CC ?= gcc
RM ?= rm -f

CFLAGS = -Wall -Wextra -Og -g -I..

SOURCES = main.c
OBJECTS = $(SOURCES:%.c=%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $^

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	$(RM) $(TARGET) $(OBJECTS)

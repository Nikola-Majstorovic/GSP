CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`
IDIR = include
SDIR = src

CFLAGS += -I$(IDIR)

TARGET = sequence_miner

SOURCES = main.c $(wildcard $(SDIR)/*.c)

OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(SDIR)/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: clean run
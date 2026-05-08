CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS = -lpthread

SRC = src/main.c src/hilos.c src/sync.c
TARGET  = terminal

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) src/*.o
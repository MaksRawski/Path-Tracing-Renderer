CC = gcc
CFLAGS = -Wall -Wpedantic -Ilib/include
LDFLAGS = -lglfw -lGL
TARGET = bin/main
GLAD_SRC = ./lib/src/gl.c

$(TARGET): main.c $(GLAD_SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

clean:
	rm -rf bin/*

.PHONY: clean

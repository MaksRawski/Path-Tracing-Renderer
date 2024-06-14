CC = gcc
CFLAGS = -Wall -Wpedantic -Ilib/include
LDFLAGS = -lglfw -lGL
TARGET = bin/main
GLAD_SRC = ./lib/src/gl.c

$(TARGET): main.c $(GLAD_SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

zip: projekt.zip

projekt.zip: $(TARGET) main.c
	rm -rf projekt
	mkdir projekt
	rsync -qav . --exclude projekt projekt/
	cd projekt && git clean -fx && rm -rf .git .gitignore
	zip -r projekt.zip projekt
	rm -rf projekt

clean:
	rm -rf bin/* projekt.zip

.PHONY: clean zip

CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused -Ilib/include
LDFLAGS = -lglfw -ldl
TARGET = bin/main
GLAD_SRC = ./lib/src/gl.c

$(TARGET): main.c $(GLAD_SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

zip: projekt.zip

projekt.zip: $(TARGET) main.c
	mkdir -p projekt
	rm -rf projekt/*
	rsync -qa . --exclude projekt projekt/
	cd projekt && git clean -fx && rm -rf .git .gitignore
	zip -r projekt.zip projekt
	rm -rf projekt

clean:
	rm -rf bin/* projekt.zip

.PHONY: clean zip

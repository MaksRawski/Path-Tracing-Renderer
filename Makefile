CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused -Ilib/include
LDFLAGS = -lglfw -ldl
TARGET = bin/main
SRC = main.c renderer.c obj_parser.c
OBJ = $(patsubst %.c, bin/%.o, $(SRC))
GLAD_SRC = ./lib/src/gl.c

all: $(TARGET)

$(TARGET): $(OBJ) $(GLAD_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

bin/%.o: %.c
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

zip: projekt.zip

projekt.zip: $(TARGET) $(SRC)
	mkdir -p projekt
	rm -rf projekt/*
	rsync -qa . --exclude projekt projekt/
	cd projekt && git clean -fx && rm -rf .git .gitignore
	zip -r projekt.zip projekt
	rm -rf projekt

clean:
	rm -rf bin/* projekt.zip

.PHONY: all clean zip

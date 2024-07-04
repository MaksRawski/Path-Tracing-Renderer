CC = zig cc
CFLAGS = -Wall -Wextra -Wno-unused -Ilib/include --target=x86_64-windows
LDFLAGS = -Llib/src -lglfw3 -lopengl32 -lgdi32 -lwinmm -static
TARGET = bin/main.exe
SRC = main.c renderer.c obj_parser.c
OBJ = $(patsubst %.c, bin/%.o, $(SRC))
SHADERS = vertex.glsl fragment.glsl
SHADERS_H = $(patsubst %.glsl, %.h, $(SHADERS))
GLAD_SRC = lib/src/gl.c

all: $(SHADERS_H) $(TARGET)

$(TARGET): $(OBJ) $(GLAD_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET)

bin/%.o: %.c
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

%.h: %.glsl
	xxd -i $^ $@

zip: projekt.zip

projekt.zip: $(TARGET) $(SRC)
	mkdir -p projekt
	rm -rf projekt/*
	rsync -qa . --exclude projekt projekt/
	cd projekt && git clean -fx && rm -rf .git .gitignore
	zip -r projekt.zip projekt
	rm -rf projekt

clean:
	rm -rf bin/* projekt.zip $(SHADERS_H)

.PHONY: all clean zip

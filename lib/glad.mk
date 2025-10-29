include vars.mk

CC = clang
CFLAGS = -fPIC -Iglad/include

$(GLAD_TARGET): glad/src/gl.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $(LIB_BUILD_DIR)/glad/gl.o
	ar rcs $@ $(LIB_BUILD_DIR)/glad/gl.o

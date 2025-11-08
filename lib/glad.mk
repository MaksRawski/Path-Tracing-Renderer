include vars.mk

GLAD_CC = clang
GLAD_CFLAGS = -fPIC -Iglad/include

$(GLAD_TARGET): glad/src/gl.c
	@mkdir -p $(dir $@)
	$(GLAD_CC) $(GLAD_CFLAGS) -c $< -o $(LIB_BUILD_DIR)/glad/gl.o
	ar rcs $@ $(LIB_BUILD_DIR)/glad/gl.o

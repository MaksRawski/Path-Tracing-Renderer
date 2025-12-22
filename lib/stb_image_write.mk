include vars.mk

stb_image_write_CC = clang
stb_image_write_CFLAGS = -fPIC

$(stb_image_write_TARGET): $(stb_image_write_INCLUDE_DIR)/stb_image_write.c
	@mkdir -p $(dir $@)
	$(stb_image_write_CC) $(stb_image_write_CFLAGS) -c $< -o $(LIB_BUILD_DIR)/stb_image_write/stb_image_write.o
	ar rcs $@ $(LIB_BUILD_DIR)/stb_image_write/stb_image_write.o

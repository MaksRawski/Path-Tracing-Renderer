include vars.mk

CGLTF_CC = clang
CGLTF_CFLAGS = -fPIC

$(CGLTF_TARGET): $(CGLTF_INCLUDE_DIR)/cgltf.c
	@mkdir -p $(dir $@)
	$(CGLTF_CC) $(CGLTF_CFLAGS) -c $< -o $(LIB_BUILD_DIR)/cgltf/cgltf.o
	ar rcs $@ $(LIB_BUILD_DIR)/cgltf/cgltf.o

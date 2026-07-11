LIB_BUILD_DIR = build
CC = clang
CXX = clang++

CIMGUI_TARGET = $(LIB_BUILD_DIR)/cimgui/libcimgui.a
GLAD_TARGET = $(LIB_BUILD_DIR)/glad/libglad.a
GLFW_TARGET = $(LIB_BUILD_DIR)/glfw/src/libglfw3.a
CGLTF_TARGET = $(LIB_BUILD_DIR)/cgltf/cgltf.a
stb_image_write_TARGET = $(LIB_BUILD_DIR)/stb_image_write/stb_image_write.a
TRACY_TARGET = $(LIB_BUILD_DIR)/tracy/libTracyClient.a

CIMGUI_INCLUDE_DIR = cimgui
GLAD_INCLUDE_DIR = glad/include
GLFW_INCLUDE_DIR = glfw/include
CGLTF_INCLUDE_DIR = cgltf
stb_image_write_INCLUDE_DIR = stb_image_write
TRACY_INCLUDE_DIR = tracy/public/tracy

LIB_TARGETS = $(CIMGUI_TARGET) $(GLAD_TARGET) $(GLFW_TARGET) $(CGLTF_TARGET) $(stb_image_write_TARGET)
LIB_INCLUDE_PATHS = $(CIMGUI_INCLUDE_DIR) $(GLAD_INCLUDE_DIR) $(GLFW_INCLUDE_DIR) $(CGLTF_INCLUDE_DIR) $(stb_image_write_INCLUDE_DIR)

ifeq ($(MODE), profile)
	LIB_TARGETS += $(TRACY_TARGET)
	LIB_INCLUDE_PATHS += $(TRACY_INCLUDE_DIR)
	TRACY_SERVER_DIR = $(LIB_BUILD_DIR)/tracy-server
	TRACY_SERVER_PROFILER = $(TRACY_SERVER_DIR)/tracy-profiler
endif

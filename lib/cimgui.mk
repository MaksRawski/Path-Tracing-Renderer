include vars.mk

CIMGUI_SRC  = cimgui/cimgui.cpp
CIMGUI_SRC += cimgui/imgui/imgui.cpp
CIMGUI_SRC += cimgui/imgui/imgui_draw.cpp
CIMGUI_SRC += cimgui/imgui/imgui_demo.cpp
CIMGUI_SRC += cimgui/imgui/imgui_tables.cpp
CIMGUI_SRC += cimgui/imgui/imgui_widgets.cpp
CIMGUI_SRC += cimgui/imgui/backends/imgui_impl_glfw.cpp
CIMGUI_SRC += cimgui/imgui/backends/imgui_impl_opengl3.cpp

CIMGUI_OBJ = $(CIMGUI_SRC:%.cpp=$(LIB_BUILD_DIR)/%.o)

CIMGUI_CXX = clang++
CIMGUI_CXXFLAGS = -O2 -fno-exceptions -fno-rtti
CIMGUI_CXXFLAGS += -DIMGUI_IMPL_API="extern \"C\""
CIMGUI_CXXFLAGS += -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
CIMGUI_CXXFLAGS += -Icimgui/imgui/
CIMGUI_CXXFLAGS += -Wall

ifeq ($(shell uname -s), Linux)
	CIMGUI_CXXFLAGS += -fPIC
endif

# ifeq ($(OS), Windows_NT)
# 	LINKFLAGS = -limm32
# endif

$(CIMGUI_TARGET): $(CIMGUI_OBJ)
	ar -rcs $(CIMGUI_TARGET) $(CIMGUI_OBJ)

$(LIB_BUILD_DIR)/cimgui/%.o: cimgui/%.cpp
	@mkdir -p $(dir $@)
	$(CIMGUI_CXX) $(CIMGUI_CXXFLAGS) -c $< -o $@

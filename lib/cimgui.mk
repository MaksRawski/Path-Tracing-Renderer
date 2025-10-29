include vars.mk

SRC  = cimgui/cimgui.cpp
SRC += cimgui/imgui/imgui.cpp
SRC += cimgui/imgui/imgui_draw.cpp
SRC += cimgui/imgui/imgui_demo.cpp
SRC += cimgui/imgui/imgui_tables.cpp
SRC += cimgui/imgui/imgui_widgets.cpp
SRC += cimgui/imgui/backends/imgui_impl_glfw.cpp
SRC += cimgui/imgui/backends/imgui_impl_opengl3.cpp

OBJ = $(SRC:%.cpp=$(LIB_BUILD_DIR)/%.o)

CXX = clang++
CXXFLAGS = -O2 -fno-exceptions -fno-rtti
CXXFLAGS += -DIMGUI_IMPL_API="extern \"C\""
CXXFLAGS += -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
CXXFLAGS += -Icimgui/imgui/
CXXFLAGS += -Wall

ifeq ($(shell uname -s), Linux)
	CXXFLAGS += -fPIC
endif

ifeq ($(OS), Windows_NT)
	LINKFLAGS = -limm32
endif

$(CIMGUI_TARGET): $(OBJ)
	ar -rcs $(CIMGUI_TARGET) $(OBJ)

$(LIB_BUILD_DIR)/cimgui/%.o: cimgui/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

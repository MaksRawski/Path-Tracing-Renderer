# GLFW
find_library(GLFW_LIBRARY glfw)

# GLAD
add_library(glad STATIC
    glad/src/gl.c
)
target_include_directories(glad PUBLIC
    glad/include
)
if(UNIX)
    set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# cgltf
add_library(cgltf STATIC
    cgltf/cgltf.c
)
target_include_directories(cgltf PUBLIC
    cgltf
)
if(UNIX)
    set_target_properties(cgltf PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# cimgui
set(CIMGUI_SOURCES
    cimgui/cimgui.cpp
    cimgui/imgui/imgui.cpp
    cimgui/imgui/imgui_draw.cpp
    cimgui/imgui/imgui_demo.cpp
    cimgui/imgui/imgui_tables.cpp
    cimgui/imgui/imgui_widgets.cpp
    cimgui/imgui/backends/imgui_impl_glfw.cpp
    cimgui/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(cimgui STATIC ${CIMGUI_SOURCES})

target_include_directories(cimgui PUBLIC
    cimgui
    cimgui/imgui
)

target_compile_definitions(cimgui PUBLIC
    IMGUI_IMPL_API=extern\ \"C\"
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
)

target_compile_options(cimgui PRIVATE
    -O2
    -fno-exceptions
    -fno-rtti
    -Wall
)

if(UNIX)
    set_target_properties(cimgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(cimgui PUBLIC glfw)

# Link IMM32 on Windows for imgui
if(WIN32)
    target_link_libraries(cimgui PRIVATE imm32)
endif()

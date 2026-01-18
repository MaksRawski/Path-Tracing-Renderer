# GTK - required for opening native file dialog on linux, I don't like this either
if(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK REQUIRED gtk+-3.0)
  if(GTK_FOUND)
    add_library(gtk INTERFACE)
    target_link_libraries(gtk INTERFACE ${GTK_LIBRARIES})
    target_include_directories(gtk INTERFACE ${GTK_INCLUDE_DIRS})
  else()
    message(ERROR "Failed to find GTK library (required for native file dialogs on Linux)")
  endif()
endif()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# GLAD
add_library(glad glad/src/gl.c)
target_include_directories(glad PUBLIC glad/include)
if(UNIX)
    set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# cgltf
add_library(cgltf cgltf/cgltf.c)
target_include_directories(cgltf PUBLIC cgltf)
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

add_library(cimgui ${CIMGUI_SOURCES})

target_include_directories(cimgui PUBLIC
    cimgui
    cimgui/imgui
)

target_compile_definitions(cimgui PUBLIC
    IMGUI_IMPL_API=extern\ \"C\"
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
)

if(UNIX)
    target_compile_options(cimgui PRIVATE
        -O2
        -fno-exceptions
        -fno-rtti
        -Wall
    )
    set_target_properties(cimgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(cimgui PUBLIC glfw)

# Link IMM32 on Windows for imgui
if(WIN32)
    target_link_libraries(cimgui PRIVATE imm32)
elseif(UNIX)
    target_link_libraries(cimgui PRIVATE X11)
endif()

# stb_image_write
add_library(stb_image_write stb_image_write/stb_image_write.c)
target_include_directories(stb_image_write PUBLIC stb_image_write)
if(UNIX)
    set_target_properties(stb_image_write PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

cmake_minimum_required(VERSION 3.15)

# project is written in C but has to be linked with a C++ library (Dear ImGui)
project(PathTracingRenderer C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# build dependencies
add_subdirectory(lib)

if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} /Zi /Od")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} /O2 /DNDEBUG")
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused -pedantic-errors")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

file(GLOB_RECURSE PROJECT_SOURCES "src/*.c")
add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

set(LIBRARIES glfw glad cgltf cimgui)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${LIBRARIES})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION .)
install(DIRECTORY fonts DESTINATION .)
install(DIRECTORY shaders DESTINATION .)

# TODO: tests using CMake's CTest https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html#testing-with-cmake-and-ctest

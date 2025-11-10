cmake_minimum_required(VERSION 3.15)
project(MyProject C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} /Zi /Od")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} /O2 /DNDEBUG")
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused -Werror -Wpedantic")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories
add_subdirectory(lib)

# Find required system libraries
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# find_library(DL_LIBRARY dl)
# find_library(M_LIBRARY m)

# Collect source files
file(GLOB_RECURSE PROJECT_SOURCES "src/*.c")

# Main executable
add_executable(main ${PROJECT_SOURCES})

target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(main PRIVATE
    cimgui
    glad
    glfw
    cgltf
)

if(UNIX AND NOT APPLE)
    target_link_libraries(main PRIVATE ${X11_LIBRARIES})
endif()

if(DL_LIBRARY)
    target_link_libraries(main PRIVATE ${DL_LIBRARY})
endif()

# Tests
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES "tests/*.c")
    list(REMOVE_ITEM PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")
    
    add_executable(tests ${TEST_SOURCES} ${PROJECT_SOURCES})
    
    target_include_directories(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/tests
    )
    
    target_link_libraries(tests PRIVATE
        cimgui
        glad
        glfw
        cgltf
    )
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(tests PRIVATE ${X11_LIBRARIES})
    endif()
    
    if(DL_LIBRARY)
        target_link_libraries(tests PRIVATE ${DL_LIBRARY})
    endif()
    
    enable_testing()
    add_test(NAME tests COMMAND tests)
endif()

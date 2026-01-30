cmake_minimum_required(VERSION 3.15)

# project is written in C but has to be linked with a C++ library (Dear ImGui)
project(PathTracingRenderer C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# build dependencies
add_subdirectory(lib)

if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} /Zi /Od")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} /O2 /DNDEBUG")
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused -pedantic-errors")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

file(GLOB_RECURSE PROJECT_SOURCES "src/*.c")
add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

set(LIBRARIES glfw glad cgltf cimgui stb_image_write)
if (UNIX)
    list(APPEND LIBRARIES gtk)
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${LIBRARIES})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION .)
install(DIRECTORY fonts DESTINATION .)
install(DIRECTORY shaders DESTINATION .)

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES "tests/*.c")
    list(REMOVE_ITEM PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")
    
    add_executable(tests ${TEST_SOURCES} ${PROJECT_SOURCES})
    
    target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)
    target_link_libraries(tests PRIVATE ${LIBRARIES})
    
    enable_testing()
    add_test(NAME tests COMMAND tests)
endif()

cmake_minimum_required(VERSION 3.28)
project(FPS)

# Remove the old build directory if it exists
if(EXISTS "${CMAKE_BINARY_DIR}")
    file(REMOVE_RECURSE ${CMAKE_BINARY_DIR})
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 (assumes SDL2 is installed in vcpkg)
find_package(SDL2 REQUIRED)

# GLAD
add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include")

# OpenGL
find_package(OpenGL REQUIRED)

#tinygltf
add_subdirectory(external/tinygltf)

# Collect all source files
file(GLOB_RECURSE SOURCE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Copy files to output folder
file(COPY res DESTINATION ${CMAKE_BINARY_DIR})

add_executable(FPS ${SOURCE_FILES})

target_include_directories(FPS PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(FPS PRIVATE 
    ${SDL2_LIBRARIES} 
    glad 
    OpenGL::GL
    tinygltf
)
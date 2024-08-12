cmake_minimum_required(VERSION 3.28)

project(FPS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable GLM experimental features warning
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

# Check if external dependencies are built
if(NOT TARGET external_dependencies)
    add_subdirectory(external)
endif()

# OpenGL
find_package(OpenGL REQUIRED)

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

target_include_directories(FPS PRIVATE 
    ${CMAKE_SOURCE_DIR}/external/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
)

target_link_libraries(FPS PRIVATE 
    glad 
    SDL2::SDL2main 
    SDL2::SDL2-static
    ${OPENGL_LIBRARIES}
    tinygltf
    glm::glm
)

add_dependencies(FPS external_dependencies)
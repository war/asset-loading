
cmake_minimum_required(VERSION 3.28)
project(FPSExternalDependencies)

# Function to initialize git submodules
function(init_git_submodule SUBMODULE_PATH)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/.git")
        message(STATUS "Initializing git submodule: ${SUBMODULE_PATH}")
        execute_process(
            COMMAND git submodule update --init --recursive -- ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "Failed to initialize git submodule: ${SUBMODULE_PATH}")
        endif()
    endif()
endfunction()

# Initialize submodules
init_git_submodule(SDL2)
init_git_submodule(tinygltf)
init_git_submodule(glm)

# SDL2
if(NOT TARGET SDL2::SDL2-static)
    add_subdirectory(SDL2)
endif()

# GLAD
if(NOT TARGET glad)
    add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c")
    target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
endif()

# tinygltf
if(NOT TARGET tinygltf)
    add_subdirectory(tinygltf)
endif()

# GLM
if(NOT TARGET glm::glm)
    add_subdirectory(glm)
endif()

# Create a custom target that depends on all external libraries
add_custom_target(external_dependencies DEPENDS SDL2::SDL2-static glad tinygltf glm::glm)
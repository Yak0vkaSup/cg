cmake_minimum_required(VERSION 3.16)
project(orbital LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.5
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
    SOURCE_SUBDIR cmake
)

FetchContent_MakeAvailable(SDL2 glad)

glad_add_library(glad_gl_core_33 STATIC API gl:core=3.3)

find_package(OpenGL REQUIRED)

add_executable(orbital
    src/main.cpp
)

target_include_directories(orbital PRIVATE src)
target_link_libraries(orbital PRIVATE
    SDL2-static
    SDL2main
    glad_gl_core_33
    OpenGL::GL
)

if (WIN32)
    set_target_properties(orbital PROPERTIES WIN32_EXECUTABLE FALSE)
endif()

add_custom_command(TARGET orbital POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:orbital>/shaders
)

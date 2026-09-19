cmake_minimum_required(VERSION 3.16)
project(hello LANGUAGES C)

include(FetchContent)

# Disable SDL's OpenGL ES/EGL backend.
set(SDL_OPENGLES OFF CACHE BOOL "Disable OpenGL ES support" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.16
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL3)

add_executable(hello src/hello.c)
target_link_libraries(hello PRIVATE SDL3::SDL3)
cmake_minimum_required(VERSION 3.16)
project(hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Executables and shared libs land in the same directory (set BEFORE SDL is added).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

set(SDL_OPENGLES OFF CACHE BOOL "Disable OpenGL ES support" FORCE)
set(SDL_SHARED ON  CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)   # build bundled FreeType/HarfBuzz

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.16
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3 SDL3_ttf)

add_executable(hello src/hello.cpp)
target_link_libraries(hello PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)

# Loader looks next to the executable, for both build tree and install tree.
if(APPLE)
    set(_origin "@loader_path")
elseif(UNIX)
    set(_origin "$ORIGIN")
endif()
if(_origin)
    set_target_properties(hello PROPERTIES
        BUILD_RPATH   "${_origin}"
        INSTALL_RPATH "${_origin}")
endif()

# Copy assets into the build tree after each build
add_custom_command(TARGET hello POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:hello>/assets)

# Shipping: exe and libraries side by side.
install(TARGETS hello SDL3-shared SDL3_ttf-shared
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin)
install(DIRECTORY assets/ DESTINATION bin/assets)
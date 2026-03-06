cmake_minimum_required(VERSION 3.6)
project(SRR2_Android)

set(GAME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")

# --- SDL2 ---
if(NOT DEFINED SDL2_SRC_DIR)
    set(SDL2_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL2")
endif()

if(NOT EXISTS "${SDL2_SRC_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "SDL2 source not found at ${SDL2_SRC_DIR}.\n"
        "  ln -s /path/to/SDL2-2.30.0 ${CMAKE_CURRENT_SOURCE_DIR}/SDL2")
endif()

set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory("${SDL2_SRC_DIR}" SDL2-build)
set(SDL_LIBRARY SDL2 SDL2main)

# --- OpenAL Soft ---
if(NOT DEFINED OPENAL_SRC_DIR)
    set(OPENAL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openal-soft")
endif()

if(NOT EXISTS "${OPENAL_SRC_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "OpenAL Soft source not found at ${OPENAL_SRC_DIR}.\n"
        "  ln -s /path/to/openal-soft-1.23.1 ${CMAKE_CURRENT_SOURCE_DIR}/openal-soft")
endif()

set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_OPENSL ON CACHE BOOL "" FORCE)
set(LIBTYPE "STATIC" CACHE STRING "" FORCE)
add_subdirectory("${OPENAL_SRC_DIR}" openal-build)
set(OPENAL_LIBRARY OpenAL)
set(OPENAL_INCLUDE_DIR "${OPENAL_SRC_DIR}/include/AL")

# --- libpng ---
if(NOT DEFINED LIBPNG_SRC_DIR)
    set(LIBPNG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libpng")
endif()

if(NOT EXISTS "${LIBPNG_SRC_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "libpng source not found at ${LIBPNG_SRC_DIR}.\n"
        "  ln -s /path/to/libpng-1.6.43 ${CMAKE_CURRENT_SOURCE_DIR}/libpng")
endif()

set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${LIBPNG_SRC_DIR}" libpng-build)

set(PNG_FOUND TRUE)
set(PNG_INCLUDE_DIRS "${LIBPNG_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/libpng-build")
set(PNG_LIBRARIES png_static)
set(PNG_PNG_INCLUDE_DIR "${LIBPNG_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/libpng-build")
set(PNG_LIBRARY png_static)

target_include_directories(png_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libpng-build>
)

if(NOT TARGET PNG::PNG)
    add_library(PNG::PNG ALIAS png_static)
endif()

# --- Build the game ---
add_subdirectory("${GAME_ROOT}" game-build)

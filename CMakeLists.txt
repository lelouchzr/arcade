cmake_minimum_required(VERSION 3.20)
project(Arcade VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
option(ARCADE_BUILD_GRAPHICS "Build all three required display plugins" ON)
set(ARCADE_NDK_ROOT "" CACHE PATH "Prefix of the compatibility-built NDK++ dependencies")
option(ARCADE_REQUIRE_ALL_GRAPHICS "Require every additional display dependency" OFF)
option(ARCADE_EXTRA_GRAPHICS "Build available additional graphical libraries from the subject" ON)
option(ARCADE_BONUS "Build the five additional games and advanced difficulties" ON)
option(ARCADE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ARCADE_BENCHMARKS "Build the reproducible CPU benchmark harness" OFF)
option(ARCADE_STRICT "Treat every compiler warning as an error" ON)
if(ARCADE_REQUIRE_ALL_GRAPHICS AND (NOT ARCADE_BONUS OR NOT ARCADE_BUILD_GRAPHICS OR NOT ARCADE_EXTRA_GRAPHICS))
    message(FATAL_ERROR "ARCADE_REQUIRE_ALL_GRAPHICS needs ARCADE_BONUS, ARCADE_BUILD_GRAPHICS and ARCADE_EXTRA_GRAPHICS")
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
if(ARCADE_STRICT)
    add_compile_options(-Werror)
endif()
if(ARCADE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
include(CTest)
find_package(Threads REQUIRED)
add_library(arcade_sdk INTERFACE)
target_include_directories(arcade_sdk INTERFACE "${PROJECT_SOURCE_DIR}/include")

add_library(arcade_support STATIC
    src/common/Font.cpp src/common/Keys.cpp src/common/SignalState.cpp
    src/loader/SharedLibrary.cpp src/loader/Plugin.cpp
    src/storage/FileLock.cpp src/storage/ScoreFile.cpp src/storage/Scores.cpp
    src/presentation/CellStyle.cpp src/presentation/BoardViewport.cpp)
target_link_libraries(arcade_support PUBLIC arcade_sdk ${CMAKE_DL_LIBS} Threads::Threads)

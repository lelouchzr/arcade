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
add_library(arcade_core STATIC src/core/Core.cpp src/core/CoreInput.cpp src/core/CorePresentation.cpp
    src/core/BoardRenderer.cpp)
target_include_directories(arcade_core PUBLIC src/core)
target_link_libraries(arcade_core PUBLIC arcade_support)
add_executable(arcade src/main.cpp)
target_link_libraries(arcade PRIVATE arcade_core)

function(arcade_plugin target)
    add_library(${target} MODULE ${ARGN})
    target_link_libraries(${target} PRIVATE arcade_support)
    set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES PREFIX "" SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endfunction()

set(snake_sources src/games/snake/Snake.cpp)
set(mine_sources src/games/minesweeper/Minesweeper.cpp)
arcade_plugin(arcade_snake ${snake_sources})
arcade_plugin(arcade_minesweeper ${mine_sources})
set(game_targets arcade_snake arcade_minesweeper)
if(ARCADE_BONUS)
    target_compile_definitions(arcade_minesweeper PRIVATE ARCADE_BONUS)
    target_include_directories(arcade_minesweeper PRIVATE bonus)
endif()

add_custom_target(stage_core ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:arcade> "${PROJECT_SOURCE_DIR}/arcade"
    DEPENDS arcade)
add_custom_target(core DEPENDS stage_core)
add_custom_target(stage_games ALL COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/lib"
    DEPENDS ${game_targets})
foreach(target IN LISTS game_targets)
    add_custom_command(TARGET stage_games POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${target}> "${PROJECT_SOURCE_DIR}/lib/$<TARGET_FILE_NAME:${target}>")
endforeach()
if(NOT ARCADE_BONUS)
    foreach(game nibbler pacman qix centipede solarfox legacy legacy_graphics)
        add_custom_command(TARGET stage_games POST_BUILD COMMAND ${CMAKE_COMMAND} -E rm -f "${PROJECT_SOURCE_DIR}/lib/arcade_${game}.so")
    endforeach()
endif()
add_custom_target(games DEPENDS stage_games)

if(ARCADE_BUILD_GRAPHICS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SFML REQUIRED IMPORTED_TARGET sfml-graphics)
    find_package(Curses REQUIRED)
    arcade_plugin(arcade_sdl2 src/graphics/sdl2/Sdl2.cpp src/graphics/sdl2/Sdl2Events.cpp src/graphics/sdl2/Sdl2Render.cpp)
    target_link_libraries(arcade_sdl2 PRIVATE PkgConfig::SDL2)
    arcade_plugin(arcade_sfml src/graphics/sfml/Sfml.cpp src/graphics/sfml/SfmlEvents.cpp src/graphics/sfml/SfmlRender.cpp)
    target_link_libraries(arcade_sfml PRIVATE PkgConfig::SFML)
    arcade_plugin(arcade_ncurses src/graphics/ncurses/Ncurses.cpp src/graphics/ncurses/NcursesEvents.cpp src/graphics/ncurses/NcursesRender.cpp src/graphics/ncurses/NcursesBoard.cpp)
    target_include_directories(arcade_ncurses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(arcade_ncurses PRIVATE ${CURSES_LIBRARIES})
    add_custom_target(stage_graphics ALL COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/lib"
        DEPENDS arcade_sdl2 arcade_sfml arcade_ncurses)
    foreach(target arcade_sdl2 arcade_sfml arcade_ncurses)
        add_custom_command(TARGET stage_graphics POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${target}> "${PROJECT_SOURCE_DIR}/lib/$<TARGET_FILE_NAME:${target}>")
    endforeach()
    if(TARGET stage_legacy_graphics)
        add_dependencies(stage_graphics stage_legacy_graphics)
    endif()
    if(ARCADE_BONUS AND ARCADE_EXTRA_GRAPHICS)
    endif()
    add_custom_target(graphicals DEPENDS stage_graphics)
endif()


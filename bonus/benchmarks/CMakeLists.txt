add_library(arcade_bench_headless MODULE Headless.cpp)
target_link_libraries(arcade_bench_headless PRIVATE arcade_support)
set_target_properties(arcade_bench_headless PROPERTIES PREFIX "" SUFFIX ".so"
    CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")
add_executable(arcade_benchmark Main.cpp Options.cpp Scenario.cpp Workspace.cpp CoreAccess.cpp Digest.cpp Headless.cpp)
target_compile_definitions(arcade_benchmark PRIVATE ARCADE_NO_ENTRY)
target_link_libraries(arcade_benchmark PRIVATE arcade_core)
set_target_properties(arcade_benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")
add_dependencies(arcade_benchmark arcade_bench_headless arcade_snake arcade_minesweeper arcade_nibbler
    arcade_pacman arcade_qix arcade_centipede arcade_solarfox)
add_executable(arcade_frame_validation FrameVariants.cpp CoreAccess.cpp Digest.cpp Headless.cpp Workspace.cpp)
target_compile_definitions(arcade_frame_validation PRIVATE ARCADE_NO_ENTRY)
target_link_libraries(arcade_frame_validation PRIVATE arcade_core)
set_target_properties(arcade_frame_validation PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")
add_executable(arcade_glyph_probe GlyphProbe.cpp)
target_link_libraries(arcade_glyph_probe PRIVATE arcade_support)
set_target_properties(arcade_glyph_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")

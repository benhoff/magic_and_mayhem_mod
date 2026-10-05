cmake_minimum_required(VERSION 3.16)
project(mnm_effect_projection LANGUAGES CXX)
add_library(mnm-effect-projection STATIC ../effect_projection.cpp ../effect_trajectory.cpp
    ../effect_placement.cpp ../effect_lighting.cpp)
target_compile_features(mnm-effect-projection PUBLIC cxx_std_17)
target_include_directories(mnm-effect-projection PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_options(mnm-effect-projection PRIVATE -Wall -Wextra -Werror -pedantic)
include(CTest)
if(BUILD_TESTING)
    add_executable(effect-projection-test ../../../tests/effect-projection-test.cpp
        ../terrain_lighting_cycle.cpp ../terrain_static_lighting.cpp
        ../terrain_creature_lighting.cpp ../terrain_lighting.cpp)
    target_link_libraries(effect-projection-test PRIVATE mnm-effect-projection)
    target_compile_options(effect-projection-test PRIVATE -Wall -Wextra -Werror -pedantic)
    add_test(NAME effect-same-cell-projection COMMAND effect-projection-test)
endif()

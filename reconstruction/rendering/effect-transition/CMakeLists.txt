cmake_minimum_required(VERSION 3.16)
project(mnm_effect_transition LANGUAGES CXX)
add_library(mnm-effect-transition STATIC ../effect_transition.cpp ../effect_projection.cpp
    ../effect_trajectory.cpp ../effect_placement.cpp ../effect_lighting.cpp)
target_compile_features(mnm-effect-transition PUBLIC cxx_std_17)
target_include_directories(mnm-effect-transition PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_options(mnm-effect-transition PRIVATE -Wall -Wextra -Werror -pedantic)
include(CTest)
if(BUILD_TESTING)
    add_executable(effect-transition-test ../../../tests/effect-transition-test.cpp
        ../terrain_lighting_cycle.cpp ../terrain_static_lighting.cpp
        ../terrain_creature_lighting.cpp ../terrain_lighting.cpp)
    target_link_libraries(effect-transition-test PRIVATE mnm-effect-transition)
    target_compile_options(effect-transition-test PRIVATE -Wall -Wextra -Werror -pedantic)
    add_test(NAME effect-empty-world-transition COMMAND effect-transition-test)
endif()

cmake_minimum_required(VERSION 3.16)
project(mnm_original_creature_execution LANGUAGES CXX)
if(NOT TARGET mnm-original-creature-ai)
    add_subdirectory(.. controllers)
endif()
add_library(mnm-original-creature-execution STATIC
    ../execution.cpp ../route_execution.cpp ../orders.cpp ../periodic.cpp
    ../ranged_execution.cpp ../action_execution.cpp ../behavior_execution.cpp ../strategy.cpp
    ../maintenance.cpp ../movement_behavior.cpp ../death_behavior.cpp ../action_callbacks.cpp)
target_compile_features(mnm-original-creature-execution PUBLIC cxx_std_17)
target_include_directories(mnm-original-creature-execution PUBLIC ..)
target_compile_options(mnm-original-creature-execution PRIVATE -Wall -Wextra -Werror -pedantic)
target_link_libraries(mnm-original-creature-execution PUBLIC mnm-original-creature-ai)
include(CTest)
if(BUILD_TESTING)
    foreach(suite IN ITEMS test actions-test orders-test strategy-test behaviors-test lifecycle-test callbacks-test)
        add_executable(creature-execution-${suite}
            ../../../tests/creature-execution-${suite}.cpp)
        target_link_libraries(creature-execution-${suite} PRIVATE mnm-original-creature-execution)
        target_compile_options(creature-execution-${suite} PRIVATE -Wall -Wextra -Werror -pedantic)
        add_test(NAME creature-execution-${suite} COMMAND creature-execution-${suite})
    endforeach()
endif()

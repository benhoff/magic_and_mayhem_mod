cmake_minimum_required(VERSION 3.16)
project(mnm_pathfinding_reconstruction LANGUAGES CXX)

add_library(mnm_pathfinding STATIC route_request.cpp route_search.cpp route_neighbors.cpp
    route_creature_acceptance.cpp route_movement.cpp route_cell.cpp route_occupancy.cpp route_validity.cpp route_cell_validity.cpp route_record_query.cpp route_cell_support.cpp route_boundary.cpp route_clearance.cpp route_scalar.cpp route_world.cpp)
target_include_directories(mnm_pathfinding PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(mnm_pathfinding PUBLIC cxx_std_17)
set_target_properties(mnm_pathfinding PROPERTIES CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mnm_pathfinding PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

add_executable(route-replay route_replay.cpp)
target_link_libraries(route-replay PRIVATE mnm_pathfinding)

include(CTest)
if(BUILD_TESTING)
    foreach(component IN ITEMS route-request route-search route-neighbors route-creature-acceptance route-movement route-cell route-occupancy route-validity route-cell-validity route-record-query route-cell-support route-boundary route-clearance route-scalar route-world)
        add_executable(${component}-test "../../tests/${component}-test.cpp")
        target_link_libraries(${component}-test PRIVATE mnm_pathfinding)
        # Assertions are the checks even in a Release build.
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${component}-test PRIVATE -UNDEBUG)
        endif()
        add_test(NAME ${component} COMMAND ${component}-test)
    endforeach()
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME route-world-tooling COMMAND "${CMAKE_COMMAND}" -E env
            "MNM_ROUTE_REPLAY=$<TARGET_FILE:route-replay>"
            "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/test-route-world.py")
        add_test(NAME search-sequence-tooling COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/test-search-sequence.py")
    endif()
endif()

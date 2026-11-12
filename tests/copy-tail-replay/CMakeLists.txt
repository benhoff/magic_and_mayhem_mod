cmake_minimum_required(VERSION 3.16)
project(copy_tail_replay LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(REPO "${CMAKE_CURRENT_SOURCE_DIR}/../..")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/observer.py"
    "${REPO}" "${CMAKE_CURRENT_BINARY_DIR}/offline-observer.cpp" COMMAND_ERROR_IS_FATAL ANY)
add_subdirectory("${REPO}/compat/legacy/world-rolling" "${CMAKE_CURRENT_BINARY_DIR}/world" EXCLUDE_FROM_ALL)
# Supplying the complete observer object first keeps the production observer
# object out of the static archive extraction. The map is verified by the runner.
add_executable(copy-tail-replay main.cpp "${CMAKE_CURRENT_BINARY_DIR}/offline-observer.cpp")
target_include_directories(copy-tail-replay PRIVATE "${REPO}/renderer")
target_link_libraries(copy-tail-replay PRIVATE mnm-world-rolling)
target_link_options(copy-tail-replay PRIVATE "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/replay.map")
target_compile_options(copy-tail-replay PRIVATE -Wall -Wextra -Werror -pedantic)

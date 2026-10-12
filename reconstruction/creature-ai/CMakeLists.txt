cmake_minimum_required(VERSION 3.16)
project(mnm_original_creature_ai LANGUAGES CXX)
add_library(mnm-original-creature-ai STATIC
    ../combat/autonomy.cpp ../combat/ai.cpp ../combat/control.cpp)
target_compile_features(mnm-original-creature-ai PUBLIC cxx_std_17)
target_include_directories(mnm-original-creature-ai PUBLIC ../combat)
target_compile_options(mnm-original-creature-ai PRIVATE -Wall -Wextra -Werror -pedantic)

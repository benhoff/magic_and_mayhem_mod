cmake_minimum_required(VERSION 3.16)
project(mnm_active_reader_gl LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
add_subdirectory(../../renderer renderer)
add_executable(native-active-reader-gl ../native-active-reader-gl.cpp
    ../../apps/qt-shell/live_command_renderer.cpp
    ../../apps/qt-shell/command_channel.cpp
    ../../apps/qt-shell/gl_viewport.cpp)
target_include_directories(native-active-reader-gl PRIVATE ../../apps/qt-shell)
target_link_libraries(native-active-reader-gl PRIVATE mnm-renderer Qt6::Widgets Qt6::OpenGLWidgets)
target_compile_options(native-active-reader-gl PRIVATE -Wall -Wextra -Werror -pedantic)

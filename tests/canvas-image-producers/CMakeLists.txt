cmake_minimum_required(VERSION 3.16)
project(mnm_canvas_image_producers LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Gui)
add_subdirectory(../../assets "${CMAKE_CURRENT_BINARY_DIR}/assets")
add_executable(canvas-image-producer-test ../canvas-image-producer-test.cpp
  ../../compat/legacy/canvas_producers.cpp ../../renderer/canvas_sequence.cpp ../../renderer/dib.cpp
  ../../renderer/minimap/minimap.cpp ../../renderer/minimap/overlays.cpp)
target_link_libraries(canvas-image-producer-test PRIVATE mnm-sprite-loader mnm-jpeg-loader mnm-pcx-loader Qt6::Gui)
target_compile_options(canvas-image-producer-test PRIVATE -Wall -Wextra -Werror -pedantic)

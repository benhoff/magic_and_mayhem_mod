cmake_minimum_required(VERSION 3.16)
project(mnm_native_playback_recovery LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Gui Multimedia)
add_executable(native-playback-recovery-test ../native-playback-recovery-test.cpp
    ../../apps/qt-shell/native_playback.cpp ../../apps/qt-shell/playback_audio_devices.cpp)
target_link_libraries(native-playback-recovery-test PRIVATE Qt6::Gui Qt6::Multimedia)
target_compile_options(native-playback-recovery-test PRIVATE -Wall -Wextra -Werror -pedantic)

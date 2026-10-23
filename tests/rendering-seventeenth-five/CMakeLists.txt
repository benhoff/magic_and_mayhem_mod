cmake_minimum_required(VERSION 3.16)
project(mnm_seventeenth_rendering_controls LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(../../apps/qt-shell qt-shell)
foreach(control render-resident-world-test world-deferred-test world-source-regions-test
    world-raster-batch-test minimap-producer-test minimap-candidates-test
    menu-resident-quit-test menu-campaign-preferences-test scene-renderer-test
    scene-upload-test native-only-session-test game-hard-restart-test native-frame-recipe-test world-gpu-resident-test live-render-channel-test)
  if(TARGET ${control})
    target_compile_options(${control} PRIVATE -UNDEBUG)
  endif()
endforeach()

add_executable(native-direct-store-test ../native-direct-store-test.c)
add_executable(pixel-store-backend-test ../pixel-store-backend-test.c
 ../../reconstruction/rendering/pixel_kernel.c
 ../../reconstruction/rendering/minimap_pixels.c
 ../../reconstruction/rendering/raster_pixels.c)
target_compile_options(native-direct-store-test PRIVATE -UNDEBUG)
target_compile_options(pixel-store-backend-test PRIVATE -UNDEBUG)

add_executable(native-sampling-test ../native-sampling-test.c)
target_compile_options(native-sampling-test PRIVATE -UNDEBUG)

add_executable(native-lease-row-copy-test ../native-lease-row-copy-test.c)
target_compile_options(native-lease-row-copy-test PRIVATE -UNDEBUG)

add_executable(native-dirty-byte-test ../native-dirty-byte-test.c)
target_compile_options(native-dirty-byte-test PRIVATE -UNDEBUG)

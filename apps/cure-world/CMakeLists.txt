cmake_minimum_required(VERSION 3.16)
project(mnm_cure_world LANGUAGES CXX)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core)
add_library(mnm-cure-world STATIC session.cpp ../cure-lab/session.cpp
  ../../game/simulation/world.cpp ../../game/simulation/movement.cpp ../../game/simulation/occupancy.cpp
  ../../game/persistence/snapshot.cpp ../../game/persistence/store_posix.cpp
  ../../reconstruction/casting/admission.cpp ../../reconstruction/casting/cure_effect.cpp
  ../../reconstruction/casting/cure_services.cpp ../../reconstruction/casting/cure_presentation.cpp
  ../../reconstruction/combat/health.cpp ../../reconstruction/audio/voice_contract.cpp
  ../../reconstruction/audio/voice_lifetime.cpp ../../reconstruction/animation/no_cd.cpp
  ../../reconstruction/rendering/effect_animation_binding.cpp ../../reconstruction/rendering/effect_animation_selection.cpp)
target_include_directories(mnm-cure-world PUBLIC ../../game ../../audio ../../assets ../../reconstruction/animation)
target_compile_features(mnm-cure-world PUBLIC cxx_std_17)
target_compile_options(mnm-cure-world PRIVATE -Wall -Wextra -Werror -pedantic)
# Keep the existing GCC16 optional-copy diagnostic visible for the unchanged
# movement dependency, without relaxing strict warnings on adapter sources.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_source_files_properties(../../game/simulation/movement.cpp PROPERTIES COMPILE_OPTIONS -Wno-error=maybe-uninitialized)
endif()
add_library(mnm-cure-world-assets STATIC installed.cpp
  ../../assets/asset_file.cpp ../../assets/path_resolver.cpp ../../assets/packed_container.cpp
  ../../assets/config_loader.cpp ../../assets/animation.cpp ../../assets/animation_decode.cpp
  ../../assets/effect_animation_catalog.cpp ../../reconstruction/rendering/effect_animation_catalog_binding.cpp)
target_link_libraries(mnm-cure-world-assets PUBLIC mnm-cure-world PRIVATE Qt6::Core)
target_compile_options(mnm-cure-world-assets PRIVATE -Wall -Wextra -Werror -pedantic)
add_executable(cure-world main.cpp)
target_link_libraries(cure-world PRIVATE mnm-cure-world-assets)
target_compile_options(cure-world PRIVATE -Wall -Wextra -Werror -pedantic)
include(CTest)
if(BUILD_TESTING)
  add_executable(cure-world-test ../../tests/cure-world-test.cpp)
  target_link_libraries(cure-world-test PRIVATE mnm-cure-world)
  target_compile_options(cure-world-test PRIVATE -Wall -Wextra -Werror -pedantic)
  add_test(NAME cure-world COMMAND cure-world-test)
endif()

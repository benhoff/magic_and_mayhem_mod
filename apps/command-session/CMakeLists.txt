cmake_minimum_required(VERSION 3.16)
project(mnm_native_command_session LANGUAGES CXX)
add_subdirectory(../../game native-game)
add_subdirectory(../../reconstruction/creature-ai/transfer-commands-owned command-core)
add_library(mnm-native-command-session STATIC runtime.cpp codec.cpp
  native_providers.cpp native_geometry.cpp
  ../../reconstruction/creature-ai/attack_selection.cpp
  ../../reconstruction/creature-ai/idle_gesture.cpp
  ../../reconstruction/creature-ai/coarse_route.cpp
  ../../reconstruction/creature-ai/motion_response.cpp
  ../../reconstruction/rendering/effect_trajectory.cpp
  ../../reconstruction/pathfinding/route_footprint_height.cpp
  ../../reconstruction/animation/controller.cpp
  ../../reconstruction/pathfinding/route_line.cpp
  ../../reconstruction/creature-ai/maintenance.cpp
  ../../reconstruction/creature-ai/segment_completion.cpp
  ../../reconstruction/creature-ai/full_segment_setup.cpp
  ../../reconstruction/creature-ai/environment_pass.cpp
  ../../reconstruction/creature-ai/periodic.cpp
  ../../reconstruction/creature-ai/route_execution.cpp
  ../../reconstruction/creature-ai/death_behavior.cpp
  ../../reconstruction/combat/autonomy.cpp
  ../../reconstruction/combat/ai.cpp
  ../../reconstruction/combat/melee.cpp
  ../../reconstruction/motion/creature_profile.cpp
  ../../reconstruction/creature-ai/command_action_bodies.cpp
  ../../reconstruction/creature-ai/action_callbacks.cpp
  ../../reconstruction/creature-ai/full_motion_action.cpp
  ../../reconstruction/creature-ai/special_flight_actions.cpp
  ../../reconstruction/creature-ai/object_action.cpp
  ../../reconstruction/creature-ai/command_actions_owned.cpp
  ../../reconstruction/creature-ai/action_execution.cpp
  ../../reconstruction/creature-ai/command_behaviors_owned.cpp
  ../../reconstruction/creature-ai/movement_behavior.cpp
  ../../reconstruction/creature-ai/ranged_execution.cpp)
target_compile_features(mnm-native-command-session PUBLIC cxx_std_17)
target_link_libraries(mnm-native-command-session PUBLIC mnm-creature-rules-session mnm-creature-transfer-commands-owned mnm_pathfinding mnm-animation-model mnm-sandbox-animation-decoder)
target_compile_options(mnm-native-command-session PRIVATE -Wall -Wextra -Werror -pedantic)
add_executable(native-command-session-test ../../tests/native-command-session-test.cpp)
target_link_libraries(native-command-session-test PRIVATE mnm-native-command-session)
target_compile_options(native-command-session-test PRIVATE -Wall -Wextra -Werror -pedantic)
enable_testing()
add_test(NAME native-command-session COMMAND native-command-session-test)
add_executable(native-command-behavior-session-test ../../tests/native-command-behavior-session-test.cpp)
target_link_libraries(native-command-behavior-session-test PRIVATE mnm-native-command-session)
target_compile_options(native-command-behavior-session-test PRIVATE -Wall -Wextra -Werror -pedantic)
add_test(NAME native-command-behavior-session COMMAND native-command-behavior-session-test)

add_executable(native-command-action-session-test ../../tests/native-command-action-session-test.cpp)
target_link_libraries(native-command-action-session-test PRIVATE mnm-native-command-session)
target_compile_options(native-command-action-session-test PRIVATE -Wall -Wextra -Werror -pedantic)
add_test(NAME native-command-action-session COMMAND native-command-action-session-test)

add_executable(native-command-action-bodies-session-test ../../tests/native-command-action-bodies-session-test.cpp)
target_link_libraries(native-command-action-bodies-session-test PRIVATE mnm-native-command-session)
target_compile_options(native-command-action-bodies-session-test PRIVATE -Wall -Wextra -Werror -pedantic)
add_test(NAME native-command-action-bodies-session COMMAND native-command-action-bodies-session-test)

add_executable(native-creature-providers-test ../../tests/native-creature-providers-test.cpp)
target_link_libraries(native-creature-providers-test PRIVATE mnm-native-command-session)
target_compile_options(native-creature-providers-test PRIVATE -Wall -Wextra -Werror -pedantic)
set(MNM_NATIVE_PROVIDER_ANI "" CACHE FILEPATH "Optional installed creature ANI for provider tests")
add_test(NAME native-creature-providers COMMAND native-creature-providers-test "${MNM_NATIVE_PROVIDER_ANI}")

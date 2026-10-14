cmake_minimum_required(VERSION 3.16)
project(mnm_native_command_session LANGUAGES CXX)
add_subdirectory(../../game native-game)
add_subdirectory(../../reconstruction/creature-ai/transfer-commands-owned command-core)
add_library(mnm-native-command-session STATIC runtime.cpp codec.cpp
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
target_link_libraries(mnm-native-command-session PUBLIC mnm-creature-rules-session mnm-creature-transfer-commands-owned)
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

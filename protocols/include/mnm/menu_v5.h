#pragma once
#include "menu_v4.h"
#define MNM_MENU_V5_MAGIC "MNMMCMD5"
#define MNM_MENU_V5_SIZE 73728
#define MNM_MENU_V5_RESULTS 42100
#define MNM_MENU_V5_RESULTS_SIZE 2608
#define MNM_MENU_RESULT_SCREEN 26
#define MNM_MENU_RESULT_CONTINUE 16
#define MNM_MENU_RESULT_QUIT 17
/* LE header: offered actions (Continue=1, Quit=2), original context,
 * stack depth, reserved zero. Four display rows follow, each 648 bytes:
 * active boolean, portrait index, five CP1252 NUL-terminated 128-byte strings
 * (name/kills/deaths/handicap/score). No scores are calculated by the host.
 * V1-V4 lanes and payloads retain their offsets. Mini remains separately gated.
 */

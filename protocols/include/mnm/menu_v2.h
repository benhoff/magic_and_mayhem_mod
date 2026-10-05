#pragma once
#include "menu_v1.h"
/* V2 is a separate, bounded LE wire contract. IDs 1..3 retain V1 semantics.
 * Host lane: seq/alive/heartbeat/id/action/generation/argument/17 rule values.
 * Engine lane: seq/generation/screen/ready/ack/status/thread. Payload uses
 * byte offsets below; names are NUL-terminated Windows-1252, never pointers.
 */
#define MNM_MENU_V2_MAGIC "MNMMCMD2"
#define MNM_MENU_V2_VERSION 2
#define MNM_MENU_V2_SIZE 32768
#define MNM_MENU_V2_ENGINE_WORD 32
#define MNM_MENU_V2_MAP 160
#define MNM_MENU_V2_MAP_COUNT 164
#define MNM_MENU_V2_RULES 168
#define MNM_MENU_V2_PLAYERS 220
#define MNM_MENU_V2_PLAYER_SIZE 48
#define MNM_MENU_V2_MAP_NAME 412
#define MNM_MENU_V2_MAP_NAMES 540
#define MNM_MENU_V2_NAME_SIZE 128
#define MNM_MENU_V2_MAX_MAPS 128
#define MNM_MENU_V2_PAYLOAD_SIZE (MNM_MENU_V2_MAP_NAMES+MNM_MENU_V2_MAX_MAPS*MNM_MENU_V2_NAME_SIZE-MNM_MENU_V2_MAP)
#define MNM_MENU_OPEN_SINGLE 4
#define MNM_MENU_SETUP_CANCEL 5
#define MNM_MENU_SETUP_MAP 6
#define MNM_MENU_SETUP_START 7
#define MNM_MENU_SETUP_PLAYER 8
#define MNM_MENU_MAP_OK 9
#define MNM_MENU_MAP_CANCEL 10
#define MNM_MENU_SETUP_APPLY 11
#define MNM_MENU_INVALID 6

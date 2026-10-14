#pragma once
#include "menu_v12.h"
#define MNM_MENU_V13_MAGIC "MNMMCM13"
#define MNM_MENU_V13_SIZE 131072
#define MNM_MENU_V13_HOST_NAME 106496
#define MNM_MENU_V13_NAME_SIZE 128
#define MNM_MENU_V13_STORAGE 110592
#define MNM_MENU_V13_MAX_SAVES 100
#define MNM_MENU_V13_STORAGE_SIZE (32+128+100*128)
#define MNM_MENU_LOAD_SCREEN 12
#define MNM_MENU_SAVE_SCREEN 13
#define MNM_MENU_OPEN_LOAD 27
#define MNM_MENU_MINI_LOAD 28
#define MNM_MENU_MINI_SAVE 29
#define MNM_MENU_STORAGE_CANCEL 30
#define MNM_MENU_STORAGE_ACCEPT 31
#define MNM_MENU_STORAGE_DELETE 32
#define MNM_MENU_MINI_CAN_LOAD 8
#define MNM_MENU_MINI_CAN_SAVE 16
/* Existing V12 lanes retained. Host name is 128-byte NUL-terminated ASCII for
 * new Save names only; Load/Delete use argument = original row index + 1.
 * Storage LE words: screen, parent screen, depth, context, modal, pending,
 * row count, selected row + 1 (0 means absent). Original edit text and 100
 * original row stems follow, each NUL-terminated 128-byte CP1252.
 * No host pointers, filesystem paths, serializer state or overwrite flags.
 * Original overwrite/error modals and serializer remain authoritative.
 */

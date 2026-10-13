#pragma once
#include "menu_v21.h"
#define MNM_MENU_V22_MAGIC "MNMMCM22"
#define MNM_MENU_V22_SIZE 524288
#define MNM_MENU_V22_GRIMOIRE 458752
#define MNM_MENU_V22_GRIMOIRE_SIZE 4096
#define MNM_MENU_V22_BOOK_ENTRIES 992
/* Experimental static campaign book policy. V14 state: chapter0..7,section0..123,
 * original cursor1,available=0,parent4/18,depth4/5,context5,reserved0.
 * New lane: wizard0..79,selected effective level0..4,available count0..992,
 * five reserved zero words;992 LE effective levels indexed chapter*124+section,
 * UINT32_MAX unavailable. Remaining96 bytes zero. Knowledge comes from original
 * first active primary wizard and its nullable 1240-byte table; override0/1.
 * Entry title,level fallback and bounded original text authorize static single
 * pages only. Dynamic directives,multiple pages and unknown chapters stay with
 * original controls. Select is checked packed chapter*124+section;Close3999.
 */

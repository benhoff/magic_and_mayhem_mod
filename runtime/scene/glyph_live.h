#pragma once
#include "../shadow/win32_min.h"
/* 0 disabled, 1 shadow, 2 takeover, -1 invalid configuration. */
int glyph_live_configure(void);
int glyph_live_enter(u32*,u8*);
void glyph_live_leave(u32*,u8*);
void glyph_live_close(void);
void glyph_live_installed(void);

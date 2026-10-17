#pragma once
/* Observer metadata only: process-local numerical IDs, never host pointers. */
#define MNM_STORAGE_TRANSITIONS_MAGIC "MNMCST02"
#define MNM_STORAGE_TRANSITIONS_VERSION 5u
#define MNM_STORAGE_TRANSITIONS_ROW_BYTES 96u
#define MNM_STORAGE_TRANSITIONS_MAX_ROWS 131072u
#define MNM_STORAGE_TRANSITIONS_MAX_ADDRESSES 512u
/* V5 retains V4 lifecycle/candidate/terminal rows. Query phase3 word7 is the
 * queried input address ID (zero for null), in the same namespace as observed
 * wrapper Lock returns and cached/interface addresses. V1..V4 require zero.
 * Known query word5 must equal word7; unknown/pending queries may retain an
 * input ID without inventing a canvas/epoch. Numerical equality and event
 * order do not establish allocation generation, byte or lifetime ownership. */

#ifndef MNM_DRAWING_INPUT_POINT_V1_H
#define MNM_DRAWING_INPUT_POINT_V1_H
/* 64 little-endian bytes, no process pointers. Flags:1 active,2 cursor valid.
 * DWORDs: version,bytes,build,flags,owner,record-prefix,entry-prefix,planes,
 * drops,gaps,pending,last-cursor-record,last-cursor-entry,tick,x,y. */
#define MNM_DRAWING_INPUT_POINT_V1_BYTES 64u
#define MNM_DRAWING_INPUT_POINT_V1_VERSION 1u
#endif

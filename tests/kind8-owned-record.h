#ifndef MNM_KIND8_OWNED_RECORD_H
#define MNM_KIND8_OWNED_RECORD_H
struct Kind8OwnedRecord {
    u32 header[16];
    u8 constructed[128],nondeleted[128];
    u16 plane[768];
    u32 frees[8],random[252];
};
#endif

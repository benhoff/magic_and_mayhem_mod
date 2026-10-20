#ifndef MNM_KIND8_CLASS_H
#define MNM_KIND8_CLASS_H
#include <stdint.h>
/* Pinned No-CD identities only. Never dereference these values natively. */
enum MnmKind8Method {MNM_K8_UNKNOWN=0,MNM_K8_ADDITIVE,MNM_K8_INHERITED,
    MNM_K8_OWNER,MNM_K8_COLOUR,MNM_K8_FLAG_ONLY,MNM_K8_EMPTY};
struct MnmKind8Class {uint32_t method,shape_offset,readable_bytes;};
static inline struct MnmKind8Class mnm_kind8_class(uint32_t table,uint32_t target){
    struct MnmKind8Class result={MNM_K8_UNKNOWN,0,0};
    if(table==0x5c7420&&target==0x54ab00){result.method=MNM_K8_ADDITIVE;result.shape_offset=0x30;result.readable_bytes=72;}
    else if(table==0x5c73f0&&target==0x54aaa0){result.method=MNM_K8_INHERITED;result.shape_offset=0x34;result.readable_bytes=72;}
    else if(table==0x5c7408&&target==0x546540){result.method=MNM_K8_OWNER;result.shape_offset=0x34;result.readable_bytes=100;}
    else if((table==0x5c7494||table==0x5c74b0)&&target==0x54ab60){result.method=MNM_K8_COLOUR;result.shape_offset=0x1c;result.readable_bytes=72;}
    else if(table==0x5c74cc&&target==0x54ac00){result.method=MNM_K8_FLAG_ONLY;result.readable_bytes=72;}
    else if((table==0x5c7438&&target==0x54ac30)||((table==0x5c7450||table==0x5c7470)&&target==0x54ac40)){
        result.method=MNM_K8_EMPTY;result.readable_bytes=4;
    }
    return result;
}
#endif

#ifndef MNM_WORLD_KINDS_H
#define MNM_WORLD_KINDS_H
/* Observer admission, not a claim that the original dispatcher lacks others. */
static int world_kind_admitted(u32 kind){
    return kind==0xfffffffe||kind==0||kind==1||kind==2||kind==3||kind==4||kind==5||kind==6||kind==22||kind==31||kind==33;
}
#endif

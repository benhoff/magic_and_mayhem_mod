/* No-CD campaign shortcut adapter. UI and original simulation stay active. */
#include "../shadow/win32_min.h"
#include "../../game/input/rts_controls.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
#define THIS __attribute__((thiscall))
typedef u32 (THIS *KeyFn)(void*,u32,u32,u32,u32);
typedef u32 (THIS *MouseFn)(void*,u32,u32,u32,u32,u32,u32,u32,u32,u32,u32,u32);
typedef void (THIS *ActorFn)(void*);
typedef u32 (THIS *ResetFn)(void*,u32);
typedef short (WIN *KeyStateFn)(int);
static MnmRtsState policy;
static MnmRtsState candidate_policy;
static MnmRtsSnapshot snapshot;
static MnmRtsResult result;
static u32 generations[256],session=1,owner_thread,last_pool,last_tick,enabled;
static ResetFn original_reset;
static u32 camera_saved[4],camera_positions[4][6],camera_sessions[4];
#include "settings.h"
static u32 get(const void* value) {
    const u8* p=value; return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;
}
static void put(void* value,u32 n) {
    u8* p=value; p[0]=n;p[1]=n>>8;p[2]=n>>16;p[3]=n>>24;
}
static void copy(void* destination,const void* source,u32 n) {
    u8* out=destination;const u8* in=source;
    for(u32 i=0;i<n;++i)out[i]=in[i];
}
static int readable(const void* p,u32 n) {
    struct {void* base;void* allocation;u32 protection,size,state,protect,type;} info;
    u32 at=(u32)p,end=at+n;
    if(!at||end<at)return 0;
    while(at<end) {
        if(!VirtualQuery((void*)at,&info,sizeof(info))||info.state!=0x1000||
            (info.protect&0x101))return 0;
        u32 next=(u32)info.base+info.size;if(next<=at)return 0;at=next;
    }return 1;
}
static int same(u32 at,const u8* bytes,u32 n) {
    if(!readable((void*)at,n))return 0;
    for(u32 i=0;i<n;++i)if(*(u8*)(at+i)!=bytes[i])return 0;
    return 1;
}
static void new_session(void) {
    if(!++session)session=1;
    mnm_rts_reset(&policy);
    for(u32 i=0;i<4;++i)camera_saved[i]=0;
}
static int world(void) {
    return enabled&&readable((void*)0x6cbb78,0x1031b)&&
        get((void*)0x6f34e0)==0x6cbb78&&get((void*)0x6cbb7c)==2&&
        get((void*)0x689920)==5&&!get((void*)0x68991c)&&
        !get((void*)0x6c4634)&&!*(u8*)0x6c4859;
}
static int owned_thread(void) {
    u32 tid=GetCurrentThreadId();
    __sync_bool_compare_and_swap(&owner_thread,0,tid);
    return owner_thread==tid;
}
static int capture(void) {
    u32 capacity=get((void*)0x6def5c),count=get((void*)0x6df180);
    u32 pool=get((void*)0x6def58),owner=get((void*)0x644520),tick=get((void*)0x6c4830);
    if(!capacity||capacity>256||count>capacity||owner>=80||
        !readable((void*)pool,capacity*0xe4b))return 0;
    if(pool!=last_pool||tick<last_tick){new_session();last_pool=pool;}
    last_tick=tick;snapshot.count=count;snapshot.owner=owner;snapshot.session=session;
    for(u32 i=0;i<count;++i) {
        u8* p=(void*)(pool+i*0xe4b);
        if(get(p)!=i)return 0;
        if(!generations[i])generations[i]=1;
        MnmRtsActor* a=snapshot.actors+i;
        a->slot=i;a->generation=generations[i];a->active=get(p+4);
        a->health=(int)get(p+0xe4);a->type=get(p+0xa8);a->owner=get(p+0x174);
        if(a->active&&a->type>27)return 0;
        if(a->type==22||a->type==27)a->health=0; /* Original primary selection excludes these. */
        a->selected=get(p+0x68);a->group=a->active?get(p+0x8e9):UINT32_MAX;
        a->wizard=a->type==0;
        a->idle=get(p+0x5ec)==1&&get(p+0x906)==UINT32_MAX&&
            !p[0x901]&&!get(p+0x698)&&!get(p+0x618)&&!get(p+0x664);
        if(a->active&&(a->group>9&&a->group!=UINT32_MAX))return 0;
    }return 1;
}
/* Selection helper dependencies must still belong to this live actor. */
static int selection_ready(u8* p) {
    u8* list=(void*)get(p+0x8f9);u8* profile=(void*)get(p+0xac);
    if(!readable(profile,4)||!readable(list,12)||get(list+8)>16)return 0;
    u8* sentinel=(void*)get(list+4);
    if(!readable(sentinel,12))return 0;
    u8* node=(void*)get(sentinel);u32 seen=0;
    while(node!=sentinel) {
        if(++seen>16||!readable(node,44))return 0;
        node=(void*)get(node);
    }return 1;
}
static int camera_ready(void) {
    u8* map=(void*)get((void*)0x689955);
    return readable(map,16)&&get(map+4)>0&&get(map+4)<=256&&
        get(map+8)>0&&get(map+8)<=256&&get(map+12)>0&&get(map+12)<=256&&
        get((void*)0x6c5494)>0&&get((void*)0x6c5498)>0;
}
static int apply(u32 action,u32 argument,int additive,u32 now,int repeat) {
    copy(&candidate_policy,&policy,sizeof(policy));
    if(!capture()||!mnm_rts_apply(&candidate_policy,&snapshot,action,argument,additive,now,
                                RTS_DOUBLE_TAP_MS,repeat,&result))return 0;
    if(!result.accepted)return 1;
    u32 pool=get((void*)0x6def58);
    /* Preflight the complete operation before invoking any original helper. */
    for(u32 i=0;i<snapshot.count;++i) {
        u8* p=(void*)(pool+i*0xe4b);
        if(result.selection_write&&
           (get(p+0x68)!=result.selection[i]||get(p+0x70))&&!selection_ready(p))return 0;
    }
    if(result.focus_slot!=UINT32_MAX&&!camera_ready())return 0;
    if(result.selection_write) {
        for(u32 i=0;i<snapshot.count;++i) {
            u8* p=(void*)(pool+i*0xe4b);
            if((get(p+0x68)||get(p+0x70))&&!result.selection[i])((ActorFn)0x5074f0)(p);
        }
        for(u32 i=0;i<snapshot.count;++i) {
            u8* p=(void*)(pool+i*0xe4b);
            if(result.selection[i]&&!get(p+0x68))((ActorFn)0x5073d0)(p);
        }
        put((void*)0x6df17c,1);
    }
    for(u32 i=0;i<snapshot.count;++i)if(result.group_write[i])
        put((void*)(pool+i*0xe4b+0x8e9),result.groups[i]);
    if(result.focus_slot!=UINT32_MAX) {
        typedef void (THIS *FocusFn)(void*,void*);
        ((FocusFn)0x4f7ae0)((void*)0x689930,(void*)(pool+result.focus_slot*0xe4b));
    }
    copy(&policy,&candidate_policy,sizeof(policy));
    return 1;
}
/* Stop is an ordinary move to the actor's current cell through the existing
 * command producer. Original delivery clears queued orders/follow and retains
 * movement admission and simulation scheduling. No new stance or AI rule. */
static int stop_selection(void) {
    if(!capture())return 0;
    u32 pool=get((void*)0x6def58);
    u32 selected=0;
    for(u32 i=0;i<snapshot.count;++i) {
        MnmRtsActor* a=snapshot.actors+i;
        if(!a->active||a->health<=0||a->owner!=snapshot.owner||!a->selected)continue;
        ++selected;
        u8* p=(void*)(pool+i*0xe4b);
        for(u32 k=0;k<3;++k)if(get(p+8+4*k)>65535)return 0;
    }
    if(!selected)return 1;
    u8* manager=(void*)0x6a2898;
    u32 capacity=get(manager+0x28),cursor=get(manager+0x30),base=get(manager+0x24);
    if(!get(manager+0x34)||!capacity||capacity>65536||cursor>capacity)return 0;
    u32 index=(cursor?cursor:capacity)-1;
    if(index>(UINT32_MAX-base)/21)return 0;
    u8* node=(void*)(base+index*21);
    if(!readable(node,21)||get(node+12)>get(node+4)||
       get(node+4)-get(node+12)<selected*21||
       !readable((void*)get(node+8),selected*21)||!readable((void*)get(node),20))return 0;
    typedef void (THIS *SubmitFn)(void*,const void*);
    for(u32 i=0;i<snapshot.count;++i) {
        MnmRtsActor* a=snapshot.actors+i;
        if(!a->active||a->health<=0||a->owner!=snapshot.owner||!a->selected)continue;
        u8* p=(void*)(pool+i*0xe4b);u8 command[21]={0};
        put(command+4,5);put(command+8,sizeof(command));command[12]=(u8)i;
        for(u32 k=0;k<3;++k) {
            u32 n=get(p+8+4*k);command[13+k*2]=n;command[14+k*2]=n>>8;
        }
        command[19]=0;command[20]=0;
        ((SubmitFn)0x537bd0)((void*)0x6a2898,command);
    }return 1;
}
static int spell_slot(u32 slot) {
    u8* bar=(void*)0x6a2dd0;
    if(!capture()||!readable(bar,0x1925)||get(bar+0x1518)>26||slot>=get(bar+0x1518))return 0;
    u8* actor=(void*)get(bar+0x18ec);u32 pool=get((void*)0x6def58);
    if(!readable(actor,0xe4b))return 0;
    u32 index=get(actor);
    if(index>=snapshot.count||actor!=(u8*)(pool+index*0xe4b)||
       !snapshot.actors[index].active||snapshot.actors[index].health<=0||
       !snapshot.actors[index].wizard||snapshot.actors[index].owner!=snapshot.owner||
       get(bar+slot*0xc6+0x1b5)!=0)return 0; /* Inventory slots use another command. */
    if(get(bar+0x1908)!=slot) {
        typedef u32 (THIS *SlotFn)(void*,u32);
        ((SlotFn)0x4d4d60)(bar,slot);
    }return 1;
}
static u32 THIS key(void* object,u32 window,u32 message,u32 vk,u32 lp) {
    u32 error=GetLastError();
    if(object!=(void*)0x6cbb78||!world()||!owned_thread()||
       (message!=0x100&&message!=0x104)) {
        if(!world())new_session();
        SetLastError(error);return ((KeyFn)0x46c070)(object,window,message,vk,lp);
    }
    KeyStateFn polling=(KeyStateFn)get((void*)0x5c5290);
    u32 mods=(polling(16)<0?1u:0u)|(polling(17)<0?2u:0u)|(polling(18)<0?4u:0u);
    int repeat=(lp&0x40000000u)!=0;u32 now=GetTickCount();int handled=0;
    if(vk>=48&&vk<=57&&mods<=4) {
        u32 action=mods==2?MNM_RTS_GROUP_SAVE:mods==1?MNM_RTS_GROUP_ADD:
            mods==3?MNM_RTS_GROUP_REMOVE:MNM_RTS_GROUP_RECALL;
        handled=apply(action,vk-48,0,now,repeat);
        if(handled&&mods==4&&!repeat)handled=apply(MNM_RTS_FOCUS,0,0,now,0);
    } else if(vk>=116&&vk<=119&&(mods==2||mods==4)) {
        u32 slot=vk-116;
        handled=capture()&&camera_ready();
        if(handled&&!repeat) {
            if(mods==2) {
                for(u32 i=0;i<6;++i)camera_positions[slot][i]=get((void*)(0x689930+0x39+i*4));
                camera_saved[slot]=1;camera_sessions[slot]=session;
            } else if(camera_saved[slot]&&camera_sessions[slot]==session) {
                typedef void (THIS *PositionFn)(void*,u32,u32,u32);
                u32* p=camera_positions[slot];
                ((PositionFn)0x4f7a20)((void*)0x689930,p[0]*32+p[3],p[1]*32+p[4],p[2]*16+p[5]);
            }
        }
    } else {
        for(u32 i=0;i<sizeof(bindings)/sizeof(*bindings);++i) {
            const u32* b=bindings[i];if(vk!=b[0]||mods!=b[1])continue;
            if(repeat){handled=1;break;}
            if(b[2]==11) {
                SetLastError(error);return ((KeyFn)0x46c070)(object,window,0x100,27,lp);
            }
            handled=b[2]==12?spell_slot(b[3]):b[2]==13?stop_selection():apply(b[2],b[3],0,now,0);
            break;
        }
    }
    if(handled){SetLastError(error);return 0;}
    policy.last_valid=0;
    SetLastError(error);return ((KeyFn)0x46c070)(object,window,message,vk,lp);
}
/* Preserve all eleven original mouse arguments, including modifier words.
 * The first word carries button edge flags; only left selection is extended. */
static u32 THIS mouse(void* object,u32 a,u32 b,u32 c,u32 d,u32 e,u32 f,
                      u32 g,u32 h,u32 shift,u32 control,u32 alt) {
    u32 error=GetLastError();
    if(object==(void*)0x689930&&world()&&owned_thread()) {
        if(a&3u){policy.cycle_valid=0;policy.last_valid=0;}
        if(RTS_CTRL_ADDITIVE&&(a&3u)&&control)shift=1;
    }
    SetLastError(error);
    return ((MouseFn)0x4fdad0)(object,a,b,c,d,e,f,g,h,shift,control,alt);
}
static u32 THIS reset_actor(void* actor,u32 slot) {
    u32 error=GetLastError();
    if(slot<256){if(!++generations[slot])generations[slot]=1;if(!slot)new_session();}
    SetLastError(error);return original_reset(actor,slot);
}
static int install(void) {
    char activation[4];u32 n=GetEnvironmentVariableA("MNM_RTS_CONTROLS_ACTIVE",activation,sizeof(activation));
    if(!n)return 1;
    if(n!=1||activation[0]!='1'||(u32)GetModuleHandleA(0)!=0x400000)return 0;
    const struct {u32 at;u8 bytes[8];u32 size;} guards[]={
        {0x46c070,{0x83,0xec,0x14,0x53,0x55,0x56,0x8b,0x74},8},
        {0x4fdad0,{0x81,0xec,0x30,1,0,0,0x53,0x55},8},
        {0x5073d0,{0x57,0x8b,0xf9,0x8b,0x8f,0xa8,0,0},8},
        {0x5074f0,{0x53,0x57,0x8b,0xf9,0x33,0xdb,0x39,0x5f},8},
        {0x4f7ae0,{0x57,0x8b,0x7c,0x24,8,0x85,0xff,0x74},8},
        {0x4f7a20,{0x55,0x8b,0x44,0x24,8,0x56,0x8b,0x74},8},
        {0x4d4d60,{0x83,0xec,0x14,0x55,0x56,0x8b,0xf1,0x57},8},
        {0x537bd0,{0x8b,0x41,0x34,0x85,0xc0,0x75,4,0x33},8},
        {0x5060a0,{0x6a,0xff,0x68,0x1b,0x24,0x5c,0},7}
    };
    for(u32 i=0;i<sizeof(guards)/sizeof(*guards);++i)
        if(!same(guards[i].at,guards[i].bytes,guards[i].size))return 0;
    if(!readable((void*)0x5c5dec,4)||get((void*)0x5c5dec)!=0x46c070||
       !readable((void*)0x5c6a54,4)||get((void*)0x5c6a54)!=0x4fdad0)return 0;
    u8* trampoline=VirtualAlloc(0,12,0x3000,0x40);if(!trampoline)return 0;
    for(u32 i=0;i<7;++i)trampoline[i]=*(u8*)(0x5060a0+i);
    trampoline[7]=0xe9;put(trampoline+8,0x5060a7-(u32)trampoline-12);
    if(!FlushInstructionCache(GetCurrentProcess(),trampoline,12))return 0;
    original_reset=(ResetFn)trampoline;
    const u32 sites[]={0x5c5dec,0x5c6a54,0x5060a0},sizes[]={4,4,7};
    u32 old[3],restore;
    for(u32 i=0;i<3;++i)if(!VirtualProtect((void*)sites[i],sizes[i],0x40,old+i)) {
        for(u32 j=0;j<i;++j)VirtualProtect((void*)sites[j],sizes[j],old[j],&restore);
        return 0;
    }
    put((void*)sites[0],(u32)&key);put((void*)sites[1],(u32)&mouse);
    *(u8*)sites[2]=0xe9;put((void*)(sites[2]+1),(u32)&reset_actor-sites[2]-5);
    *(u8*)(sites[2]+5)=*(u8*)(sites[2]+6)=0x90;
    int ok=FlushInstructionCache(GetCurrentProcess(),(void*)sites[2],7);
    if(!ok) {
        put((void*)sites[0],0x46c070);put((void*)sites[1],0x4fdad0);
        for(u32 i=0;i<7;++i)*(u8*)(sites[2]+i)=trampoline[i];
        FlushInstructionCache(GetCurrentProcess(),(void*)sites[2],7);
    }
    int restored=1;
    for(u32 i=0;i<3;++i)if(!VirtualProtect((void*)sites[i],sizes[i],old[i],&restore))restored=0;
    enabled=ok&&restored;
    /* Keep the DLL loaded if a protection restore fails after hook publication.
     * Disabled handlers forward the original path; no dangling hook addresses. */
    return ok;
}
__declspec(dllexport) void RtsControlsAnchor(void){}
int WIN DllMain(void* dll,u32 reason,void* reserved) {
    (void)dll;(void)reserved;u32 error=GetLastError();
    int ok=reason!=1||install();SetLastError(error);return ok;
}

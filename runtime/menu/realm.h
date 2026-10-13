#include "../../protocols/include/mnm/menu_v19.h"
static u8 realm_payload[MNM_MENU_V19_REALM_SIZE];
static u32 realm_map_object,realm_map_kind,realm_map_count,realm_map_pointer;
static u32 realm_map_points[19][2];
static u32 realm_activation,realm_input_index,realm_saved_input[3];
static int realm_map(u8* p,u32 kind,u32 count){
    u8* map=(u8*)get(p+0x59);
    if(!readable(map,800*600))return 0;
    if(realm_map_object==(u32)p&&realm_map_pointer==(u32)map&&realm_map_kind==kind&&realm_map_count==count)return 1;
    volatile u32 sums[19][3];
    for(u32 i=0;i<19;++i)for(u32 j=0;j<3;++j)sums[i][j]=0;
    for(u32 y=0;y<600;++y)for(u32 x=0;x<800;++x){u32 id=map[y*800+x];
        if(id>count)return 0;
        if(id){sums[id-1][0]+=x;sums[id-1][1]+=y;++sums[id-1][2];}
    }
    for(u32 i=0;i<count;++i){if(!sums[i][2])return 0;
        realm_map_points[i][0]=sums[i][0]/sums[i][2];realm_map_points[i][1]=sums[i][1]/sums[i][2];}
    realm_map_object=(u32)p;realm_map_pointer=(u32)map;realm_map_kind=kind;realm_map_count=count;return 1;
}
static int realm_snapshot(void* object,u8* out){
    u8* p=(u8*)object;
    if(p!=(u8*)0x659408||!readable(p,0x1e25)||get(p)!=0x5c6a60||get(p+4)!=4||p[0xc]||
       get(p+0xa2b)||get(p+0xa37)||get(p+0xa44)||p[0xa48]||p[0xa3b]||
       get(p+0x12)!=(u32)(p+0xa49)||!readable((void*)0x68991c,8)||get((void*)0x68991c)||get((void*)0x689920)!=5||
       !readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=3||get((void*)0x6f34ac)!=(u32)p||
       get((void*)0x6f34e0)!=(u32)p||get((void*)0x6f34a8)!=0x657ce0||
       !readable((void*)0x657ce0,8)||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3||
       !readable((void*)0x6e0080,4)||get((void*)0x6e0080)||!readable((void*)0x6c54dc,4)||get((void*)0x6c54dc)||
       !readable((void*)0x6f2d28,24)||get((void*)0x6f2d28)||get((void*)0x6f2d34)||get((void*)0x6f2d2c)||get((void*)0x6f2d30))return 0;
    u32 kind=equal(p+0xa49,"Celtic",7)?1:equal(p+0xa49,"Greek",6)?2:equal(p+0xa49,"Medieval",9)?3:0;
    u32 count=get(p+0x823),player=get(p+0xb49),wizards=get(p+0xb4d);
    if(!kind||!count||count>19||!wizards||wizards>80||player>=wizards||!realm_map(p,kind,count))return 0;
    u32 location=get((u8*)0x65ae1d+player*4),destination=get((u8*)0x65af5d+player*4);
    if(location>count||(destination>count&&destination!=0xffffffffu))return 0;
    u32 current_owner=get((u8*)0x65b1d9+location*4);
    if(current_owner>=wizards)return 0;
    if(!get((u8*)0x65a065+player*20)&&current_owner!=player&&get((u8*)0x65ae1d+current_owner*4)==location)return 0;
    for(u32 i=0;i<MNM_MENU_V19_REALM_SIZE;++i)out[i]=0;
    put(out,3);put(out+4,3);put(out+8,5);put(out+12,kind);put(out+16,count);put(out+20,player);put(out+24,location);put(out+28,destination);
    u32 available=0;
    for(u32 id=1;id<=count;++id){u32 owner=get((u8*)0x65b1d9+id*4),adjacent=0;
        if(owner>=wizards)return 0;
        for(u32 source=1;source<=count;++source)if(get((u8*)0x65b1d9+source*4)==player&&get(p+0x1df+id*4+(source-1)*0x50))adjacent=1;
        u8* row=out+48+(id-1)*24;put(row,id);put(row+4,owner);put(row+8,owner==player);put(row+12,adjacent);
        put(row+16,realm_map_points[id-1][0]);put(row+20,realm_map_points[id-1][1]);
        if(owner==player||adjacent)available|=1u<<id;
    }
    put(out+32,available);
    u8* callback=(u8*)get(p+0xa23);
    if(readable(callback,16)&&get(callback)==0x5c75a0&&get(callback+4)==0x54fd20&&get(callback+8)==0&&get(callback+12)==(u32)p)put(out+36,7);
    return 1;
}
static u32 realm_dispatch(void* object,u32 action,u32 argument){
    u8 state[MNM_MENU_V19_REALM_SIZE];if(realm_activation||!realm_snapshot(object,state))return MNM_MENU_UNAVAILABLE;
    if(action==MNM_MENU_REALM_ACTIVATE){
        if(!argument||argument>get(state+16))return MNM_MENU_INVALID;
        if(!(get(state+32)&(1u<<argument)))return MNM_MENU_UNAVAILABLE;
        if(!readable((void*)0x641fbc,4)||get((void*)0x641fbc)>3||*(u8*)0x6f2d3d)return MNM_MENU_UNAVAILABLE;
        u32 index=get((void*)0x641fbc);if(!readable((u8*)0x641ab8+index*40,16)||get((u8*)0x641ac4+index*40))return MNM_MENU_UNAVAILABLE;
        realm_activation=argument;return MNM_MENU_OK;
    }
    u32 original=action==MNM_MENU_REALM_CHARACTER?2:action==MNM_MENU_REALM_GRIMOIRE?3:action==MNM_MENU_REALM_SPELLBOX?1:0;
    if(!original)return MNM_MENU_UNSUPPORTED;
    if(argument)return MNM_MENU_INVALID;
    if(!(get(state+36)&(1u<<(original-1))))return MNM_MENU_UNAVAILABLE;
    typedef void (WIN *RealmAction)(u32);
    record(2,object,original,0);((RealmAction)0x54fd20)(original);record(3,object,original,0);return MNM_MENU_OK;
}
/* Native semantic ingress only: supply a release to one normally scheduled tick.
 * Never invoke an extra tick or write destination/ownership/progression fields.
 * Restore physical input afterwards; the original owns the consumed latch. */
static int realm_input_begin(void* object){
    if(!realm_activation)return 0;
    u8 state[MNM_MENU_V19_REALM_SIZE];u32 id=realm_activation;realm_activation=0;
    if(!realm_snapshot(object,state)||id>get(state+16)||!(get(state+32)&(1u<<id))||get((void*)0x641fbc)>3||*(u8*)0x6f2d3d)return -1;
    u32 index=get((void*)0x641fbc);u8* input=(u8*)0x641ab8+index*40;
    if(!readable(input,16)||get(input+12))return -1;
    u8* map=(u8*)get((u8*)object+0x59);u32 high=*(u8*)0x6de6d5!=0,width=high?800:640,height=high?600:480;
    u32 x=0,y=0,found=0;
    for(u32 py=0;py<height&&!found;++py)for(u32 px=0;px<width;++px)if(map[(py*600/height)*800+px*800/width]==id){x=px;y=py;found=1;break;}
    if(!found)return -1;
    realm_input_index=index;realm_saved_input[0]=get(input);realm_saved_input[1]=get(input+4);realm_saved_input[2]=get(input+12);
    put(input,x);put(input+4,y);put(input+12,0);*(u8*)0x6f2d3d=1;return 1;
}
static void realm_input_end(void){
    u8* input=(u8*)0x641ab8+realm_input_index*40;put(input,realm_saved_input[0]);put(input+4,realm_saved_input[1]);put(input+12,realm_saved_input[2]);
}

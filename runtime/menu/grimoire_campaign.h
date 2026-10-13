#include "../../protocols/include/mnm/menu_v22.h"
static int grimoire_campaign_enabled;
static u8 grimoire_campaign_payload[MNM_MENU_V22_GRIMOIRE_SIZE];
static int grimoire_static_text(const u8* text){
    if(!text)return 0;
    for(u32 i=0;i<32768;++i){if(!readable(text+i,1))return 0;if(!text[i])return 1;
        if(text[i]=='~'){
            if(!readable(text+i,3))return 0;
            if(text[i+1]=='D'||(text[i+1]=='N'&&text[i+2]=='P'))return 0;
        }
    }return 0;
}
static int grimoire_campaign_snapshot(void* object,u8* out){
    u8* p=(u8*)object,*realm=(u8*)0x659408;
    if(!grimoire_campaign_enabled||p!=(u8*)0x6e0088||!readable(p,0x1a3e)||get(p)!=0x5c6b70||get(p+4)||get(p+8)||get(p+0x42)>=8||get(p+0x46)>=124||get(p+0x4a)!=1||
       !readable((void*)0x68991c,8)||get((void*)0x68991c)||get((void*)0x689920)!=5||
       !readable((void*)0x6f349c,0x48)||get((void*)0x6f34e0)!=(u32)p)return 0;
    u32 depth=get((void*)0x6f349c);if(depth!=4&&depth!=5)return 0;
    if(get((void*)(0x6f34a0+depth*4))!=(u32)p||get((void*)0x6f34a8)!=0x657ce0||get((void*)0x6f34ac)!=(u32)realm||
       !readable((void*)0x657ce0,8)||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3)return 0;
    if(!readable(realm,0x1e25)||get(realm)!=0x5c6a60||get(realm+4)!=4||get(realm+0xa2b)||get(realm+0xa37)||get(realm+0xa44)||realm[0xa48]||
       !(equal(realm+0xa49,"Celtic",7)||equal(realm+0xa49,"Greek",6)||equal(realm+0xa49,"Medieval",9))||!get(realm+0xb4d)||get(realm+0xb4d)>80)return 0;
    int loaded=get(realm+0x12)==(u32)(realm+0xa49)&&get(realm+0x823)&&get(realm+0x823)<=19&&readable((void*)get(realm+0x59),800*600);
    int fresh=depth==5&&!get(realm+0x59)&&!get(realm+8)&&equal(realm+0xa49,"Celtic",7)&&get(realm+0xb4d)==9&&get((void*)0x6e1aba)==1&&get((void*)0x6e1abe)==1;
    if(!loaded&&!fresh)return 0;
    u32 parent=4;
    if(depth==5){u8* entry=(u8*)0x6578c0;parent=18;
        if(get((void*)0x6f34b0)!=(u32)entry||!readable(entry,0x81)||get(entry)!=0x5c667c||get(entry+4)!=18||get(entry+0x7c)!=4||
           !get(entry+0x6f)||(loaded?get(entry+0x6f)>get(realm+0x823):(get(entry+0x6f)!=1||entry[0x77]||get(entry+8)))||entry[0x80])return 0;
    }
    u8* cb=(u8*)get(p+0xa00),*catalog=(u8*)get(p+0x16);
    if(!readable(cb,12)||get(cb)!=0x5c6b8c||get(cb+4)!=0x4c77c0||get(cb+8)!=(u32)p||!readable(catalog,10*0x1fc)||
       !readable((void*)get(p+0x1a),0x960)||!readable((void*)get(p+0x1a3a),4)||get(p+0x1a2a)>32||get((void*)0x6e1f20)>1||
       !readable((void*)0x65b250,80*0x93a))return 0;
    u32 wizard=80;for(u32 i=0;i<80;++i){u8* w=(u8*)0x65b250+i*0x93a;if(get(w)&&!get(w+8)){wizard=i;break;}}
    if(wizard>=get(realm+0xb4d))return 0;
    u8* knowledge=(u8*)get((u8*)0x65b250+wizard*0x93a+0x936);if(knowledge&&!readable(knowledge,1240))return 0;
    static u8 projection[MNM_MENU_V22_GRIMOIRE_SIZE];for(u32 i=0;i<sizeof(projection);++i)projection[i]=0;
    for(u32 i=0;i<MNM_MENU_V22_BOOK_ENTRIES;++i)put(projection+32+4*i,0xffffffff);
    u32 count=0;
    for(u32 chapter=0;chapter<8;++chapter){u32 seen[4]={0,0,0,0};
        for(u32 index=0;index<124;++index){u8* e=(u8*)get(catalog+chapter*0x1fc+12+index*4);if(!e)continue;
            if(!readable(e,32))return 0;
            u32 section=get(e+4);if(section>=124||(seen[section/32]&(1u<<(section%32))))return 0;seen[section/32]|=1u<<(section%32);
            if(!section){put(projection+32+4*(chapter*124),0);++count;continue;}
            if(!readable((void*)get(e),1))return 0;
            u32 level=get((void*)0x6e1f20)==1?4:knowledge?knowledge[chapter*124+section]:0;if(level>4)return 0;
            while(level&&!get(e+12+level*4))--level;
            if(!grimoire_static_text((u8*)get(e+12+level*4)))continue;
            put(projection+32+4*(chapter*124+section),level);++count;
        }
    }
    u32 selected=get(p+0x42)*124+get(p+0x46),level=get(projection+32+4*selected);if(level>4)return 0;
    for(u32 i=0;i<MNM_MENU_V14_GRIMOIRE_SIZE;++i)out[i]=0;
    put(out,get(p+0x42));put(out+4,get(p+0x46));put(out+8,1);put(out+16,parent);put(out+20,depth);put(out+24,5);
    put(projection,wizard);put(projection+4,level);put(projection+8,count);copy(grimoire_campaign_payload,projection,sizeof(projection));return 1;
}

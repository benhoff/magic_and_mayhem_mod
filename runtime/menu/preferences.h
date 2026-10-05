#include "../../protocols/include/mnm/menu_v6.h"
static u8 preferences_payload[MNM_MENU_V6_PREFERENCES_SIZE];
static int preferences_callback(const u8* cb,u32 function,void* receiver){
    return readable(cb,12)&&get(cb)==0x5c64bc&&get(cb+4)==function&&get(cb+8)==(u32)receiver;
}
static u32 preferences_rejections;
static int preferences_reject_detail(void* p,u32 reason,u32 detail){
    if(!(preferences_rejections&(1u<<reason))){preferences_rejections|=1u<<reason;record(5,p,reason,detail);}
    return 0;
}
static int preferences_reject(void* p,u32 reason){return preferences_reject_detail(p,reason,0);}
static int preferences_snapshot(void* object,u8* out){
    u8* p=object;if(p!=(u8*)0x6a4948)return 0;
    if(!readable(p,0x70)||get(p)!=0x5c648c||get(p+4)!=10||get(p+8)!=1)return preferences_reject(p,1);
    if(!readable((void*)0x6f349c,0x48))return preferences_reject(p,2);
    u32 depth=get((void*)0x6f349c);if(depth<1||depth>15||get((void*)(0x6f34a0+depth*4))!=(u32)p)return preferences_reject(p,3);
    u8* parent=(u8*)get((void*)(0x6f34a0+(depth-1)*4));if(!readable(parent,8)||get(parent)!=0x5c63e4||get(parent+4)!=3)return preferences_reject(p,4);
    u8* sliders=(u8*)get(p+0x53),*radios=(u8*)get(p+0x57),*buttons=(u8*)get(p+0x5b),*groups=(u8*)get(p+0x63);
    if(!readable(sliders,2*0x86)||!readable(radios,12*0x7e)||!readable(buttons,2*0x59)||!readable(groups,5*16)||
       !preferences_callback((void*)get(p+0x4b),0x4a9700,p)||!preferences_callback((void*)get(p+0x5f),0x4a9840,p))return preferences_reject(p,5);
    u32 available=0,actions=0;
    for(u32 i=0;i<12;++i){u8* r=radios+i*0x7e;if(get(r)!=0x5c6d40||get(r+0x2d)!=i||get(r+0x3d)>2)return preferences_reject(p,6);
        if(get(r+8)==1&&get(r+0x3d)!=2)available|=1u<<i;}
    for(u32 i=0;i<2;++i){u8* s=sliders+i*0x86,*b=buttons+i*0x59;
        if(get(s)!=0x5c6d10)return preferences_reject_detail(p,11,get(s));
        if(get(s+0x2d)!=i)return preferences_reject_detail(p,12,get(s+0x2d));
        if(get(s+0x65)!=(i?2500u:0u))return preferences_reject_detail(p,13,get(s+0x65));
        if(get(s+0x69)!=(i?5000u:15u))return preferences_reject_detail(p,14,get(s+0x69));
        if(get(s+0x3d)>2)return preferences_reject_detail(p,15,get(s+0x3d));
        if(!preferences_callback((void*)get(s+0x25),0x4a9700,p))return preferences_reject(p,16);
        if(get(b)!=0x5c6c6c)return preferences_reject_detail(p,17,get(b));
        if(get(b+0x2d)!=i)return preferences_reject_detail(p,18,get(b+0x2d));
        if(get(b+0x3d)>2)return preferences_reject_detail(p,19,get(b+0x3d));
        if(!preferences_callback((void*)get(b+0x25),0x4a9840,p))return preferences_reject(p,20);
        u32 v=get(s+0x61);if(v<get(s+0x65)||v>get(s+0x69))return preferences_reject(p,8);
        if(get(s+8)==1&&get(s+0x3d)!=2)available|=1u<<(12+i);
        if(get(b+8)==1&&get(b+0x3d)!=2)actions|=1u<<i;
    }
    for(u32 i=0;i<MNM_MENU_V6_PREFERENCES_SIZE;++i)out[i]=0;
    put(out,actions);put(out+4,available);put(out+8,3);put(out+12,depth);
    put(out+16,get(sliders+0x61));put(out+20,get(sliders+0xe7)-5000);
    const u32 first[5]={0,2,4,7,10},count[5]={2,2,3,3,2};
    for(u32 i=0;i<5;++i){u8* g=groups+i*16;u32 selected=get(g+4);u8* list=(u8*)get(g);
        if(get(g+8)!=count[i]||get(g+12)!=count[i]||selected>=count[i]||!readable(list,count[i]*4))return preferences_reject(p,9);
        for(u32 j=0;j<count[i];++j)if(get(list+j*4)!=(u32)(radios+(first[i]+j)*0x7e))return preferences_reject(p,10);
        put(out+24+i*4,i==4?selected==0:selected);
    }
    return 1;
}
static u32 preferences_dispatch(void* object,u32 action,u32 argument,const u32* values){
    u8 current[MNM_MENU_V6_PREFERENCES_SIZE];if(!preferences_snapshot(object,current))return MNM_MENU_UNAVAILABLE;
    u8* p=object;
    if(action==MNM_MENU_PREFERENCES_CANCEL){if(!(get(current)&2))return MNM_MENU_UNAVAILABLE;}
    else if(action==MNM_MENU_PREFERENCES_OK||action==MNM_MENU_PREFERENCES_PREVIEW){
        if(values[0]>15||(int)values[1]<-2500||(int)values[1]>0||values[2]>1||values[3]>1||values[4]>2||values[5]>2||values[6]>1)return MNM_MENU_INVALID;
        if(action==MNM_MENU_PREFERENCES_PREVIEW){if(argument>1)return MNM_MENU_INVALID;
            if(!(get(current+4)&(1u<<(12+argument))))return MNM_MENU_UNAVAILABLE;
        }else{
            if(!(get(current)&1))return MNM_MENU_UNAVAILABLE;
            const u32 first[5]={0,2,4,7,10};
            for(u32 i=0;i<7;++i)if(values[i]!=get(current+16+i*4)){
                u32 bit=i<2?12+i:first[i-2]+(i==6?!values[i]:values[i]);
                if(!(get(current+4)&(1u<<bit)))return MNM_MENU_UNAVAILABLE;
            }
        }
    }else return MNM_MENU_UNSUPPORTED;
    // All receiver, bounds and availability checks precede any original mutation.
    u8* sliders=(u8*)get(p+0x53),*radios=(u8*)get(p+0x57),*groups=(u8*)get(p+0x63);
    if(action==MNM_MENU_PREFERENCES_OK){
        const u32 first[5]={0,2,4,7,10};
        typedef void (THIS *SelectFn)(void*,void*,u32);
        for(u32 i=0;i<5;++i){u32 selected=i==4?!values[i+2]:values[i+2];
            if(values[i+2]!=get(current+24+i*4))((SelectFn)0x4ce730)(groups+i*16,radios+(first[i]+selected)*0x7e,0);}
    }
    if(action!=MNM_MENU_PREFERENCES_CANCEL){
        for(u32 i=0;i<2;++i)if((action==MNM_MENU_PREFERENCES_OK||argument==i)&&values[i]!=get(current+16+i*4))
            {
                typedef void (THIS *SetFn)(void*,u32);
                record(2,object,65536+i,0);((SetFn)0x4cdf40)(sliders+i*0x86,values[i]+(i?5000:0));record(3,object,65536+i,0);
            }
    }
    if(action!=MNM_MENU_PREFERENCES_PREVIEW){u32 index=action==MNM_MENU_PREFERENCES_OK?0:1;
        record(2,object,index,0);u32 result=((ActionFn)0x4a9840)(object,index);record(3,object,index,result);}
    return MNM_MENU_OK;
}

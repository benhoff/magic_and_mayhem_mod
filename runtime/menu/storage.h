#include "../../protocols/include/mnm/menu_v13.h"
static u8 storage_payload[MNM_MENU_V13_STORAGE_SIZE];
static u32 storage_diagnostic_seen;
static int storage_refuse(u8* p,u32 reason,u32 value){
    if(get(p+8)==1&&!(storage_diagnostic_seen&(1u<<reason))){storage_diagnostic_seen|=1u<<reason;record(31,p,reason,value);}
    return 0;
}
static int storage_name(const u8* name){
    u32 n=0;for(;n<MNM_MENU_V13_NAME_SIZE&&name[n];++n)
        if(name[n]<32||name[n]>=127||name[n]=='/'||name[n]=='\\'||name[n]==':'||name[n]=='"'||name[n]=='<'||name[n]=='>'||name[n]=='|'||name[n]=='?'||name[n]=='*')return 0;
    if(!n||n>=MNM_MENU_V13_NAME_SIZE||name[0]==' '||name[n-1]==' '||name[n-1]=='.')return 0;
    // Windows device names must not turn a save request into an alternate device.
    u8 key[4]={0};u32 end=0;for(;end<n&&name[end]!='.';++end)if(end<4)key[end]=name[end]>='a'&&name[end]<='z'?name[end]-32:name[end];
    if(end==3&&((key[0]=='C'&&key[1]=='O'&&key[2]=='N')||(key[0]=='P'&&key[1]=='R'&&key[2]=='N')||(key[0]=='A'&&key[1]=='U'&&key[2]=='X')||(key[0]=='N'&&key[1]=='U'&&key[2]=='L')))return 0;
    if(end==4&&key[3]>='1'&&key[3]<='9'&&((key[0]=='C'&&key[1]=='O'&&key[2]=='M')||(key[0]=='L'&&key[1]=='P'&&key[2]=='T')))return 0;
    return 1;
}
static int storage_snapshot(void* object,u8* out){
    u8* p=object;u32 id;
    if(!readable(p,0x69))return 0;
    id=get(p+4);int save=id==MNM_MENU_SAVE_SCREEN;
    if(id!=MNM_MENU_LOAD_SCREEN&&id!=MNM_MENU_SAVE_SCREEN)return 0;
    if((save&&(p!=(u8*)0x6de7c8||get(p)!=0x5c64fc))||(!save&&(p!=(u8*)0x6de4e8||get(p)!=0x5c64c4)))return storage_refuse(p,14,get(p));
    if(!readable((void*)0x6f349c,0x48)||!readable((void*)0x68991c,8))return storage_refuse(p,15,0);
    if(get((void*)0x68991c)||get((void*)0x689920)!=5)return storage_refuse(p,16,get((void*)0x689920));
    u32 depth=get((void*)0x6f349c);if((depth!=3&&depth!=6)||get((void*)(0x6f34a0+4*depth))!=(u32)p)return storage_refuse(p,17,depth);
    u8* parent=(u8*)get((void*)(0x6f34a0+4*(depth-1)));
    if(depth==3){if(save||parent!=(u8*)0x657ce0||!readable(parent,8)||get(parent)!=0x5c63e4||get(parent+4)!=3||get(p+0x60)!=3)return storage_refuse(p,18,get(p+0x60));}
    else if(parent!=(u8*)0x6a5088||!readable(parent,0x5b)||get(parent)!=0x5c6644||get(parent+4)!=17||get(parent+0x53)!=2||get(parent+0x57)!=5||get((void*)0x6f34a8)!=0x657ce0||get((void*)0x6f34ac)!=0x659408||get((void*)0x6f34b0)!=0x6cbb78||get((void*)0x6cbb78)!=0x5c5dd8||get((void*)0x6cbb7c)!=2||(!save&&get(p+0x60)!=17))return storage_refuse(p,19,(u32)parent);
    u8* edit=(u8*)get(p+0x4f);u8* list=(u8*)get(p+(save?0x57:0x5b));
    if(!readable(edit,0x64))return storage_refuse(p,1,(u32)edit);
    if(get(edit)!=0x5c6ca0)return storage_refuse(p,2,get(edit));
    if(get(edit+0x60)!=256)return storage_refuse(p,3,get(edit+0x60));
    if(!readable(list,0x6f))return storage_refuse(p,4,(u32)list);
    if(get(list)!=0x5c6db8)return storage_refuse(p,5,get(list));
    u8* callbacks=(u8*)get(p+0x53);if(!readable(callbacks,8))return storage_refuse(p,6,(u32)callbacks);
    for(u32 i=0;i<2;++i){u8* cb=(u8*)get(callbacks+4*i);
        if(!readable(cb,12))return storage_refuse(p,7,(u32)cb);
        if(get(cb)!=(save?0x5c652c:0x5c64f4)||get(cb+4)!=(save?0x4ab750:0x4aa800)||get(cb+8)!=(u32)p)return storage_refuse(p,8,get(cb+4));}
    u32 count=get(list+0x49),selected=get(list+0x5b);u8* rows=(u8*)get(list+0x45);
    if(count>MNM_MENU_V13_MAX_SAVES)return storage_refuse(p,9,count);
    if(count&&!readable(rows,count*0x101))return storage_refuse(p,10,(u32)rows);
    if(selected!=0xffffffffu&&selected>=count)return storage_refuse(p,11,selected);
    for(u32 i=0;i<MNM_MENU_V13_STORAGE_SIZE;++i)out[i]=0;
    put(out,id);put(out+4,get(parent+4));put(out+8,depth);put(out+12,5);put(out+16,get(p+0x3b)!=0);put(out+20,p[0x64]!=0);put(out+24,count);put(out+28,selected==0xffffffffu?0:selected+1);
    if(!battle_string(out+32,128,(void*)get(edit+0x45),128))return storage_refuse(p,12,get(edit+0x45));
    for(u32 i=0;i<count;++i)if(!battle_string(out+160+i*128,128,rows+i*0x101,128))return storage_refuse(p,13,i);
    return 1;
}
static u32 storage_dispatch(void* object,u32 action,u32 row,const u8* name){
    static u8 current[MNM_MENU_V13_STORAGE_SIZE];
    if(!storage_snapshot(object,current)||get(current+16)||get(current+20))return MNM_MENU_UNAVAILABLE;
    u8* p=object;int save=get(current)==MNM_MENU_SAVE_SCREEN;u32 index;
    if(action==MNM_MENU_STORAGE_CANCEL)index=1;
    else if(action==MNM_MENU_STORAGE_ACCEPT||action==MNM_MENU_STORAGE_DELETE){
        u8* edit=(u8*)get(p+0x4f);u8* list=(u8*)get(p+(save?0x57:0x5b));
        const u8* text=name;
        if(!save||action==MNM_MENU_STORAGE_DELETE){
            if((!save&&action==MNM_MENU_STORAGE_DELETE)||!row||row>get(current+24))return MNM_MENU_INVALID;
            text=current+160+(row-1)*128;
            // Validate a safe stem before original serializer/DeleteFile dispatch.
            if(!storage_name(text))return MNM_MENU_INVALID;
        }else if(row||!storage_name(text))return MNM_MENU_INVALID;
        if(!readable((void*)get(edit+0x45),256))return MNM_MENU_UNAVAILABLE;
        // All transaction guards precede any original setter.
        if(!save||action==MNM_MENU_STORAGE_DELETE){((ActionFn)0x4d1060)(list,0xffffffffu);((ActionFn)0x4d1060)(list,row-1);}
        ((ActionFn)0x4cd0e0)(edit,(u32)text);index=action==MNM_MENU_STORAGE_DELETE?2:0;
    }else return MNM_MENU_UNSUPPORTED;
    record(2,object,index,0);u32 result=((ActionFn)(save?0x4ab750:0x4aa800))(object,index);record(3,object,index,result);
    return MNM_MENU_OK;
}

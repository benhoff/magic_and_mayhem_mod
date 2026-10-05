/* Actual Wine DIBSECTIONs, independently copied by the fake engine. */
API HANDLE WIN CreateCompatibleDC(HANDLE);
API HANDLE WIN CreateDIBSection(HANDLE,void*,u32,void**,HANDLE,u32);
API HANDLE WIN SelectObject(HANDLE,HANDLE);
API i32 WIN DeleteObject(HANDLE);
API i32 WIN DeleteDC(HANDLE);
static HANDLE bs_dc,bs_bitmap,bs_old_bitmap;static u8* bs_dc_bits;static u32 bs_dc_calls,bs_dc_fail;
static int bs_dc_case(void){return bootstrap_mode[0]=='d' && bootstrap_mode[1]=='c' && (bootstrap_mode[2]==0 || bootstrap_mode[2]=='-');}
static i32 WIN bs_get_dc(void* object,void** out){
    bs_entry();struct BsSurface* s=bs_state(object);if(s->held)ExitProcess(210);
    s->held=1;++bs_dc_calls;*out=bs_dc;return 23;
}
static i32 WIN bs_release_dc(void* object,void* dc){
    bs_entry();struct BsSurface* s=bs_state(object);if(!s->held || dc!=bs_dc)ExitProcess(211);
    ++bs_dc_calls;if(bs_dc_fail){bs_dc_fail=0;return -1;}
    u32 row=s->width*(s->bits/8);
    for(u32 y=0;y<s->height;++y)for(u32 x=0;x<row;++x)s->pixels[y*row+x]=bs_dc_bits[(bs_mode("dc-bottom-up")?s->height-1-y:y)*row+x];
    /* Capturing after original ReleaseDC would copy poison instead. */
    for(u32 i=0;i<row*s->height;++i)bs_dc_bits[i]=0xcc;s->held=0;return 19;
}
static void bs_dc_seed(struct BsSurface* source){
    u32 d[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))source->table[22])(source,d)!=23 || GetLastError()!=0x88)ExitProcess(212);
    bs_dc=CreateCompatibleDC(0);u32 info[13]={40,800,(u32)(bs_mode("dc-bottom-up")?600:-600),(source->bits<<16)|1,source->bits==24?0:3,0,0,0,0,0,
        source->bits==16?0xf800:0xff0000,source->bits==16?0x7e0:0xff00,source->bits==16?0x1f:0xff};
    if(bs_mode("dc-format"))info[10]=0x7c00,info[11]=0x3e0;
    bs_bitmap=CreateDIBSection(bs_dc,info,0,(void**)&bs_dc_bits,0,0);if(!bs_dc || !bs_bitmap || !bs_dc_bits)ExitProcess(213);
    bs_old_bitmap=SelectObject(bs_dc,bs_bitmap);
    u32 row=source->width*(source->bits/8);
    for(u32 y=0;y<600;++y)for(u32 x=0;x<row;++x)bs_dc_bits[(bs_mode("dc-bottom-up")?599-y:y)*row+x]=source->pixels[y*row+x];
    void* out=0;HANDLE other=0;SetLastError(0x77);
    if(bs_mode("dc-unmatched")){
        if(bs_get_dc(source,&out)!=23 || GetLastError()!=0x88)ExitProcess(214);
    }else if(((i32 (WIN *)(void*,void**))source->table[17])(source,&out)!=23 || GetLastError()!=0x88 || out!=bs_dc)ExitProcess(214);
    if(bs_mode("dc-swapped")){
        void* other_bits=0;other=CreateDIBSection(bs_dc,info,0,&other_bits,0,0);
        if(!other || !other_bits)ExitProcess(218);SelectObject(bs_dc,other);
    }
    if(bs_mode("dc-retry")){bs_dc_fail=1;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))source->table[26])(source,bs_dc)!=-1 || GetLastError()!=0x88)ExitProcess(215);}
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))source->table[26])(source,bs_dc)!=19 || GetLastError()!=0x88)ExitProcess(216);
    if(bs_dc_calls!=(bs_mode("dc-retry")?3u:2u))ExitProcess(217);
    SelectObject(bs_dc,bs_old_bitmap);if(other)DeleteObject(other);DeleteObject(bs_bitmap);DeleteDC(bs_dc);
}

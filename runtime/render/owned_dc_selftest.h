/* Ordered handoff fixtures use independently maintained engine pixels. */
static u32 WIN bs_dc_other_thread(void* object){
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))((struct BsSurface*)object)->table[26])(object,bs_dc)!=19 || GetLastError()!=0x88)ExitProcess(221);
    return 0;
}
static void bs_test_owned_dc(struct BsSurface* target,struct BsSurface* source){
    u32 d[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))target->table[22])(target,d)!=23 || GetLastError()!=0x88)ExitProcess(222);
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))target->table[28])(target,0)!=23 || GetLastError()!=0x88)ExitProcess(223);
    bs_fill(target,0); /* Establish a recorded native identity before GetDC. */
    u32 bytes=target->bits/8,row=target->width*bytes;
    bs_dc=CreateCompatibleDC(0);
    u32 info[13]={40,800,(u32)(bs_dc_bottom_up()?600:-600),(target->bits<<16)|1,target->bits==24?0:3,0,0,0,0,0,
        target->bits==16?0xf800:0xff0000,target->bits==16?0x7e0:0xff00,target->bits==16?0x1f:0xff};
    if(bs_mode("dcs-format"))info[10]=0x7c00,info[11]=0x3e0;
    bs_bitmap=CreateDIBSection(bs_dc,info,0,(void**)&bs_dc_bits,0,0);
    if(!bs_dc || !bs_bitmap || !bs_dc_bits)ExitProcess(224);
    bs_old_bitmap=SelectObject(bs_dc,bs_bitmap);HANDLE other=0;
    for(u32 pass=0;pass<(bs_mode("dcs-repeat")?2u:1u);++pass){
        for(u32 y=0;y<600;++y)for(u32 x=0;x<row;++x)bs_dc_bits[(bs_dc_bottom_up()?599-y:y)*row+x]=target->pixels[y*row+x];
        void* out=0;
        if(bs_mode("dcs-get-failed")){
            bs_dc_get_fail=1;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void**))target->table[17])(target,&out)!=-1 || GetLastError()!=0x88 || out)ExitProcess(225);
        }
        SetLastError(0x77);
        if(((i32 (WIN *)(void*,void**))target->table[17])(target,&out)!=23 || GetLastError()!=0x88 || out!=bs_dc)ExitProcess(226);
        /* Asymmetric native writes test tight rows, orientation and unused
         * RGB32 high bits. The fake engine independently copies at release. */
        for(u32 y=7;y<83;++y)for(u32 x=13;x<139;++x){
            u32 value=target->bits==16?((x+y*3+pass*7)&0xffff):
                ((x*17+y*11+pass)&0xffffff)|(target->bits==32?0x80000000u:0);
            bs_put(bs_dc_bits+((bs_dc_bottom_up()?599-y:y)*800+x)*bytes,bytes,value);
        }
        if(bs_mode("dcs-swapped")){
            void* bits=0;other=CreateDIBSection(bs_dc,info,0,&bits,0,0);
            if(!other || !bits)ExitProcess(227);SelectObject(bs_dc,other);
        }
        if(bs_mode("dcs-mutation")){
            u32 key[2]={0,0};SetLastError(0x77);
            if(((i32 (WIN *)(void*,u32,void*))target->table[29])(target,8,key)!=23 || GetLastError()!=0x88)ExitProcess(228);
        }
        if(bs_mode("dcs-pending"))break;
        if(bs_mode("dcs-retry")){
            bs_dc_fail=1;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*))target->table[26])(target,bs_dc)!=-1 || GetLastError()!=0x88)ExitProcess(229);
            /* No frame is published by a failed ReleaseDC. */
            ++bs_draws;bs_record(target);
            bs_put(bs_dc_bits+7*row+13*bytes,bytes,target->bits==16?0x4321:0x876543);
        }
        if(bs_mode("dcs-contended"))RenderCaptureGuardForTest(1);
        if(bs_mode("dcs-thread")){
            HANDLE thread=CreateThread(0,0,&bs_dc_other_thread,target,0,0);
            if(!thread || WaitForSingleObject(thread,0xffffffff)!=0)ExitProcess(230);CloseHandle(thread);
        }else{
            SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*))target->table[26])(target,bs_dc)!=19 || GetLastError()!=0x88)ExitProcess(231);
        }
        if(bs_mode("dcs-contended")){
            RenderCaptureGuardForTest(0);d[0]=124;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*))target->table[22])(target,d)!=23 || GetLastError()!=0x88)ExitProcess(232);
        }
        ++bs_draws;bs_record(target);
    }
    int refused=bs_mode("dcs-format") || bs_mode("dcs-swapped") || bs_mode("dcs-mutation") ||
        bs_mode("dcs-pending") || bs_mode("dcs-thread") || bs_mode("dcs-contended");
    if(!refused){bs_seed(source);bs_draw(target,source,0,0,0);}
    if(bs_dc_calls!=(bs_mode("dcs-pending")?1u:bs_mode("dcs-repeat")?4u:
                     (bs_mode("dcs-retry")||bs_mode("dcs-get-failed"))?3u:2u) ||
       bs_locks!=(refused?0u:1u) || bs_unlocks!=bs_locks || bs_queries || bs_creates ||
       bs_descriptions!=(bs_mode("dcs-contended")?2u:1u))ExitProcess(233);
    SelectObject(bs_dc,bs_old_bitmap);if(other)DeleteObject(other);DeleteObject(bs_bitmap);DeleteDC(bs_dc);
    ExitProcess(0);
}

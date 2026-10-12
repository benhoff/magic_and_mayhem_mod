/* Selected pool budget caller only: cdecl stat result, consumed DWORD +0x14.
   This is not a general CRT stat implementation or a complete stat structure. */
static i32 file_size_query(const char* filename,void* output){
    if(!channel || !(channel[31]&MNM_AUDIO_CAP_FILE_SIZES) || !output)return -1;
    if((u32)output>0xffffffe7u || !audio_memory_readable((u8*)output+20,4,1))return -1;
    char path[512];u32 count,start=0;
    if(!profile_string_copy(filename,path,sizeof(path),&count) || count<=1)return -1;
    const u32 length=count-1;
    for(u32 i=0;i<length;++i){
        if(path[i]=='/' || path[i]=='\\')start=i+1;
        if(path[i]=='.' && i+1<length && path[i+1]=='.' &&
           (!i || path[i-1]=='/' || path[i-1]=='\\') &&
           (i+2==length || path[i+2]=='/' || path[i+2]=='\\'))return -1;
    }
    if(start==length || length-start>259)return -1;
    u32 bytes=0;
    if(request_wave(MNM_AUDIO_FILE_SIZE,0,0,length-start,0,0,path+start,&bytes,0,0))return -1;
    copy((u8*)output+20,&bytes,4);return 0;
}
static i32 native_file_size(const char* filename,void* output){
    const u32 error=GetLastError();const i32 result=file_size_query(filename,output);
    SetLastError(error);return result;
}
static int file_sizes_install(u32 base){
    static const u8 call[5]={0xe8,0x5c,0xf5,0x02,0};
    static const u8 consume[4]={0x8b,0x54,0x24,0x50};
    u8* site=(u8*)(base+0x16ed0f);u8* read=(u8*)(base+0x16ed1b);
    if(!audio_memory_readable(site,5,0) || !audio_memory_readable(read,4,0) ||
       !same(site,call,5) || !same(read,consume,4))return 0;
    u32 protection,ignored;
    if(!VirtualProtect(site,5,0x40,&protection))return 0;
    const i32 relative=(i32)((u32)&native_file_size-(u32)(site+5));copy(site+1,&relative,4);
    FlushInstructionCache(GetCurrentProcess(),site,5);VirtualProtect(site,5,protection,&ignored);return 1;
}
#ifdef MNM_AUDIO_SELFTEST
i32 WIN AudioFileSizeForTest(const char* filename,void* output){return native_file_size(filename,output);}
u32 WIN AudioFileSizesInstallForTest(u32 base){return file_sizes_install(base);}
#endif

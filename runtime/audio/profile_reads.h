/* Selected audio profile reads only. Native snapshot policy is session-scoped. */
static int profile_string_copy(const char* text,char* out,u32 limit,u32* bytes){
    if(!text){out[0]=0;*bytes=1;return 1;}
    for(u32 i=0;i<limit;++i){
        if(!audio_memory_readable(text+i,1,0))return 0;
        u8 c=(u8)text[i];if(c && (c<32 || c>=127))return 0;out[i]=(char)c;
        if(!c){*bytes=i+1;return 1;}
    }return 0;
}
static int profile_path(const char* filename){
    char path[512];u32 count;if(!filename || !profile_string_copy(filename,path,sizeof(path),&count))return 0;
    u32 start=0;
    for(u32 i=0;i+1<count;++i){
        if(path[i]=='/' || path[i]=='\\')start=i+1;
        if(path[i]=='.' && i+1<count && path[i+1]=='.' && (!i || path[i-1]=='/' || path[i-1]=='\\') &&
           (i+2==count-1 || path[i+2]=='/' || path[i+2]=='\\'))return 0;
    }
    static const char name[]="sounds.ini";
    if(count-start!=sizeof(name))return 0;
    for(u32 i=0;i<sizeof(name);++i){u8 c=(u8)path[start+i];if(c>='A' && c<='Z')c+=32;if(c!=(u8)name[i])return 0;}
    return 1;
}
static u32 profile_query(u32 op,const char* section,const char* key,const char* fallback,void* out,u32 value,const char* filename){
    if(!channel || !(channel[31]&MNM_AUDIO_CAP_PROFILE_READS) || !profile_path(filename))return op==MNM_AUDIO_PROFILE_INTEGER?value:0;
    char text[1024];u32 used=0,count;
    if(!section || !profile_string_copy(section,text,128,&count) || count==1)return op==MNM_AUDIO_PROFILE_INTEGER?value:0;
    used+=count;
    if(op!=MNM_AUDIO_PROFILE_SECTION){
        if(!key || !profile_string_copy(key,text+used,128,&count) || count==1)return op==MNM_AUDIO_PROFILE_INTEGER?value:0;
        used+=count;
    }
    if(op==MNM_AUDIO_PROFILE_STRING){if(!profile_string_copy(fallback,text+used,260,&count))return 0;used+=count;}
    struct NativeProfileReply reply={out,value};u32 result=0;
    if(op!=MNM_AUDIO_PROFILE_INTEGER && (!value || value>16384 || (op==MNM_AUDIO_PROFILE_SECTION && value<2) || !audio_memory_readable(out,value,1)))return 0;
    i32 status=request_wave(op,0,value,used,0,0,text,&result,0,op==MNM_AUDIO_PROFILE_INTEGER?0:&reply);
    return status?(op==MNM_AUDIO_PROFILE_INTEGER?value:0):result;
}
static u32 WIN profile_value(const char* section,const char* key,const char* fallback,char* out,u32 capacity,const char* filename){
    u32 error=GetLastError();u32 result=profile_query(MNM_AUDIO_PROFILE_STRING,section,key,fallback,out,capacity,filename);SetLastError(error);return result;
}
static u32 WIN profile_section(const char* section,char* out,u32 capacity,const char* filename){
    u32 error=GetLastError();u32 result=profile_query(MNM_AUDIO_PROFILE_SECTION,section,0,0,out,capacity,filename);SetLastError(error);return result;
}
static u32 WIN profile_integer(const char* section,const char* key,u32 fallback,const char* filename){
    u32 error=GetLastError();u32 result=profile_query(MNM_AUDIO_PROFILE_INTEGER,section,key,0,0,fallback,filename);SetLastError(error);return result;
}
static int profile_install(u32 base){
    static const u32 sites[]={0x16e03a,0x16e20b,0x16ea77,0x16e41f,0x16ebd9,0x16f6cb,0x16e589};
    static const u8 expected[][6]={{0xff,0x15,0xf0,0x50,0x5c,0},{0xff,0x15,0xf0,0x50,0x5c,0},{0xff,0x15,0xf0,0x50,0x5c,0},
        {0x8b,0x1d,0x7c,0x50,0x5c,0},{0xff,0x15,0x7c,0x50,0x5c,0},{0xff,0x15,0x7c,0x50,0x5c,0},{0xff,0x15,0xec,0x50,0x5c,0}};
    void* hooks[]={profile_section,profile_section,profile_section,profile_value,profile_value,profile_value,profile_integer};
    /* Production DllMain checks the pinned executable base before reaching
       this helper. Synthetic guard tests use their own allocated image. */
    for(u32 i=0;i<7;++i)if(!audio_memory_readable((void*)(base+sites[i]),6,0) || !same((void*)(base+sites[i]),expected[i],6))return 0;
    u32 protection,ignored;void* begin=(void*)(base+sites[0]);u32 bytes=sites[5]+6-sites[0];
    if(!VirtualProtect(begin,bytes,0x40,&protection))return 0;
    for(u32 i=0;i<7;++i){u8* at=(u8*)(base+sites[i]);u32 word=i==3?(u32)hooks[i]:(u32)hooks[i]-(u32)(at+5);
        at[0]=i==3?0xbb:0xe8;copy(at+1,&word,4);at[5]=0x90;}
    FlushInstructionCache(GetCurrentProcess(),begin,bytes);VirtualProtect(begin,bytes,protection,&ignored);return 1;
}
#ifdef MNM_AUDIO_SELFTEST
u32 WIN AudioProfileForTest(u32 op,const char* section,const char* key,const char* fallback,char* out,u32 value,const char* filename){
    if(op==MNM_AUDIO_PROFILE_STRING)return profile_value(section,key,fallback,out,value,filename);
    if(op==MNM_AUDIO_PROFILE_SECTION)return profile_section(section,out,value,filename);
    if(op==MNM_AUDIO_PROFILE_INTEGER)return profile_integer(section,key,value,filename);
    return 0;
}
u32 WIN AudioProfileInstallForTest(u32 base){return profile_install(base);}
#endif

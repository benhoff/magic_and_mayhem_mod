/* Bind only named KERNEL32 imports in the privately mapped original image.
 * Constructor/allocator instructions remain original. Other IAT families remain
 * unavailable; this is not execution of the game's CRT/entrypoint or DLL init. */
API HANDLE WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(HANDLE,const char*);
static int original_kernel_name(const char* s){
    const char* expected="KERNEL32.DLL";
    for(u32 i=0;;++i){char c=s[i];if(c>='a'&&c<='z')c-=32;
        if(c!=expected[i])return 0;if(!c)return 1;}
}
static u32 original_kernel_imports(void){
    char path[1024];u32 n=GetEnvironmentVariableA("MNM_ORIGINAL_IMAGE",path,sizeof(path));
    original_require(n&&n<sizeof(path),151);
    HANDLE file=CreateFileA(path,0x80000000,1,0,3,0,0);original_require(file!=(HANDLE)-1,152);
    static u8 bytes[4096];u32 read=0;
    original_require(ReadFile(file,bytes,sizeof(bytes),&read,0)&&read==sizeof(bytes),153);CloseHandle(file);
    u32 pe=original_word(bytes,sizeof(bytes),60),opt=pe+24;
    u32 length=original_word(bytes,sizeof(bytes),opt+56),rva=original_word(bytes,sizeof(bytes),opt+104);
    original_require(rva<length&&length-rva>=20,154);u32 total=0;
    for(u32 i=0;i<128;++i){
        original_require(rva<=length&&length-rva>=20,155);
        u32* d=(u32*)(0x400000+rva);if(!d[0]&&!d[3]&&!d[4])return total;
        original_require(d[3]<length&&d[4]<length,156);
        const char* name=(const char*)(0x400000+d[3]);
        if(original_kernel_name(name)){
            HANDLE module=LoadLibraryA(name);original_require(module!=0,157);
            u32 lookup=d[0]?d[0]:d[4];
            for(u32 j=0;j<2048;++j){
                original_require(lookup+j*4<=length-4&&d[4]+j*4<=length-4,158);
                u32 value=*(u32*)(0x400000+lookup+j*4);if(!value)break;
                original_require(!(value&0x80000000)&&value<length-2,159);
                void* target=GetProcAddress(module,(const char*)(0x400000+value+2));original_require(target!=0,160);
                *(void**)(0x400000+d[4]+j*4)=target;++total;
                original_require(j!=2047,161);
            }
        }
        rva+=20;
    }
    original_require(0,162);return 0;
}

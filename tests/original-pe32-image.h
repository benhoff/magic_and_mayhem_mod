/* Private fixed-base PE32 image for isolated original-function controls.
 * The Python runner pins the complete input SHA-256. No original instruction
 * is patched, no entrypoint is run and no original file is written. */
#include "../runtime/shadow/win32_min.h"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
static void original_require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void original_copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static u32 original_word(const u8* b,u32 n,u32 at){
    original_require(at<=n&&n-at>=4,101);
    return b[at]|(u32)b[at+1]<<8|(u32)b[at+2]<<16|(u32)b[at+3]<<24;
}
static void original_map(void){
    char path[1024];u32 n=GetEnvironmentVariableA("MNM_ORIGINAL_IMAGE",path,sizeof(path));
    original_require(n&&n<sizeof(path),102);
    HANDLE f=CreateFileA(path,0x80000000,1,0,3,0,0);original_require(f!=(HANDLE)-1,103);
    u32 high=0,size=GetFileSize(f,&high);original_require(!high&&size>=256&&size<32*1024*1024,104);
    /* Reserve the original range before a large input-buffer HeapAlloc can
       occupy it in Wine's low-address allocator. */
    static u8 header[4096];u32 header_size=size<sizeof(header)?size:sizeof(header),read=0;
    original_require(ReadFile(f,header,header_size,&read,0)&&read==header_size,118);
    u32 pe=original_word(header,header_size,60),opt=pe+24;
    original_require(original_word(header,header_size,pe)==0x4550&&
        original_word(header,header_size,opt+28)==0x400000,107);
    u32 length=original_word(header,header_size,opt+56);original_require(length&&length<32*1024*1024,108);
    u8* image=VirtualAlloc((void*)0x400000,length,0x3000,0x40);
    if(image!=(void*)0x400000){
        /* A padded fixture reserves this range as part of its own PE image
           before Wine creates heaps. Verify that its live code begins after
           the entire original image; never overwrite an unrelated mapping. */
        u8* own=GetModuleHandleA(0);u32 own_pe;
        original_require(own==(void*)0x400000,109);
        own_pe=original_word(own,4096,60);
        u32 own_entry=original_word(own,4096,own_pe+24+16);
        u32 own_length=original_word(own,4096,own_pe+24+56),old;
        original_require(own_entry>=0x401000&&length<0x400000&&own_length>own_entry,119);
        original_require(VirtualProtect(own,length,0x40,&old),120);image=own;
    }
    CloseHandle(f);f=CreateFileA(path,0x80000000,1,0,3,0,0);original_require(f!=(HANDLE)-1,103);
    u8* bytes=HeapAlloc(GetProcessHeap(),0,size);original_require(bytes!=0,105);
    read=0;original_require(ReadFile(f,bytes,size,&read,0)&&read==size,106);CloseHandle(f);
    original_require(original_word(bytes,size,pe)==0x4550&&
        original_word(bytes,size,opt+28)==0x400000,107);
    original_require(original_word(bytes,size,opt+56)==length,108);
    original_require(pe<=size&&size-pe>=24,110);
    u32 count=bytes[pe+6]|(u32)bytes[pe+7]<<8;
    u32 optional=bytes[pe+20]|(u32)bytes[pe+21]<<8;
    original_require(count&&count<=96&&optional>=96,111);
    for(u32 i=0;i<count;++i){
        u32 row=opt+optional+i*40;
        u32 rva=original_word(bytes,size,row+12),stored=original_word(bytes,size,row+16),
            offset=original_word(bytes,size,row+20);
        original_require(rva<=length&&stored<=length-rva&&offset<=size&&stored<=size-offset,112);
        original_copy(image+rva,bytes+offset,stored);
    }
    HeapFree(GetProcessHeap(),0,bytes);
}

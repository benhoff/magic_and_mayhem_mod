/* Opt-in pre-opened fatal breadcrumb. Never acquire the recorder/scene locks or
 * allocate memory while diagnosing an observer failure. Empty on success. */
static HANDLE drawing_fatal_file;
static void __attribute__((noreturn,noinline)) drawing_fatal_exit(u32 code,const char* file,u32 line){
 if(code&&drawing_fatal_file){u32 r[64]={0};const char magic[8]={'M','N','M','D','F','A','T','1'};u8* out=(u8*)r;for(u32 i=0;i<8;++i)out[i]=(u8)magic[i];r[2]=1;r[3]=code;r[4]=line;r[5]=GetCurrentThreadId();for(u32 i=0;i<223&&file[i];++i)out[32+i]=(u8)file[i];u32 written;WriteFile(drawing_fatal_file,r,256,&written,0);}
 ExitProcess(code);for(;;){}
}
#define ExitProcess(code) drawing_fatal_exit((code),__FILE__,__LINE__)

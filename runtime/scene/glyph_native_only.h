/* Native-only glyph admission must never return to the original pixel body. */
static u32 glyph_native_only;
static void glyph_native_only_configure(void){
 const u32 error=GetLastError();char value[16];SetLastError(0);
 u32 n=GetEnvironmentVariableA("MNM_NATIVE_ONLY",value,sizeof(value));
 if(!n){if(GetLastError()!=203)ExitProcess(124);SetLastError(error);return;}
 if(n!=1 || value[0]!='1')ExitProcess(124);
 glyph_native_only=1;
 const char* expected="takeover";
 n=GetEnvironmentVariableA("MNM_GLYPH_BACKEND",value,sizeof(value));
 if(n!=8)ExitProcess(124);
 for(u32 i=0;i<8;++i)if(value[i]!=expected[i])ExitProcess(124);
 if(GetEnvironmentVariableA("MNM_GLYPH_CAPTURE",value,sizeof(value))!=1 || value[0]!='0')ExitProcess(124);
 if(GetEnvironmentVariableA("MNM_PUBLIC_RESIDENT_WORLD",value,sizeof(value))!=1 || value[0]!='1')ExitProcess(124);
 SetLastError(error);
}
static void glyph_native_only_refuse(u32 reason){
 if(!glyph_native_only)return;
 u32 r[8]={0x474d4e4d,0x3146524c,1,32,reason,GetCurrentThreadId(),0,0},written=0;
 HANDLE file=CreateFileA("native-glyph-forward-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,r,sizeof(r),&written,0);CloseHandle(file);}
 ExitProcess(124);
}

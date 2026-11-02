/* Optional bounded observation of the recovered +906 follow reference.
 * Original sampling only; never installs a reference or issues a command. */
static HANDLE gameplay_follow_file;
static u32 gameplay_follow_sequence;
static int gameplay_follow_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_CAMPAIGN_FOLLOW_OBSERVE",flag,sizeof(flag));
 if(!n)return 1;
 if(n!=1||flag[0]!='1')return 0;
 gameplay_follow_file=CreateFileA("native-creature-follow.bin",0x40000000,1,0,1,0x80,0);
 if(gameplay_follow_file==(HANDLE)-1){gameplay_follow_file=0;return 0;}
 const u32 header[4]={0x464d4e4d,0x3130304c,1,64};u32 wrote=0;
 if(!WriteFile(gameplay_follow_file,header,sizeof(header),&wrote,0)||wrote!=sizeof(header)){
  CloseHandle(gameplay_follow_file);gameplay_follow_file=0;return 0;
 }
 return 1;
}
static void gameplay_follow_sample(u8* p){
 if(!gameplay_follow_file||gameplay_follow_sequence>=4096||get(p+0xa8)!=14||get(p+0x174)!=0||!get(p+4))return;
 u32 error=GetLastError(),reference=get(p+0x906),wrote=0;u8* target=gameplay_creature(reference);
 u32 row[16]={++gameplay_follow_sequence,GetTickCount(),GetCurrentThreadId(),get(p),get(p+0xa8),get(p+0x174),get(p+4),reference,
  target?get(target+0xa8):0xffffffffu,target?get(target+0x174):0xffffffffu,target?get(target+4):0,target?get(target+0xe4):0,
  get(p+0xe4),get(p+8),get(p+0xc),get(p+0x10)};
 if(!WriteFile(gameplay_follow_file,row,sizeof(row),&wrote,0)||wrote!=sizeof(row)){CloseHandle(gameplay_follow_file);gameplay_follow_file=0;}
 SetLastError(error);
}

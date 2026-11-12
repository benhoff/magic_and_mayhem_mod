/* Optional transition-only original Space/Escape API results. No input writes. */
static char input_ui_path[2048];
static u32 input_ui_rows[16+64*12],input_ui_count,input_ui_seen[2],input_ui_down[2];
static volatile i32 input_ui_busy;
static void input_ui_acceptance(i32 code,u32 value,u32 sequence,u32 owned,u32 caller){
 if(!input_ui_path[0]||(code!=32&&code!=27)||__sync_lock_test_and_set(&input_ui_busy,1))return;
 u32 slot=code==32?0:1,down=(value&0x8000)!=0;
 if((input_ui_seen[slot]?down!=input_ui_down[slot]:down)&&input_ui_count<64){
  u32* row=input_ui_rows+16+input_ui_count*12;
  row[0]=++input_ui_count;row[1]=GetTickCount();row[2]=GetCurrentThreadId();row[3]=code;row[4]=value&65535;
  row[5]=owned;row[6]=sequence;row[7]=caller;input_ui_seen[slot]=1;input_ui_down[slot]=down;
  copy(input_ui_rows,"MNMUIK01",8);input_ui_rows[2]=1;input_ui_rows[3]=48;input_ui_rows[4]=input_ui_count;input_ui_rows[5]=64;
  HANDLE file=CreateFileA(input_ui_path,0x40000000,3,0,2,0x80,0);u32 written=0;
  if(file!=(HANDLE)-1){WriteFile(file,input_ui_rows,64+input_ui_count*48,&written,0);CloseHandle(file);}
 }
 __sync_lock_release(&input_ui_busy);
}
static void input_ui_configure(void){
 char path[2048];u32 length=GetEnvironmentVariableA("MNM_DRAW_UI_ACCEPTANCE_FILE",path,sizeof(path));
 if(length&&length+5<sizeof(input_ui_path)){copy(input_ui_path,path,length);copy(input_ui_path+length,".keys",6);}
}

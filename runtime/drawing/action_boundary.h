/* Optional reference-only action epochs. Atomic same-directory publication;
 * never a native drawing input. Existing API journal remains authoritative. */
API i32 WIN MoveFileExA(const char*,const char*,u32);
static u32 action_state_enabled,action_state[16];
static int action_state_publish(void){
 char temporary[600],final[600];path(temporary,"drawing-action-state.next");path(final,"drawing-action-state.bin");
 HANDLE f=CreateFileA(temporary,0x40000000,3,0,2,0x80,0);if(f==(HANDLE)-1)return 0;
 int ok=write(f,action_state,64);if(!CloseHandle(f))ok=0;
 return ok&&MoveFileExA(temporary,final,1|8);
}
static int action_state_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_ACTION_BOUNDARIES",flag,2);
 if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1')return 0;
 copy(action_state,"MNMDACT1",8);action_state[2]=1;action_state[3]=64;action_state_enabled=1;return action_state_publish();
}
static void action_state_record(const struct Context* c){
 if(!action_state_enabled||!c->observed)return;
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);EnterCriticalSection(journal_lock);
 u32 phase=c->r[26],request=c->r[21];
 if(!request||(phase!=1&&phase!=2)||action_state[4]>=65536||
    (phase==1&&action_state[10]==1)||(phase==2&&(action_state[10]!=1||action_state[5]!=request||action_state[6]!=c->r[22]||action_state[7]!=c->r[23]||action_state[8]!=c->r[24]||action_state[9]!=c->r[25])))ExitProcess(95);
 ++action_state[4];for(u32 i=0;i<7;++i)action_state[5+i]=c->r[21+i];action_state[12]=c->r[29];action_state[13]=c->r[30];
 if(phase==2){action_state[14]=request;action_state[15]=action_state[4];}
 if(!action_state_publish())ExitProcess(95);LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
}

/* Separate additive diagnostic ordering. Never hold this lock across the
 * original call, driver classification, GDI flush, or physical ACK wait.
 * These points describe observer order, not scanout or producer ownership. */
static HANDLE interval_file;
static u32 interval_records;
static int interval_open(void){
 char name[600];path(name,"api-call-intervals.bin");
 interval_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(interval_file==(HANDLE)-1){interval_file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMDINT1",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=786433;
 return write(interval_file,h,64);
}
/* Caller owns journal_lock. Entry/return fields are captured before any
 * observer readback; ACK is recorded only after the exact peer ACK is read. */
static void interval_event(const struct Context* c,u32 phase){
 if(!interval_file)return;
 u32 r[16]={0};r[0]=++interval_records;r[1]=phase;r[2]=c->r[29];r[3]=c->r[1];r[4]=c->r[2];
 r[5]=c->r[3];r[6]=c->interval_related;r[7]=c->r[6];r[8]=c->r[5];r[9]=GetTickCount();
 if(phase>=2)r[10]=c->r[8];if(phase==3){r[11]=c->r[12];r[12]=c->r[0];}
 r[13]=pending;
 if(interval_records>=786433||!write(interval_file,r,64))ExitProcess(97);
}
static void interval_close(u32 reason){
 if(!interval_file)return;
 u32 r[16]={0};r[0]=++interval_records;r[1]=4;r[2]=reason;r[7]=GetCurrentThreadId();r[9]=GetTickCount();
 r[10]=records;r[11]=planes;r[12]=entries;r[13]=pending;r[14]=drops;r[15]=gaps;
 if(interval_records>786433||!write(interval_file,r,64)||!CloseHandle(interval_file))ExitProcess(97);
 interval_file=0;
}

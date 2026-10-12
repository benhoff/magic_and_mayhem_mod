#pragma once
/* Prospective bounded separate diagnostic journal; no handles persisted. */
static unsigned cursor_gate_journal_lock[6],cursor_gate_journal_count,cursor_gate_journal_drops,cursor_gate_journal_finalized;
static unsigned cursor_gate_journal_rows[8192][48],cursor_gate_journal_snapshot[8192][48];
static HANDLE cursor_gate_journal_file;
static void cursor_gate_journal_emit(unsigned phase,unsigned reason,unsigned original_result,
 unsigned diagnostic_result,unsigned error,unsigned role,unsigned call,unsigned identity,unsigned pending,const unsigned* point){
 unsigned row[48]={0};row[1]=phase;row[2]=GetCurrentThreadId();row[3]=GetTickCount();row[4]=reason;row[8]=original_result;row[9]=diagnostic_result;row[10]=error;row[11]=role;row[12]=call;row[13]=identity;row[14]=pending;
 if(point){row[5]=point[5];row[6]=point[7];row[7]=point[8];cursor_gate_copy(row+16,point,64);}
 EnterCriticalSection(cursor_gate_journal_lock);
 if(cursor_gate_journal_finalized||cursor_gate_journal_count==8192)++cursor_gate_journal_drops;
 else{row[0]=++cursor_gate_journal_count;cursor_gate_copy(cursor_gate_journal_rows+row[0]-1,row,192);}
 LeaveCriticalSection(cursor_gate_journal_lock);
}
static int cursor_gate_journal_write(const void* bytes,unsigned size){
 unsigned at=0;while(at<size){unsigned written=0;if(!WriteFile(cursor_gate_journal_file,(const unsigned char*)bytes+at,size-at,&written,0)||!written||written>size-at)return 0;at+=written;}return 1;
}
/* Caller first freezes module admission and proves no original wrapper/gate
 * writer remains. Otherwise a separate copied snapshot is required. */
static int cursor_gate_journal_finish(unsigned reason,unsigned active,unsigned fault,unsigned orphans){
 if(!cursor_gate_journal_file)return 0;
 unsigned h[16]={0};cursor_gate_copy(h,"MNMDGT01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=8192;h[6]=cursor_gate_journal_count;h[7]=cursor_gate_journal_drops;h[8]=reason;h[9]=active;h[10]=fault;h[11]=orphans;
 EnterCriticalSection(cursor_gate_journal_lock);
 if(cursor_gate_journal_finalized){LeaveCriticalSection(cursor_gate_journal_lock);return 0;}
 cursor_gate_journal_finalized=1;unsigned count=cursor_gate_journal_count;h[6]=count;h[7]=cursor_gate_journal_drops;
 cursor_gate_copy(cursor_gate_journal_snapshot,cursor_gate_journal_rows,count*192);
 LeaveCriticalSection(cursor_gate_journal_lock);
 int ok=cursor_gate_journal_write(h,64)&&cursor_gate_journal_write(cursor_gate_journal_snapshot,count*192);
 if(!CloseHandle(cursor_gate_journal_file))ok=0;cursor_gate_journal_file=0;return ok;
}
static int cursor_gate_journal_open(void){
 char path[1024];unsigned n=GetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE_FILE",path,sizeof(path));
 if(!n||n>=sizeof(path))return 0;
 InitializeCriticalSection(cursor_gate_journal_lock);
 cursor_gate_journal_file=CreateFileA(path,0x40000000,0,0,1,0x80,0);
 return cursor_gate_journal_file!=(HANDLE)-1;
}

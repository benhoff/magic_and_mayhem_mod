/* Opt-in observations, serialized by the existing COPY lease. No admission,
 * coordinate conversion, input lease or original return value changes. */
#include "../../protocols/include/mnm/cursor_copy_state_v1.h"
static struct MnmCursorCopyState* cursor_copy_state;
typedef i32(WIN* CursorCopyRectangle)(void*,i32*);
static CursorCopyRectangle cursor_copy_window_rectangle,cursor_copy_client_rectangle;
static void cursor_copy_state_init(const u32* census){
 char flag[4];
 if(GetEnvironmentVariableA("MNM_CURSOR_COPY_STATE",flag,sizeof(flag))!=1 || flag[0]!='1')return;
 char path[544];copy(path,lock_capture_path,lock_capture_path_length);
 static const char suffix[]="\\cursor-copy-state-v1.bin";copy(path+lock_capture_path_length,suffix,sizeof(suffix));
 HANDLE file=CreateFileA(path,0xc0000000,1,0,1,0x80,0);
 if(file==(HANDLE)-1)return;
 HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_CURSOR_COPY_STATE_BYTES,0);CloseHandle(file);
 if(!mapping)return;cursor_copy_state=MapViewOfFile(mapping,2,0,0,MNM_CURSOR_COPY_STATE_BYTES);CloseHandle(mapping);
 if(!cursor_copy_state)return;
 u32* h=cursor_copy_state->header;copy(h,"MNMCCS01",8);
 h[2]=1;h[3]=128;h[4]=320;h[5]=MNM_CURSOR_COPY_STATE_CAPACITY;h[10]=0x40209ca7;h[11]=census[15];h[12]=census[16];
 void* user=GetModuleHandleA("USER32.dll");
 if(user){cursor_copy_window_rectangle=(CursorCopyRectangle)GetProcAddress(user,"GetWindowRect");cursor_copy_client_rectangle=(CursorCopyRectangle)GetProcAddress(user,"GetClientRect");}
}
static void cursor_copy_state_observe(const u32* copy_row){
 if(!cursor_copy_state || (copy_row[2]!=0x18c488u && copy_row[2]!=0x18cbb2u))return;
 u32 row[80];zero(row,sizeof(row));copy(row,copy_row,176);row[44]=GetTickCount();row[45]=(u32)input_window;
 row[46]=input_words?(input_desktop_coordinates?2u:1u):0u;
 if(input_words){
  row[47]=__atomic_load_n(input_words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);
  row[48]=__atomic_load_n(input_words+MNM_INPUT_V1_ACTIVE_OFFSET/4,__ATOMIC_RELAXED);
  row[49]=__atomic_load_n(input_words+MNM_INPUT_V1_CURSOR_X_OFFSET/4,__ATOMIC_RELAXED);
  row[50]=__atomic_load_n(input_words+MNM_INPUT_V1_CURSOR_Y_OFFSET/4,__ATOMIC_RELAXED);
  row[51]=__atomic_load_n(input_words+MNM_INPUT_V1_WIDTH_OFFSET/4,__ATOMIC_RELAXED);
  row[52]=__atomic_load_n(input_words+MNM_INPUT_V1_HEIGHT_OFFSET/4,__ATOMIC_RELAXED);
  __atomic_thread_fence(__ATOMIC_ACQUIRE);
  row[53]=__atomic_load_n(input_words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);
  row[77]=row[47]==row[53] && !(row[47]&1);
 }
 /* Pinned NoCD saved cursor state; never dereference an observed host pointer. */
 static const u32 addresses[10]={0x642c54,0x641a78,0x641a7c,0x641a80,0x641a84,0x641aa0,0x641aa4,0x5e04c8,0x642c64,0x6e1ee4};
 row[78]=1;
 for(u32 i=0;i<10;++i){if(readable((void*)addresses[i],4))row[54+i]=*(const volatile u32*)addresses[i];else row[78]=0;}
 if(input_window && cursor_copy_window_rectangle)row[64]=(u32)cursor_copy_window_rectangle(input_window,(i32*)(row+65));
 if(input_window && cursor_copy_client_rectangle)row[69]=(u32)cursor_copy_client_rectangle(input_window,(i32*)(row+70));
 if(input_original_cursor)row[74]=(u32)input_original_cursor((i32*)(row+75));
 mnm_cursor_copy_state_record(cursor_copy_state,row);
}
static void cursor_copy_state_close(const u32* census){
 if(!cursor_copy_state)return;
 cursor_copy_state->header[13]=census[6];cursor_copy_state->header[14]=census[8];
 cursor_copy_state->header[15]=census[7]|census[9];cursor_copy_state->header[9]=1;
 UnmapViewOfFile(cursor_copy_state);cursor_copy_state=0;
}

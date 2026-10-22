#include "storage_point_exports.h"
/* Exact map interval linkage; the descriptor address never enters the wire. */
typedef u32(WIN* StoragePoint)(void*,u32,u32,u32*,u32);
static StoragePoint wine_map_storage_getter;
static HANDLE wine_map_storage_file;
static u32 wine_map_storage_enabled,wine_map_storage_records,wine_map_storage_points;
static void wine_map_storage_emit(u32* r){r[0]=++wine_map_storage_records;if(wine_map_storage_records>131072||!write(wine_map_storage_file,r,96))ExitProcess(99);}
static int wine_map_storage_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_MAP_STORAGE",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!producer_points_enabled)return 0;
 char name[600];path(name,"wine-map-storage-point.bin");wine_map_storage_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_map_storage_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWSP01",8);h[2]=1;h[3]=96;h[4]=0x40209ca7;h[5]=131072;if(!write(wine_map_storage_file,h,64))return 0;__atomic_store_n(&wine_map_storage_enabled,1,__ATOMIC_RELEASE);return 1;
}
static void wine_map_storage_capture(const u32* map,const u32* descriptor){
 if(!__atomic_load_n(&wine_map_storage_enabled,__ATOMIC_ACQUIRE))return;
 if(!wine_map_storage_getter){void* scene=GetModuleHandleA("MnmScene.dll");wine_map_storage_getter=scene?(StoragePoint)GetProcAddress(scene,storage_point_export_name(0)):0;if(!wine_map_storage_getter)ExitProcess(99);}
 u32 r[24]={0};r[1]=2;r[2]=map[2];r[3]=map[10];r[4]=map[27];r[5]=GetCurrentThreadId();r[6]=GetTickCount();
 /* Original-driver return and export run outside journal_lock. */
 r[7]=wine_map_storage_getter((void*)descriptor[2],map[24],map[25],r+8,64);
 if(r[7]>1)ExitProcess(99);if(!r[7])memset(r+8,0,64);
 EnterCriticalSection(journal_lock);if(__atomic_load_n(&wine_map_storage_enabled,__ATOMIC_ACQUIRE)){++wine_map_storage_points;wine_map_storage_emit(r);}LeaveCriticalSection(journal_lock);
}
static void wine_map_storage_close(u32 reason){
 if(!wine_map_storage_file)return;__atomic_store_n(&wine_map_storage_enabled,0,__ATOMIC_RELEASE);u32 r[24]={0};r[1]=4;r[2]=reason;r[3]=wine_map_storage_points;r[5]=GetCurrentThreadId();r[6]=GetTickCount();wine_map_storage_emit(r);if(!CloseHandle(wine_map_storage_file))ExitProcess(99);wine_map_storage_file=0;
}

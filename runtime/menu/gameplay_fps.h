#include "../../protocols/include/mnm/gameplay_fps_v1.h"
static u32* gameplay_fps_words;
static u32 gameplay_fps_previous;
static int gameplay_fps_opened,gameplay_fps_observed;
static void gameplay_fps_open(void){
    if(gameplay_fps_opened)return;
    gameplay_fps_opened=1;
    char path[1040];u32 n=GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,1024);
    if(!n||n>=1024)return;
    copy(path+n,".gameplay",10);
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    if(file==(HANDLE)-1)return;
    u32 high=0,size=GetFileSize(file,&high);
    HANDLE mapping=size==MNM_GAMEPLAY_FPS_BYTES&&!high?CreateFileMappingA(file,0,4,0,0,0):0;
    CloseHandle(file);if(!mapping)return;
    u32* p=MapViewOfFile(mapping,2|4,0,0,MNM_GAMEPLAY_FPS_BYTES);CloseHandle(mapping);
    if(!p)return;
    if(!mnm_gameplay_fps_header(p)||p[4]||p[5]||p[6]||p[7]){UnmapViewOfFile(p);return;}
    gameplay_fps_words=p; // Process lifetime, released by the OS at exit.
}

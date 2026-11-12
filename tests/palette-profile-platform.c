/* Freestanding PE32 probe of the actual profile/directory boundary. */
typedef unsigned int U;
#define API __declspec(dllimport)
#define CALL __stdcall
API U CALL GetCurrentDirectoryA(U,char*);
API U CALL GetPrivateProfileStringA(const char*,const char*,const char*,char*,U,const char*);
API void* CALL CreateFileA(const char*,U,U,void*,U,U,void*);
API int CALL WriteFile(void*,const void*,U,U*,void*);
API int CALL CloseHandle(void*);
API void CALL ExitProcess(U);
static const char* sections[]={"STONE_EFFECT","STONE_EFFECT","STONE_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","REDTINT_EFFECT"};
static const char* keys[]={"Reducer","SpecalValue","Darkest","Reducer","SpecalValue","Darkest","BlueTint","Contrast","Reducer"};
void start(void){
    struct {U returned;char text[256];} record;
    char path[512];U i,j,written,length=GetCurrentDirectoryA(256,path);
    void* output=CreateFileA("profile.bin",0x40000000,0,0,2,0x80,0);
    if(output==(void*)-1 || !length || length>=256)ExitProcess(2);
    record.returned=length;for(j=0;j<256;++j)record.text[j]=j<=length?path[j]:0;
    if(!WriteFile(output,&record,sizeof(record),&written,0)||written!=sizeof(record))ExitProcess(3);
    if(path[length-1]!='\\'&&path[length-1]!='/')path[length++]='\\';
    {const char tail[]="cfg\\tables.cfg";for(j=0;j<sizeof(tail);++j)path[length+j]=tail[j];}
    for(i=0;i<9;++i){
        for(j=0;j<256;++j)record.text[j]=0;
        record.returned=GetPrivateProfileStringA(sections[i],keys[i],"",record.text,256,path);
        if(record.returned>255 || record.text[record.returned] ||
           !WriteFile(output,&record,sizeof(record),&written,0)||written!=sizeof(record))ExitProcess(4);
    }
    if(!CloseHandle(output))ExitProcess(5);
    ExitProcess(0);
}

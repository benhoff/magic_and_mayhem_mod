/* Admission must reject diagnostic public-page modes before creating objects. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
typedef uint32_t u32;
static const char* selector;
static const char* resident;
static u32 error;
static jmp_buf stopped;
static void SetLastError(u32 value){error=value;}
static u32 GetLastError(void){return error;}
static u32 GetEnvironmentVariableA(const char* name,char* out,u32 bytes){
 const char* value=!strcmp(name,"MNM_NATIVE_OWNERSHIP_REQUIRED")?selector:resident;
 if(!value){error=203;return 0;}
 u32 size=(u32)strlen(value);if(size>=bytes)return size+1;
 memcpy(out,value,size+1);return size;
}
static void ExitProcess(u32 code){if(code!=124)abort();longjmp(stopped,1);}
#include "../runtime/native-draw/ownership_admission.h"
static int attempt(u32 mode){
 if(setjmp(stopped))return 1;
 ns_ownership_admit(mode);return 0;
}
int main(void){
 unsigned cases=0;
 const char* choices[]={0,"","0","1","2","yes"};
 const char* worlds[]={0,"","0","1","true"};
 for(unsigned f=0;f<6;++f)for(unsigned w=0;w<5;++w)for(u32 mode=0;mode<4;++mode){
  selector=choices[f];resident=worlds[w];ns_ownership_required=0;
  int refused=attempt(mode);
  int wanted=f!=0 && !(f==3 && w==3 && mode==3);
  if(!!refused!=wanted || (!refused && ns_ownership_required!=(f==3))){fprintf(stderr,"admission %u/%u/%u\n",f,w,mode);return 1;}
  ++cases;
 }
 printf("{\"success\":true,\"cases\":%u}\n",cases);return 0;
}

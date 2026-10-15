#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef uint32_t u32;
static u32 words[64],*menu_words=words,menu_version=1;
static int valid=1,stable=1,live=1,request;
static u32 menu_load(u32* p){return *p;}
static int menu_header(void){return valid;}
/* Both an odd initial sequence and a crossed snapshot must return false. */
static int menu_host(u32* out){out[0]=live;out[2]=request;return stable;}
#include "../runtime/menu/startup_admission.h"
int main(void){
 u32 host[145];
 assert(menu_admission_check(host,0)==MENU_ADMISSION_OK);
 stable=0;assert(menu_admission_check(host,1)==MENU_ADMISSION_BUSY);
 stable=1;assert(menu_admission_check(host,2)==MENU_ADMISSION_OK);
 valid=0;assert(menu_admission_check(host,3)==MENU_ADMISSION_HEADER);valid=1;
 live=0;assert(menu_admission_check(host,4)==MENU_ADMISSION_DEAD);live=1;
 request=1;assert(menu_admission_check(host,5)==MENU_ADMISSION_REQUEST);request=0;
 words[16]=2;assert(menu_admission_check(host,6)==MENU_ADMISSION_OWNED);
 menu_version=17;assert(menu_admission_check(host,7)==MENU_ADMISSION_OK);
 words[32]=1;assert(menu_admission_check(host,8)==MENU_ADMISSION_OWNED);words[32]=0;
 stable=0;assert(menu_admission_check(host,119999)==MENU_ADMISSION_BUSY);
 assert(menu_admission_check(host,120000)==MENU_ADMISSION_TIMEOUT);
 stable=1;assert(menu_admission_check(host,120001)==MENU_ADMISSION_TIMEOUT);
 puts("menu admission policy: stable/busy/header/dead/request/V1/V17 ownership/deadline passed");
}

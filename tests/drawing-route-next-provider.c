#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#ifndef MNM_PROVIDER_ID
#define MNM_PROVIDER_ID 1
#endif
int mnm_route_probe(void){return MNM_PROVIDER_ID;}
int mnm_route_next(void){
 void* pointer=dlsym(RTLD_NEXT,"mnm_route_probe");int(*probe)(void);memcpy(&probe,&pointer,sizeof(probe));
 return probe?probe():-1;
}

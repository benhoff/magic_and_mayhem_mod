#include "../protocols/include/mnm/native_cpu_reader_journal_v1.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
struct Worker {struct MnmCpuReaderJournal* journal;uint32_t id;};
static void* write_rows(void* pointer){
 struct Worker* w=pointer;
 for(uint32_t i=0;i<2000;++i){uint32_t r[32]={0};r[1]=w->id;r[2]=i;mnm_crj_record(w->journal,r);}return 0;
}
int main(void){
 for(unsigned a=0;a<3;++a)for(unsigned b=0;b<3;++b)for(unsigned c=0;c<3;++c)
  assert(mnm_crj_gate(a,b,c)==(a==1 && b==1 && c==1));
 struct MnmCpuReaderJournal* j=calloc(1,sizeof(*j));assert(j);
 uint32_t r[32];for(unsigned n=0;n<32;++n)r[n]=100+n;
 assert(!mnm_crj_record(0,r));assert(mnm_crj_record(j,r));
 assert(j->rows[0][0]==1 && j->rows[0][31]==131);r[31]=999;assert(j->rows[0][31]==131);
 for(unsigned n=1;n<MNM_CRJ_CAPACITY;++n)assert(mnm_crj_record(j,r));
 assert(j->header[6]==MNM_CRJ_CAPACITY && j->header[7]==MNM_CRJ_CAPACITY);
 for(unsigned n=0;n<100;++n)assert(!mnm_crj_record(j,r));
 assert(j->header[8]==100 && j->header[9]==1 && j->rows[0][31]==131 && j->rows[MNM_CRJ_CAPACITY-1][0]==MNM_CRJ_CAPACITY);
 memset(j,0,sizeof(*j));j->header[10]=1;assert(!mnm_crj_record(j,r));assert(j->header[6]==0 && j->header[9]==4);
 memset(j,0,sizeof(*j));pthread_t threads[4];struct Worker workers[4];
 for(unsigned n=0;n<4;++n){workers[n]=(struct Worker){j,n+1};assert(!pthread_create(threads+n,0,write_rows,workers+n));}
 for(unsigned n=0;n<4;++n)assert(!pthread_join(threads[n],0));
 assert(j->header[6]==j->header[7] && j->header[6]+j->header[8]==8000 && !(j->header[9]&~2u));
 unsigned char seen[4][2000]={{0}};
 for(unsigned n=0;n<j->header[6];++n){uint32_t* row=j->rows[n];assert(row[0]==n+1 && row[1]>=1 && row[1]<=4 && row[2]<2000);assert(!seen[row[1]-1][row[2]]++);}
 free(j);return 0;
}

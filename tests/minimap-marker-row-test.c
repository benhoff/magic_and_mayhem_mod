#include <assert.h>
#include <limits.h>
#include "../runtime/scene/minimap_marker_row.h"
struct Reader {unsigned address,value,reads;int readable;};
static int read_row(void* context,unsigned address,unsigned* value){
 struct Reader* r=context;++r->reads;assert(address==r->address);
 *value=r->value;return r->readable;
}
int main(void){
 long long entry,expected;
 struct Reader r={0x1000,0x2000+(12*199+88)*2,0,1};
 assert(mnm_minimap_marker_row(0x1000,0,0x2000,199,199,88,12,read_row,&r,&entry,&expected));
 assert(r.reads==1&&entry==0x1000&&expected==r.value);
 /* Row table base may precede the only readable entry. */
 r.reads=0;r.address=0x1000;r.value=0x2000+(13*199+88)*2;
 assert(mnm_minimap_marker_row(0xffc,1,0x2000,199,199,88,12,read_row,&r,&entry,&expected));assert(r.reads==1);
 r.reads=0;r.readable=0;
 assert(!mnm_minimap_marker_row(0xffc,1,0x2000,199,199,88,12,read_row,&r,&entry,&expected));assert(r.reads==1);
 r.reads=0;r.readable=1;r.value+=2;
 assert(!mnm_minimap_marker_row(0xffc,1,0x2000,199,199,88,12,read_row,&r,&entry,&expected));assert(r.reads==1);
 r.reads=0;
 assert(!mnm_minimap_marker_row(0xffc,1,0x2000,199,198,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0xffc,1,0x2000,0,0,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0xffc,1,0x2000,4097,4097,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0,0,0x2000,199,199,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0xfffffffdu,0,0x2000,199,199,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0x1000,INT_MAX,0x2000,199,199,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0x1000,INT_MIN,0x2000,199,199,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0x1000,0,0xfffffff0u,199,199,88,12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0x1000,0,0,199,199,88,-12,read_row,&r,&entry,&expected));
 assert(!mnm_minimap_marker_row(0x1000,0,0x2000,199,199,88,12,0,&r,&entry,&expected));
 assert(r.reads==0);
 return 0;
}

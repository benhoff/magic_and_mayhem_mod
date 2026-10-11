#include <array>
#include <cassert>
#include <vector>
struct Region {unsigned start,bytes,state,protection;};
static std::vector<Region> regions;
static unsigned probes;
static unsigned query(unsigned p,unsigned *m) {
 ++probes;
 for(auto r:regions) if(p>=r.start && p-r.start<r.bytes) {
  std::array<unsigned,7> fields{r.start,r.start,0,r.bytes,r.state,r.protection,0};
  for(unsigned k=0;k<7;++k)m[k]=fields[k];
  return 28;
 }
 return 0;
}
#define MNM_REGION_QUERY(p,m) query(p,m)
#include "../runtime/scene/readable_extent.h"
int main(){
 regions={{4096,4096,0x1000,4},{8192,4096,0x1000,2}};
 unsigned through=0;assert(mnm_readable_extent(4100,40,&through)&&through==8192&&probes==1);
 assert(mnm_readable_tail(4100,4000,through)&&probes==1);
 assert(mnm_readable_tail(4100,8000,through)&&probes==2);
 for(unsigned bad:{1u,0x104u,0u}) {
  regions[1].protection=bad;
  assert(!mnm_readable_tail(4100,8000,through));
 }
 regions[1].protection=4;regions[1].state=0;assert(!mnm_readable_tail(4100,8000,through));
 regions[1].state=0x1000;
 assert(!mnm_readable_extent(0,40,&through));
 assert(!mnm_readable_extent(~0u-10,40,&through));
 assert(!mnm_readable_tail(~0u-10,40,~0u));
 assert(!mnm_readable_extent(3000,40,&through));
 // A region proof admits addresses both before and after the first header,
 // but must query again across the lower boundary or any unproven region.
 unsigned base=0,end=0;probes=0;
 assert(mnm_readable_window(4100,40,&base,&end)&&base==4096&&end==8192&&probes==1);
 assert(mnm_readable_in_window(4096,512,base,end)&&probes==1);
 assert(mnm_readable_in_window(7000,512,base,end)&&probes==1);
 assert(!mnm_readable_in_window(4095,1,base,end)&&probes==2);
 regions[1].protection=1;
 assert(!mnm_readable_in_window(8000,512,base,end));
 assert(!mnm_readable_in_window(~0u-10,40,base,end));
 assert(!mnm_readable_window(~0u-10,40,&base,&end));
 regions[1].protection=2;
 assert(mnm_readable_window(8100,200,&base,&end)&&!base&&!end);
 // A new admission re-probes even the same address after a protection change.
 regions[0].protection=1;assert(!mnm_readable_extent(4100,40,&through));
 regions[0].protection=4;assert(mnm_readable_extent(4100,40,&through));
 assert(mnm_readable_tail(4100,40,0));
 // A single fog-state admission visits many cells in two readable regions.
 // Compare the unchanged per-cell checker and require constant query counts;
 // neither checker is permitted to reuse a proof for another admission.
 regions={{4096,4096,0x1000,4},{8192,4096,0x1000,2}};
 std::vector<unsigned> states;for(unsigned i=0;i<8192;++i)states.push_back((i<4096?4096u:8192u)+(i%2048));
 probes=0;for(auto p:states)assert(mnm_readable_extent(p,1,0));assert(probes==8192);
 base=end=probes=0;for(auto p:states)assert(mnm_readable_refresh_window(p,1,&base,&end));assert(probes==2);
 // Lower boundary, end boundary, guard/hole, overflow and new admission.
 assert(!mnm_readable_refresh_window(4095,1,&base,&end)&&!base&&!end);
 assert(mnm_readable_refresh_window(8192,1,&base,&end));
 assert(!mnm_readable_refresh_window(12288,1,&base,&end)&&!base&&!end);
 assert(!mnm_readable_refresh_window(~0u,1,&base,&end));
 assert(!mnm_readable_refresh_window(0,1,&base,&end));
 base=end=0;regions[0].protection=0x104;assert(!mnm_readable_refresh_window(4096,1,&base,&end));
 regions[0].protection=4;regions[0].state=0;assert(!mnm_readable_refresh_window(4096,1,&base,&end));
 regions[0].state=0x1000;assert(mnm_readable_refresh_window(4096,1,&base,&end));
 base=end=0;regions[0].protection=1;assert(!mnm_readable_refresh_window(4096,1,&base,&end));
 regions[0].protection=4;regions[1].protection=1;base=end=0;
 assert(!mnm_readable_refresh_window(8100,200,&base,&end));
 regions[1].protection=2;assert(mnm_readable_refresh_window(8100,200,&base,&end)&&!base&&!end);
}

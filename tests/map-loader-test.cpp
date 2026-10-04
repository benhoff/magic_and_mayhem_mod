#include "map.hpp"
#include <cassert>
#include <stdexcept>
using namespace mnm::assets;
static void put(Bytes& b,unsigned at,std::uint32_t value){for(unsigned i=0;i<4;++i)b.at(at+i)=value>>(8*i);}
int main(){
 Bytes b(76+12*12);put(b,0,6);put(b,4,3);put(b,8,2);put(b,12,2);put(b,16,6);put(b,20,12);
 for(unsigned i=0;i<12;++i){put(b,76+12*i,100+i);put(b,76+12*i+4,200+i);put(b,76+12*i+8,300+i);}
 auto decoded=decodeMapPayload(b);assert(std::holds_alternative<MapAsset>(decoded));auto map=std::get<MapAsset>(std::move(decoded));b[76]=0;
 assert(map.cell(0,0,0).definition==100 && map.cell(2,1,1).definition==111 && map.cell(0,1,0).references[1]==203);
 bool rejected=false;try{map.cell(3,0,0);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
 for(auto at:{0U,4U,16U,20U}){auto malformed=b;put(malformed,at,at==0?5:0);assert(std::holds_alternative<PersistenceError>(decodeMapPayload(malformed)));}
 auto huge=b;put(huge,4,0xffffffff);assert(std::holds_alternative<PersistenceError>(decodeMapPayload(huge)));
 b.pop_back();assert(std::holds_alternative<PersistenceError>(decodeMapPayload(b)));b.push_back(0);b.push_back(0);assert(std::holds_alternative<PersistenceError>(decodeMapPayload(b)));
 MapLimits limits;limits.cells=4;assert(std::holds_alternative<PersistenceError>(decodeMapPayload(b,limits)));
 assert(std::holds_alternative<PersistenceError>(decodeMapPayload({})));
}

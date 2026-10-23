#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace mnm::reconstruction::combat {
// NoCD 40209ca7 task6 suffix vectors. M supplies checked value-address memory.
// Normal successful, disjoint allocations only; no SEH or allocator-failure claim.
// The real 418050 base destructor does NOT destroy these suffixes. A caller
// reproducing that path must retain their backing separately after task release.
template<class M> class OwnerTask6Features {
    using U=std::uint32_t;
    M& m;
    U r(U p){return m.get32(p);}
    void w(U p,U v){m.set32(p,v);}
    U count(U h,U off){U b=r(h+4),e=r(h+off);if(!b){if(e)throw std::invalid_argument("task6 null vector");return 0;}if(e<b||(e-b)%4)throw std::invalid_argument("task6 vector range");return (e-b)/4;}
    U allocate(U n){U p=m.allocate(n*4);if(!p)throw std::runtime_error("task6 feature allocation");return p;}
    void append(U h,U value){
        U n=count(h,8),cap=count(h,12),b=r(h+4);
        if(n>cap||cap>4096)throw std::invalid_argument("task6 feature capacity");
        if(n==cap){U c=n+std::max(n,1u),p=allocate(c);for(U i=0;i<n;++i)w(p+4*i,r(b+4*i));m.free(b);b=p;w(h+4,b);w(h+12,b+4*c);}
        w(b+4*n,value);w(h+8,b+4*(n+1));
    }
public:
    explicit OwnerTask6Features(M& memory):m(memory){}
    void candidate(U task,const std::vector<int>& values){
        if(values.size()!=72)throw std::invalid_argument("task6 feature dimension");
        U h=task+0x15f;w(h+8,r(h+4));for(int v:values)append(h,U(v));
    }
    // 4261b0: copy candidate metadata, assign saved vector with exact-size
    // reallocation when capacity is insufficient, otherwise reuse its backing.
    void save(U task){
        for(U o:{0x153u,0x157u,0x15bu})w(task+o+0x1c,r(task+o));
        U from=task+0x15f,to=task+0x17b,n=count(from,8),cap=count(to,12),b=r(to+4);
        if(n>4096||cap>4096)throw std::invalid_argument("task6 saved capacity");
        if(n>cap){m.free(b);b=allocate(n);w(to+4,b);w(to+12,b+n*4);}
        for(U i=0;i<n;++i)w(b+i*4,r(r(from+4)+i*4));
        w(to+8,b+n*4);
    }
};
}

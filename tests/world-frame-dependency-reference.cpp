// Original-only watch: this output is never a native rendering input.
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <vector>
namespace {
struct DependencyWatch {
    unsigned record,op,backend;
    const std::vector<std::uint16_t>& pixels;
    unsigned width;
    std::uint16_t target,source;
    DependencyWatch(unsigned r,unsigned o,unsigned b,const std::vector<std::uint16_t>& p,unsigned w,unsigned h)
        :record(r),op(o),backend(b),pixels(p),width(w),target(0),source(0){
        if(w!=800||h!=600||p.size()!=480000)throw std::runtime_error("Dependency watch requires retained800x600 domain");
        target=p[433*w+548];source=p[433*w+563];
        if(r==1||r==1261)emit("before",target,source);
    }
    void emit(const char* phase,std::uint16_t t,std::uint16_t s)const{
        std::fprintf(stderr,"DEPENDENCY %u %u %u %s %u %u\n",record,op,backend,phase,unsigned(t),unsigned(s));
    }
    ~DependencyWatch(){
        const auto t=pixels[433*width+548],s=pixels[433*width+563];
        if(t!=target||s!=source||record==1261)emit("after",t,s);
    }
};
}
#define MNM_WORLD_RECORD_WATCH(r,o,b,p,w,h) DependencyWatch dependencyWatch(r,o,b,p,w,h)
#include "world-frame-reference.cpp"

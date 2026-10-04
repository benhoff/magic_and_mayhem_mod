#include "sprite_queue.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>
namespace mnm::reconstruction {
static std::int32_t signedBits(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
std::int32_t spriteDepthKey(SpriteDepth p,std::uint32_t view){
    if(view>3 || p.height<0)throw std::invalid_argument("Queue needs view 0..3 and nonnegative height");
    const auto scaled=signedBits(std::uint32_t(p.height)*82u)/100;
    auto result=std::uint32_t(scaled)+std::uint32_t(p.priority);
    // All high-height branches use x+y, including views 1..3.
    if(p.height>=500 || view==0)result+=std::uint32_t(p.x)+std::uint32_t(p.y);
    else if(view==1)result+=std::uint32_t(p.y)-std::uint32_t(p.x);
    else if(view==2)result-=std::uint32_t(p.x)+std::uint32_t(p.y);
    else result+=std::uint32_t(p.x)-std::uint32_t(p.y);
    return signedBits(result);
}
void sortSpriteQueue(std::vector<SpriteQueueEntry>& a){
    if(a.size()>65536)throw std::length_error("Queue fixture limit exceeded");
    if(a.empty())return;
    using Index=std::int32_t;
    std::vector<std::pair<Index,Index>> pending{{0,Index(a.size()-1)}};
    while(!pending.empty()){
        auto [low,high]=pending.back();pending.pop_back();
        while(low<high){
            std::swap(a[low],a[low+(high-low)/2]);
            Index i=low+1,j=high+1;
            for(;;){
                while(i<=high && a[i].key<=a[low].key)++i;
                do{--j;}while(j>low && a[j].key>=a[low].key);
                if(j<i)break;
                std::swap(a[i],a[j]);++i;
            }
            std::swap(a[low],a[j]);
            if(j-low-1>=high-i){
                if(low+1<j)pending.emplace_back(low,j-1);
                low=i;
            }else{
                if(i<high)pending.emplace_back(i,high);
                high=j-1;
            }
        }
    }
}
}

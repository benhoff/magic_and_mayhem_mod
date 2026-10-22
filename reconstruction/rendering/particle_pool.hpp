#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
// 5457e0/5458b0/545910. A successful insertion transfers the token into the
// pool; failure leaves it with the caller. Tokens identify owned generations.
struct ParticlePool {
    static constexpr std::int32_t capacity=0x700;
    std::array<std::uint32_t,capacity> entries{};
    std::int32_t count{},hint{};
    void extent()const {
        if(count<0||count>capacity)throw std::invalid_argument("Invalid particle pool count");
    }
    template<class Backend> bool insert(std::uint32_t token,Backend& backend){
        extent();
        if(!token)throw std::invalid_argument("Null particle ownership token");
        for(std::int32_t i=0;i<count;++i)if(entries[i]==token)throw std::invalid_argument("Duplicate particle ownership token");
        if(count<capacity){entries[count++]=token;return true;}
        auto priority=backend.priority(token);
        if(priority<=0x400||hint>=priority)return false;
        std::int32_t selected=-1;
        for(std::int32_t i=0;i<count;++i){
            const auto score=backend.priority(entries[i]);
            if(score<priority){selected=i;priority=score;}
        }
        if(selected<0){hint=priority;return false;}
        if(entries[selected])backend.destroy(entries[selected]);
        entries[selected]=token;
        // The original compares this cache with the selected INDEX minus one,
        // despite its use against priorities on the next insertion.
        if(hint<selected-1)hint=selected-1;
        return true;
    }
    template<class Backend> void clear(Backend& backend){
        extent();
        for(std::int32_t i=0;i<count;++i){
            if(entries[i])backend.destroy(entries[i]);
            entries[i]=0;
        }
        count=hint=0;
    }
    template<class Backend> void update(Backend& backend){
        extent();hint=0;
        // Iterate the original prefix backwards. Newly appended children are
        // not visited. A dying slot is reloaded after the callback, so nested
        // insertion/eviction can change which generation is destroyed.
        for(std::int32_t i=count-1;i>=0;--i){
            if(!entries[i])throw std::invalid_argument("Unowned active particle slot");
            const auto alive=backend.update(entries[i]);extent();
            if(i>=count)throw std::invalid_argument("Particle callback invalidated iteration extent");
            if(alive)continue;
            if(entries[i])backend.destroy(entries[i]);
            const auto last=count-1;
            if(i<last){count=last;entries[i]=entries[last];entries[last]=0;}
            else{entries[i]=0;--count;}
        }
    }
};
}

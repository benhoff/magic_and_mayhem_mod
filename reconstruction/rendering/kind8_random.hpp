#pragma once
#include <array>
#include <cstdint>

namespace mnm::reconstruction {
// Exact pinned 54e170 initialization and 54e200 transition on an owned R250
// state. The initializer's high-half/carry arithmetic is preserved explicitly.
struct Kind8Random {
    std::uint32_t first{},second{103},lcg{};
    std::array<std::uint32_t,250> words{};
    explicit Kind8Random(std::uint32_t seed) : lcg(seed) {
        for(unsigned n=250;n;--n) {
            const auto product=std::uint64_t(lcg)*0x41c64e6du;
            const auto low=std::uint32_t(product);
            lcg=low+0x3039u;
            const auto high=(std::uint32_t(product>>32)<<16)+0xffffu+std::uint32_t(lcg<low);
            words[n-1]=(lcg>>16)|(high&0xffff0000u);
        }
        for(unsigned i=0;i<32;++i)words[3+7*i]=(words[3+7*i]&(0xffffffffu>>i))|(0x80000000u>>i);
    }
    std::uint32_t next() {
        const auto a=first,b=second;first=(a+1)%250;second=(b+1)%250;
        return words[a]^=words[b];
    }
};
}

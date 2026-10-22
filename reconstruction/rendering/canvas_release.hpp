#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
// Recovered 58b3e0 call/registry semantics. Tokens are owned identities, never
// dereferenced pointers. Draining a borrowed COM reference is original behavior,
// not a recommendation for a native owner. Backends cannot reenter this state.
struct ReleaseCanvas { std::array<std::uint32_t,16> words{}; };
struct CanvasRegistry {
    std::array<std::uint32_t,128> entries{};
    std::uint32_t count{}, primary{};
    bool pageLocked{};
};
enum class ReleaseOperation { pageUnlock, release };
template<class Backend, class Resolve>
void releaseCanvas(std::uint32_t token, ReleaseCanvas& canvas,
        CanvasRegistry& registry, Backend&& backend,
        Resolve&& resolve) {
    if (!canvas.words[2]) return; // Original null-Surface2 exit does no cleanup.
    if (!canvas.words[1] || !registry.count || registry.count>128 ||
            canvas.words[7]>=registry.count || registry.entries[canvas.words[7]]!=token)
        throw std::invalid_argument("Malformed canvas release registry");
    for (unsigned i=0;i<registry.count;++i) {
        if (!registry.entries[i] || resolve(registry.entries[i]).words[7]!=i)
            throw std::invalid_argument("Unresolved canvas release registry owner");
        for (unsigned j=0;j<i;++j)
            if (registry.entries[i]==registry.entries[j])
                throw std::invalid_argument("Duplicate canvas registry owner");
    }
    if (registry.count<128 && registry.entries[registry.count])
        throw std::invalid_argument("Unterminated canvas release registry");
    if (registry.pageLocked) backend(ReleaseOperation::pageUnlock,canvas.words[2]);
    unsigned attempts=0;
    for (unsigned field:{1u,2u}) {
        std::uint32_t result;
        do { ++attempts; result=backend(ReleaseOperation::release,canvas.words[field]); }
        while(result && attempts<1000);
    }
    if (canvas.words[3]) {
        backend(ReleaseOperation::release,canvas.words[3]);canvas.words[3]=0;
    }
    if (registry.primary==token) registry.primary=0;
    while (canvas.words[7]<127) {
        const auto at=canvas.words[7];registry.entries[at]=registry.entries[at+1];
        if (!registry.entries[at]) break;
        resolve(registry.entries[at]).words[7]=at;
        ++canvas.words[7];
    }
    registry.entries[127]=0;--registry.count;
    registry.entries[canvas.words[7]]=0;
    for (unsigned field:{1u,2u,4u,5u,6u,10u})canvas.words[field]=0;
}
}

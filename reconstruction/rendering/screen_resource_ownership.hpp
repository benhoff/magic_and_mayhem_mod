#pragma once
#include "screen_lifetime.hpp"

namespace mnm::reconstruction {
// No-CD 40209ca7. These are the complete local slot+20 release callers of
// the 21 concrete shared-tick tables. Resource destructors and the CRT array
// iterator are explicit endpoints, not claims about their transitive semantics.
template<class Memory, class Services>
bool releaseOriginalScreenResources(Memory& m, Services& p,
                                    std::uint32_t table, std::uint32_t owner) {
    using U = std::uint32_t;
    auto field = [&](U offset) { return m.get(owner + offset); };
    auto clear = [&](U offset) { m.set(owner + offset, 0); };
    auto direct = [&](U offset, U destructor) {
        const U object = field(offset);
        if (!object) return;
        p.destruct(destructor, object);
        p.free(object); // The saved receiver survives destructor mutations.
        clear(offset);
    };
    auto callback = [&](U offset) {
        const U object = field(offset);
        if (object) { p.dispose(object, 1); clear(offset); }
    };
    auto array = [&](U offset, U count, U destructor) {
        for (U i = 0; i < count; ++i) {
            const U object = m.get(field(offset) + 4*i);
            if (!object) continue;
            if (destructor) { p.destruct(destructor, object); p.free(object); }
            else p.dispose(object, 1);
            // Both the next load and this clear reread the owning array field.
            m.set(field(offset) + 4*i, 0);
        }
    };
    auto cookie = [&](U offset, U stride, U destructor) {
        const U objects = field(offset);
        if (!objects) return;
        p.destroyArray(objects, stride, m.get(objects - 4), destructor);
        p.free(objects - 4);
        clear(offset);
    };
    auto surface = [&] {
        const U first = field(0x13);
        if (!first) return;
        p.destruct(0x58b3e0, first);
        const U second = field(0x13);
        if (second) { p.destruct(0x58b3e0, second); p.free(second); }
        clear(0x13);
    };
    auto vector = [&] {
        const U at = owner + 0x1f, begin = m.get(at+4), end = m.get(at+8);
        if (begin && ((end-begin) & 0xfffffffcu)) m.set(at+8, begin);
        m.set(at+16, 0); // 4cacb0 retains backing allocation, destroys no item.
    };
    auto ordinaryTail = [&] {
        cookie(0x3f, 0x141, 0x559880); callback(0x47); vector();
    };
    auto bitmap = [&](U offset) {
        const U at = owner + offset, bytes = m.get(at);
        if (bytes) {
            p.freeArray(bytes);
            for (U j=0;j<16;j+=4) m.set(at+j,0);
        }
        for (U j=16;j<32;j+=4) {
            const U value=m.get(at+j);
            if(value) { p.bitmapPlane(at,value); m.set(at+j,0); }
        }
        const U tail=m.get(at+32);
        if(tail) { p.freeArray(tail); m.set(at+32,0); }
    };
    if (table==0x5c6454) { callback(0x47); return true; }
    if (table==0x5c5ebc) {
        if(field(0x4b)) { array(0x4b,21,0x4d27d0); p.free(field(0x4b)); clear(0x4b); }
        if(field(0x47)) { array(0x47,1,0x475c10); p.free(field(0x47)); clear(0x47); }
        callback(0x4f); cookie(0x3f,0x141,0x559880); vector();
        for(U offset:{0x257u,0x25bu}) if(field(offset)) { p.free(field(offset)); clear(offset); }
        return true; // Report release is not gated by initialized+8.
    }
    if (table==0x5c648c) {
        callback(0x4b); callback(0x47);
        cookie(0x4f,0x29,0x4d27d0); cookie(0x53,0x86,0x4cdbc0);
        cookie(0x57,0x7e,0x4cea90); cookie(0x5b,0x59,0x475c10);
        cookie(0x63,0x10,0x4ce660); callback(0x5f);
        cookie(0x3f,0x141,0x559880); clear(8); return true;
    }
    if (table==0x5c66b4) {
        callback(0x6b); surface(); callback(0x6f);
        cookie(0x73,0x59,0x475c10); cookie(0x3f,0x141,0x559880);
        const bool labels=field(0x77)!=0;
        cookie(0x77,0x29,0x4d27d0); if(labels) clear(0x8b);
        cookie(0x7b,0x32,0x464310); cookie(0x7f,0x2c,0x4dd9a0);
        cookie(0x83,0x2c,0x4dd6b0); cookie(0x87,0x7a,0x4cb560);
        direct(0x11b,0x572400); bitmap(0x47); clear(8); return true;
    }
    if(table==0x5c67f8 || table==0x5c689c) {
        releaseOriginalScreenResources(m,p,0x5c66ec,owner);
        // Derived cleanup runs even when the base initialized guard skips.
        if(table==0x5c67f8) {
            direct(0xa0,0x475c10); callback(0xa4); direct(0xac,0x559880);
            direct(0x3b,0x4cefa0); callback(0xa8);
        } else {
            array(0xa8,13,0x4cdbc0); array(0xac,2,0x475c10);
            array(0xb0,3,0x4cb560); array(0xa4,3,0);
            cookie(0xb4,0x141,0x559880);
        }
        return true;
    }
    switch(table) {
    case 0x5c5ef4: case 0x5c6120: case 0x5c63e4: case 0x5c641c:
    case 0x5c64c4: case 0x5c64fc: case 0x5c6534: case 0x5c659c:
    case 0x5c65d4: case 0x5c660c: case 0x5c6644: case 0x5c667c:
    case 0x5c66ec: case 0x5c6940: case 0x5c6978: break;
    default: return false; // 5c6564 has a purecall slot, not a concrete no-op.
    }
    if (!field(8)) return true;
    clear(8); surface();
    switch(table) {
    case 0x5c5ef4:
        array(0x4f,26,0x4d27d0); array(0x57,3,0x475c10);
        callback(0x4b); array(0x53,4,0x4d2a30); break;
    case 0x5c6120:
        array(0x4b,1,0x475c10); array(0x4f,1,0); break;
    case 0x5c63e4:
        array(0x4b,6,0x475c10); direct(0x53,0x4d27d0); callback(0x4f); break;
    case 0x5c641c:
        array(0x4b,4,0x475c10); direct(0x4f,0x4d27d0); callback(0x53); break;
    case 0x5c64c4:
        array(0x4b,2,0x475c10); array(0x57,2,0x4d27d0);
        direct(0x4f,0x4ccd80); direct(0x5b,0x4d13b0);
        direct(0x3b,0x4cefa0); array(0x53,5,0); break;
    case 0x5c64fc:
        array(0x4b,3,0x475c10); array(0x5b,2,0x4d27d0);
        direct(0x4f,0x4ccd80); direct(0x57,0x4d13b0);
        array(0x53,6,0); direct(0x3b,0x4cefa0); break;
    case 0x5c6534:
        array(0x53,40,0x4d27d0); array(0x4f,3,0x475c10);
        array(0x57,4,0x4cb560); array(0x5f,17,0x4cdbc0);
        array(0x4b,3,0); array(0x5b,6,0x4d2a30);
        cookie(0x3f,0x141,0x559880); callback(0x47);
        p.destruct(0x58a0a0,owner+0x48b); vector(); return true;
    case 0x5c659c:
        array(0x4b,2,0x475c10); array(0x4f,3,0x4d27d0);
        array(0x53,3,0x4cea90); direct(0x5b,0x4ccd80); array(0x57,2,0);
        ordinaryTail(); p.destruct(0x4ce6d0,owner+0x5f); clear(8); return true;
    case 0x5c65d4:
        direct(0x57,0x4d27d0); direct(0x53,0x4d13b0);
        array(0x4b,2,0x475c10); array(0x4f,2,0); break;
    case 0x5c660c:
        array(0x4b,2,0x475c10); array(0x4f,4,0x4d27d0);
        array(0x57,3,0x4cea90); array(0x53,2,0x4ccd80); array(0x5b,2,0);
        ordinaryTail(); p.destruct(0x4ce6d0,owner+0x5f); return true;
    case 0x5c6644:
        for(U i=0;static_cast<std::int32_t>(i)<static_cast<std::int32_t>(field(0x57));++i) {
            const U object=m.get(field(0x4b)+4*i);
            if(object) { p.destruct(0x475c10,object); p.free(object); m.set(field(0x4b)+4*i,0); }
        }
        array(0x4f,2,0); cookie(0x3f,0x141,0x559880); callback(0x47);
        direct(0x3b,0x4cefa0); vector(); return true;
    case 0x5c667c:
        cookie(0x4b,0x59,0x475c10); cookie(0x4f,0x7a,0x4cb560);
        cookie(0x53,0x7e,0x4cea90); cookie(0x57,0x29,0x4d27d0);
        callback(0x5b); break;
    case 0x5c66ec:
        array(0x57,1,0x475c10); array(0x5b,40,0x4d27d0);
        array(0x5f,8,0x4cb560); direct(0x4f,0x4d1ff0); direct(0x53,0x4ccd80);
        array(0x4b,5,0); cookie(0x3f,0x141,0x559880); callback(0x47);
        direct(0x63,0x4cdbc0); vector(); return true;
    case 0x5c6940:
        array(0x4b,2,0x475c10); array(0x53,1,0x4d27d0);
        direct(0x57,0x4d13b0); array(0x4f,2,0); break;
    case 0x5c6978:
        array(0x4b,2,0x475c10); array(0x4f,1,0x4d27d0); array(0x53,1,0); break;
    }
    ordinaryTail(); return true;
}
// Normal-return construction and complete enclosing destruction for Main,
// Quick Battle and Mini. Allocation must supply admitted writable storage: the
// original constructors immediately zero their allocation, without a null test.
template<class Memory, class Services>
bool constructOriginalCoreMenu(Memory& m, Services& p,
                               std::uint32_t table, std::uint32_t owner) {
    using U=std::uint32_t;
    U id, count;
    if(table==0x5c63e4) { id=3;count=6; }
    else if(table==0x5c641c) { id=22;count=4; }
    else if(table==0x5c6644) { id=17;count=5; }
    else return false;
    m.set(owner+8,0);m.byte(owner+12,0);m.set(owner+13,0);m.byte(owner+17,1);
    m.set(owner,0x5c5df4);constructOriginalScreenVector(m,owner+0x1f);
    for(U offset:{0x13u,0x17u,0x1bu,0x33u,0x3bu,0x3fu,0x37u,0x43u})m.set(owner+offset,0);
    m.set(owner,table);m.set(owner+4,id);
    const U buttons=p.allocate(count*4);m.set(owner+0x4b,buttons);
    for(U i=0;i<count;++i)m.set(buttons+4*i,0);
    if(table==0x5c6644) {
        const U callbacks=p.allocate(8);m.set(owner+0x4f,callbacks);
        m.set(callbacks,0);m.set(callbacks+4,0);m.set(owner+0x47,0);m.byte(owner+0x11,0);
    } else {
        // These stores follow allocation and deliberately overwrite any endpoint
        // mutation to the fields. Main additionally clears its +5b marker.
        if(table==0x5c63e4) { m.set(owner+0x53,0);m.set(owner+0x4f,0); }
        else { m.set(owner+0x4f,0);m.set(owner+0x53,0); }
        m.set(owner+0x47,0);
        if(table==0x5c63e4)m.set(owner+0x5b,0);
    }
    return true;
}

template<class Memory, class Services>
bool destroyOriginalCoreMenu(Memory& m, Services& p,
                             std::uint32_t table, std::uint32_t owner) {
    if(table!=0x5c63e4 && table!=0x5c641c && table!=0x5c6644)return false;
    m.set(owner,table);
    releaseOriginalScreenResources(m,p,table,owner);
    for(std::uint32_t offset:{0x4bu,0x4fu}) {
        if(offset==0x4f && table!=0x5c6644)continue;
        const auto allocation=m.get(owner+offset);
        if(allocation) { p.free(allocation);m.set(owner+offset,0); }
    }
    destroyOriginalScreenVector(m,p,owner+0x1f);
    return true;
}

template<class Memory,class Services>
void retireOriginalScreenBitmap(Memory& m,Services& p,std::uint32_t object) {
    using U=std::uint32_t;
    if(const U pointer=m.get(object)) {
        p.freeArray(pointer);for(U i=0;i<16;i+=4)m.set(object+i,0);
    }
    for(U i=16;i<32;i+=4)if(const U plane=m.get(object+i)) {
        p.bitmapPlane(object,plane);m.set(object+i,0);
    }
    if(const U pointer=m.get(object+32)){p.freeArray(pointer);m.set(object+32,0);}
}

template<class Memory,class Services>
void exitOriginalCampaignReport(Memory& m,Services& p,std::uint32_t owner) {
    m.set(owner+8,0);
    if(const auto bitmap=m.get(owner+0x2a5)) {
        retireOriginalScreenBitmap(m,p,bitmap);p.free(bitmap);m.set(owner+0x2a5,0);
    }
    p.destruct(0x58b3e0,owner+0x25f);
    // Valid concrete report vtable; arbitrary virtual-table replacement remains
    // outside this concrete provider's admitted domain.
    releaseOriginalScreenResources(m,p,0x5c5ebc,owner);
    m.set(owner+0x43,1);m.set(owner+0x53,0);
}

template<class Memory,class Services>
bool destroyOriginalResultScreen(Memory& m,Services& p,
                                 std::uint32_t table,std::uint32_t owner) {
    if(table!=0x5c5ebc && table!=0x5c5ef4)return false;
    m.set(owner,table);
    if(table==0x5c5ebc) {
        if(m.get(owner+8))exitOriginalCampaignReport(m,p,owner);
        // The original enclosing destructor repeats the slot's cleanup even
        // after the exit path has performed it. The embedded surface is also
        // retired again unconditionally after the static report resource.
        releaseOriginalScreenResources(m,p,table,owner);
        p.destruct(0x58a0a0,0x5fcc80);p.destruct(0x58b3e0,owner+0x25f);
    } else {
        releaseOriginalScreenResources(m,p,table,owner);
        for(std::uint32_t offset:{0x4fu,0x53u,0x57u}) {
            const auto pointer=m.get(owner+offset);
            if(pointer){p.free(pointer);m.set(owner+offset,0);}
        }
        retireOriginalScreenBitmap(m,p,owner+0x5b);
    }
    destroyOriginalScreenVector(m,p,owner+0x1f);return true;
}
} // namespace mnm::reconstruction

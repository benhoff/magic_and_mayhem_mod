#pragma once
#include <cstdint>
#include <initializer_list>

namespace mnm::reconstruction {
// Complete local World exit 46ab30..46ae8c, No-CD 40209ca7. Services accept
// original wire receivers/arguments and return original DWORDs. Simulation,
// asset, allocator, DirectDraw and OS endpoints retain separate contracts.
template<class Memory,class Services>
void retireOriginalWorldScreen(Memory& m,Services& p,std::uint32_t owner) {
    using U=std::uint32_t;
    auto call=[&](U fn,U receiver=0,U arg=0){return p.call(fn,receiver,arg);};
    const U prior=m.get(0x5d9e38);
    if(prior!=0xffffffffu){m.set(0x6de6d9,prior);m.set(0x5d9e38,0xffffffffu);}
    if(const U object=m.get(owner+0x100df))call(0x49e860,object);
    if(!m.getByte(owner+0x100ac)) {
        m.byte(owner+0x100ac,1);call(0x4711d0,owner,1);
        call(0x4ffad0,0x689930);call(0x52baf0,0x6def58);
        U object=m.get(0x689498);
        for(U i=0;i<m.get(0x68949c);++i,object+=0x22e)
            if(m.get(object+4))call(0x4943f0,object);
    }
    const U returned=call(0x4a5000,0x689930);
    // Original SETE AL preserves EAX's upper 24 bits before PUSH EAX.
    call(0x56d0a0,0x6f37f8,(returned&0xffffff00u)|U(!m.getByte(owner+0x100a4)));
    m.set(0x6def60,0);m.set(0x6def64,m.get(0x6def58));
    for(U object=call(0x52a670,0x6def58);object;object=call(0x52a670,0x6def58))
        if(m.get(object+4))call(0x506d30,object);
    U object=m.get(0x6b1248);
    for(U i=0;i<m.get(0x6b124c);++i,object+=0x7e)
        if(m.get(object+4))call(0x543c60,object);
    m.set(0x6e1f94,0);m.set(0x6e1f98,0);call(0x535970,0x6de580);
    if(const U pointer=m.get(0x6e266c)){p.free(pointer);m.set(0x6e266c,0);}
    auto bitmap=[&](U at){
        if(const U bytes=m.get(at)){
            p.freeArray(bytes);for(U i=0;i<16;i+=4)m.set(at+i,0);
        }
        for(U i=16;i<32;i+=4)if(const U plane=m.get(at+i)){
            p.bitmapPlane(at,plane);m.set(at+i,0);
        }
        if(const U bytes=m.get(at+32)){p.freeArray(bytes);m.set(at+32,0);}
    };
    bitmap(0x6de558);call(0x4ff9a0,0x689930);call(0x464ad0,0x657ea8);
    bitmap(0x6a28f8);
    for(U fn:{0x549230u,0x5496b0u,0x549b20u,0x4879f0u})call(fn);
    call(0x5458b0,owner+0x12);
    // Original segmented pointer queue: advance one DWORD, and retire an
    // exhausted block at its boundary or when the last element is removed.
    while(m.get(owner+0xfc52)) {
        const U next=m.get(owner+0xfc32)+4;
        m.set(owner+0xfc32,next);m.set(owner+0xfc52,m.get(owner+0xfc52)-1);
        if(!m.get(owner+0xfc52)||next==m.get(owner+0xfc2e)) {
            const U slot=m.get(owner+0xfc36);
            m.set(owner+0xfc36,slot+4);p.free(m.get(slot));
            if(!m.get(owner+0xfc52)) {
                for(U offset:{0xfc2au,0xfc3au})for(U i=0;i<16;i+=4)m.set(owner+offset+i,0);
                p.free(m.get(owner+0xfc4a));
            } else {
                const U block=m.get(m.get(owner+0xfc36));
                m.set(owner+0xfc2a,block);m.set(owner+0xfc2e,block+0x1000);
                m.set(owner+0xfc32,block);
            }
        }
    }
    call(0x4ecbe0,0x6c5490);call(0x573960,0x6c4838);
    call(0x58bc10,0x6a49e0,0);call(0x58bc10,0x6a4fe0,0);
    m.set(0x6903e8,0);call(0x46a930,owner);m.set(owner+8,0);
    if(m.getByte(0x6e1f9c))m.set(0x656624,call(0x5c52e8));
}
} // namespace mnm::reconstruction

// Execute the original factory, Lock and both complete helpers without text patches.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <algorithm>
#include <array>

using U = std::uint32_t;
struct Interface { void** table; unsigned owner, version; };
struct Event { U site, owner, version, pointer, result; };
static std::array<void*, 40> firstTable{}, secondTable{}, factoryTable{};
static Interface interfaces[2][2], factory;
static std::array<U, 16> objects[2];
static std::vector<std::uint16_t> pixels[2];
static std::vector<Event> events;
static unsigned creating, pitch[2], results[2];
static U word(const void* p) { return reinterpret_cast<U>(p); }
static void check(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
static U __attribute__((stdcall, force_align_arg_pointer)) query(void* receiver, const void* iid, void** out) {
    auto* p = static_cast<Interface*>(receiver);
    const unsigned char expected[16] = {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};
    check(p == &interfaces[creating][0] && !std::memcmp(iid, expected, 16), "Surface2 QueryInterface identity");
    check(out == reinterpret_cast<void**>(objects[creating].data()+2), "Surface2 output is wrapper+8");
    *out = &interfaces[creating][1]; return 0;
}
static U __attribute__((stdcall, force_align_arg_pointer)) create(void* receiver, const U* desc, void** out, void* outer) {
    check(receiver == &factory && !outer && desc[0] == 108, "CreateSurface arguments");
    check(out == reinterpret_cast<void**>(objects[creating].data()+1), "Surface1 output is wrapper+4");
    *out = &interfaces[creating][0]; return 0;
}
static U __attribute__((stdcall, force_align_arg_pointer)) describe(void* receiver, U* desc) {
    check(receiver == &interfaces[creating][0] && desc[0] == 108 && desc[2] && desc[3], "Surface1 GetSurfaceDesc boundary");
    return 0;
}
static U __attribute__((stdcall, force_align_arg_pointer)) lockSurface(void* receiver, void* rect, U* desc, U flags, void* wait) {
    auto* p = static_cast<Interface*>(receiver);
    check(p->version == 2 && p == &interfaces[p->owner][1] && !rect && flags == 1 && !wait, "Lock must use Surface2");
    check(desc == reinterpret_cast<U*>(0x6f68e0) && desc[0] == 108, "Lock descriptor");
    for (unsigned i=1; i<27; ++i) check(desc[i] == 0, "Lock clears descriptor");
    desc[4] = pitch[p->owner]; desc[9] = word(pixels[p->owner].data()+32);
    events.push_back({word(__builtin_return_address(0)),p->owner,2,desc[9],0}); return 0;
}
static U __attribute__((stdcall, force_align_arg_pointer)) unlockSurface(void* receiver, void* data) {
    auto* p = static_cast<Interface*>(receiver);
    check(p->version == 1 && p == &interfaces[p->owner][0], "Unlock must use Surface1");
    check(data == pixels[p->owner].data()+32, "Unlock uses original Lock base pointer");
    events.push_back({word(__builtin_return_address(0)),p->owner,1,word(data),results[p->owner]});
    return results[p->owner];
}
int main(int argc, char** argv) try {
    check(argc==2, "Expected pinned original image"); map_image(read(argv[1]));
    const std::array<std::pair<U,U>,5> ranges{{{0x58ad90,0x58b263},{0x58b660,0x58b815},
        {0x58d3b0,0x58d4a7},{0x58d770,0x58d8b6},{0x59bee0,0x59bf07}}};
    std::vector<Bytes> original;
    for (auto r:ranges) original.emplace_back(reinterpret_cast<unsigned char*>(r.first),reinterpret_cast<unsigned char*>(r.second));
    firstTable[0]=reinterpret_cast<void*>(query); firstTable[22]=reinterpret_cast<void*>(describe); firstTable[32]=reinterpret_cast<void*>(unlockSurface);
    secondTable[25]=reinterpret_cast<void*>(lockSurface); factoryTable[6]=reinterpret_cast<void*>(create);
    factory={factoryTable.data(),0,0};
    for (unsigned i=0;i<2;++i) {
        interfaces[i][0]={firstTable.data(),i,1}; interfaces[i][1]={secondTable.data(),i,2};
        pixels[i].resize(4096);
    }
    U cases=0, factories=0, dispatches=0, pixelWords=0;
    for (U kind:{0u,1u}) for (U format:{0u,1u}) for (U early:{0u,1u})
    for (U w:{0u,1u,3u,7u}) for (U h:{0u,1u,3u,5u})
    for (U sourceResult:{0u,0x80004005u}) for (U destResult:{0u,0x88760248u}) {
        for(unsigned i=0;i<2;++i) {
            objects[i].fill(0); creating=i;
            U desc[27]={}; desc[0]=108; desc[1]=0x1007; desc[2]=h?h:1; desc[3]=w?w:1; desc[26]=0x840;
            global(0x6f98c8,word(&factory)); global(0x6f8154,0); global(0x6e1f1c,0); global(0x6de6dd,0);
            using Create=void(__attribute__((thiscall))*)(void*,void*,U,U);
            reinterpret_cast<Create>(0x58ad90)(objects[i].data(),desc,0,0); ++factories;
            check(objects[i][1]==word(&interfaces[i][0]) && objects[i][2]==word(&interfaces[i][1]), "Distinct original interface fields");
            check(objects[i][4]==(w?w:1) && objects[i][5]==(h?h:1), "Factory dimensions");
            objects[i][4]=w; objects[i][5]=h; // Explicit zero-sized draw controls after valid creation.
            pitch[i]=16+4*i;
            for(unsigned j=0;j<pixels[i].size();++j) pixels[i][j]=static_cast<std::uint16_t>(j*317+cases*997+i*593);
        }
        const auto sourceBefore=pixels[0]; auto expected=pixels[1];
        auto sourceExpected=sourceBefore;
        if(kind==0) {
            // The original loop uses the byte-pitch field as a WORD stride. Preserve it.
            for(U y=0;y<h;++y) for(U x=0;x<w;++x) expected[32+y*pitch[1]+x]=sourceBefore[32+y*pitch[0]+x];
        } else {
            const U shift=format?10:11, greenMask=format?0x3e0:0x7e0, greenMax=format?31:63, greenAdd=format?16:32;
            for(U j=0;j<w*h;++j) {
                const U p=sourceBefore[32+j];
                sourceExpected[32+j]=static_cast<std::uint16_t>((std::min(31u,((p>>shift)&31)/2+16)<<shift)
                    | (std::min(greenMax,((p>>6)&(greenMask>>6))+greenAdd)<<5)
                    | std::min(31u,((p&255)>>1 & 15)+16));
            }
        }
        results[0]=sourceResult; results[1]=destResult; events.clear();
        global(0x6e1f88,format); global(0x6e1f1c,early);
        U returned;
        if(kind==0) {
            U rect[4]={123,456,w,h};
            using Copy=U(__attribute__((thiscall))*)(void*,void*,void*,U);
            returned=reinterpret_cast<Copy>(0x58d3b0)(objects[0].data(),objects[1].data(),rect,0x12345678);
        } else {
            using Brighten=U(__attribute__((thiscall))*)(void*);
            returned=reinterpret_cast<Brighten>(0x58d770)(objects[0].data());
        }
        check(returned==(kind?sourceResult:destResult), "Final Unlock HRESULT survives as EAX");
        check(pixels[0]==sourceExpected && pixels[1]==expected, "Complete pixels and guards");
        std::vector<Event> want;
        auto add=[&](U site,U owner,U version,U result) {want.push_back({site,owner,version,word(pixels[owner].data()+32),result});};
        add(0x58b6a0,0,2,0); if(early) add(0x58b80c,0,1,sourceResult);
        if(!kind) {add(0x58b6a0,1,2,0); if(early) add(0x58b80c,1,1,destResult); add(0x58d48d,0,1,sourceResult); add(0x58d49a,1,1,destResult);}
        else add(0x58d8ae,0,1,sourceResult);
        check(events.size()==want.size(), "Exact service count including zero-size draws");
        for(unsigned i=0;i<events.size();++i) {
            const auto &a=events[i], &b=want[i];
            check(a.site==b.site && a.owner==b.owner && a.version==b.version && a.pointer==b.pointer && a.result==b.result, "Exact caller/receiver/arguments/result/order");
        }
        for(unsigned i=0;i<ranges.size();++i) check(!std::memcmp(original[i].data(),reinterpret_cast<void*>(ranges[i].first),original[i].size()), "Original text unchanged");
        dispatches+=kind?1:2; pixelWords+=8192; ++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"factories\":"<<factories<<",\"target_dispatches\":"<<dispatches<<",\"physical_words\":"<<pixelWords<<",\"original_text_unchanged\":true}\n";
    return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n'; return 1;}

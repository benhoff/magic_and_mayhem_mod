// Reuse the established value-address storage and provider ABIs without changing
// its historical fixture or routing. This suite supplies its own owned session,
// allocator, complete snapshots and original callback bridge.
#define main unused_actor_services_reference_main
#include "actor-services-reference.cpp"
#undef main
#include "../reconstruction/rendering/actor_session.hpp"
#include <map>

namespace {
constexpr U Heap = Base + 0x40000, HeapLimit = Base + 0xf0000;
constexpr std::array<std::array<U, 2>, 9> SessionGlobals{{
    {{0x690600, 0x12000}}, {{0x644550, 0x12000}}, {{0x5d854c, 4}},
    {{0x5d865c, 4}}, {{0x656634, 4}}, {{0x6e1ac8, 1008}},
    {{0x5c7264, 8}}, {{0x6e9218, 1008}}, {{0x6a29d8, 1008}}
}};
struct SessionMemory : Memory {
    struct Block { U at, size; bool alive; };
    std::vector<Block> blocks;
    std::map<U, U> originalHints;
    U cursor = Heap + 32, allocations = 0, failAllocation = 0, frames = 0;
    SessionMemory() { std::vector<unsigned char> space(0x100000); owned.swap(space); }
    U hash() {
        U value = 2166136261u;
        auto add = [&](U at, U size, bool node = false) {
            for (U i = 0; i < size; ++i) value = (value ^ (node && i == 0x25 ? 0 : get8(at + i))) * 16777619u;
        };
        for (auto range : SessionGlobals) add(range[0], range[1]);
        for (const auto& block : blocks) { value = (value ^ block.alive) * 16777619u; add(block.at, block.size, block.size == 0x8d); }
        return value;
    }
    U allocate(U size, U originalHint = ~0u) {
        ++allocations;
        if (allocations == failAllocation) return 0;
        U aligned = (size + 15) & ~15u;
        if ((cursor & 0xffff) < 32) cursor = (cursor & ~65535u) + 32;
        if ((cursor & 0xffff) + aligned > 65536) cursor = (cursor + 65535) & ~65535u, cursor += 32;
        if (cursor + aligned > HeapLimit) throw std::runtime_error("Session heap extent");
        if ((cursor & 0xffff) < 32 || (cursor & 0xffff) + aligned > 65536)
            throw std::runtime_error("Session allocation overlaps a heap header");
        U at = cursor; cursor += aligned;
        U page = at & ~65535u;
        set32(page + 8, 0x1001f200); set16(page + 14, 0xcad0);
        std::memset(pointer(at, size), 0, size); blocks.push_back({at, size, true});
        if (size == 0x8d && originalHint != ~0u) originalHints.emplace(at, originalHint);
        return at;
    }
    void free(U address) {
        if (!address) return;
        for (auto& block : blocks) if (block.at == address && block.alive) { block.alive = false; return; }
        throw std::runtime_error("Session free requires one live allocation");
    }
    void verifyHints() {
#ifdef MNM_ORIGINAL_REFERENCE
        for (auto [node, value] : originalHints)
            if ((get32(node + 0x41) | get32(node + 0x45)) && get8(node + 0x25) != value)
                throw std::runtime_error("Original source byte25 machine provenance");
#endif
    }
    U external(U id, U self, U second, std::initializer_list<U> args, U hint = ~0u) {
        events.insert(events.end(), {0xface0010, hash()});
        U a[12]{}; std::copy(args.begin(), args.end(), a);
        if (id == 0x598a10 || id == 0x1001f200 || id == 0x597df0) {
            events.insert(events.end(), {id, self, second, U(args.size())});
            events.insert(events.end(), args.begin(), args.end()); digest();
            if (id == 0x598a10) return allocate(a[1], hint);
            if (id == 0x1001f200) free(self);
            return 0;
        }
        for (U kind = 1; kind <= 16; ++kind) if (id && id == s::behaviorCallback(kind)) {
            events.insert(events.end(), {id, self, second, U(args.size())}); digest();
            return 0;
        }
        return Memory::external(id, self, second, args);
    }
    U invokeFast(U entry, U self, U second) { return dispatch(entry, self, {}, second); }
    U fastcall(U entry, U self, U second, std::initializer_list<U> args) { return dispatch(entry, self, args, second); }
    U invoke(U entry, U self, std::initializer_list<U> args) { return dispatch(entry, self, args); }
    U com(U id, U buffer, std::initializer_list<U> args) {
        return args.size() ? external(id, 0, 0, {buffer, *args.begin()}) : external(id, 0, 0, {buffer});
    }
    U dispatch(U entry, U self, std::initializer_list<U> args, U second = 0) {
#ifndef MNM_ORIGINAL_REFERENCE
        if (entry == 0x520790 && args.size() == 1 && *args.begin() == 0x20000000)
            return invoke(entry, self, {temporary(20)});
#endif
#ifdef MNM_ORIGINAL_REFERENCE
        U values[12]{}, i = 0; for (U value : args) values[i++] = value;
        for (const auto& abi : SessionAbis) if (abi.id == entry)
            return actor_services_call(entry, self, second, U(args.size()), values, abi.pop);
        throw std::runtime_error("Missing original session ABI: " + std::to_string(entry));
#else
        if (auto result = s::SessionComposition<SessionMemory>(*this).tryRun(entry, self, args, second)) {
            if (std::find(completedServices.begin(), completedServices.end(), entry) == completedServices.end()) completedServices.push_back(entry);
            return *result;
        }
        if (entry == 0x59cb60) {
            auto it = args.begin(); U dst = *it++, src = *it++, count = *it;
            std::memmove(pointer(dst, count), pointer(src, count), count); return dst;
        }
        return external(entry, self, second, args);
#endif
    }
    void initialize(U seed) {
        reset(seed);
        for (auto range : SessionGlobals) std::memset(pointer(range[0], range[1]), 0, range[1]);
        for (U source : {0x690600u, 0x644550u}) set32(source, source == 0x690600 ? 0 : 1);
        set32(0x5d854c, 1); set32(0x5d865c, seed % 3); set32(0x656634, seed % 8);
        set32(0x6e1f0c, 1); set32(0x6f37fc, 0); set32(0x689920, 0);
        set32(0x644520, 7); set32(0x6898dc, 0); set32(0x5fed7c, 0);
        set32(0x6def58, 0); set32(0x6df180, 0); set32(0x6def5c, 0);
        set32(0x6def5c + 0x94, 0); // No pre-existing manager list before construction.
        set32(0x5f2094, Other); set32(0x5f1ec8, 0); set8(0x5f20e8, 0);
        set32(0x5c7268, 0x52cbe0);
        for (U owner = 0; owner != 8; ++owner) {
            U army = 0x643a80 + owner * 0x154;
            set32(army, 1); set32(army + 4, 0); set32(army + 8, 0);
            set32(army + 12, 0); set32(army + 16, 0);
        }
        for (U receiver : {0x6574c8u, 0x6e1ac8u, 0x6a29d8u}) {
            set32(receiver, seed % 250); set32(receiver + 4, (seed + 103) % 250);
            for (U i = 0; i < 250; ++i) { set32(0x5eb940 + i*4, (i+1)%250); set32(receiver + 8 + i*4, seed*12345u + i*712397u + 17u); }
        }
        constexpr U Map = Base + 0x16000;
        set32(0x6c5494, 6); set32(0x6c5498, 5); set32(0x6c54a0, 30); set32(0x6c54dc, Map);
        set32(0x6a49c0, 0); set32(0x65660c, Other + 0x1200);
        for (U i = 0; i < 5; ++i) set32(0x6cb942 + i*4, i*6);
        set32(0x6cb8be, U(-30)); for (U i = 0; i < 4; ++i) set32(0x6cb8c2 + i*4, i*30);
        for (U i = 0; i < 120; ++i) { U at = Map + i*12; set16(at, 0); for (U off : {2u, 4u, 6u}) set16(at+off, 0xffff); set32(at+8, 0); }
        set32(Other + 0x1200 + 0x94, 0); set32(Other + 0x1200 + 0xc8, 0);
        for (U type : {0u, 17u, 22u, 24u, 25u, 26u, 27u}) {
            U definition = 0x6a5f80 + type*0x5c9;
            std::memset(pointer(definition, 0x5c9), 0, 0x5c9);
            set32(definition, type); set32(definition+8, 1); set32(definition+12, 1);
            set32(definition+24, 100); set32(definition+0x74, ~0u);
            U descriptor = 0x6dbf00 + type*0x114;
            std::memset(pointer(descriptor, 0x114), 0, 0x114);
        }
    }
    void importQueueBacking(U actor, U layout) {
        U root = get32(actor + 0x678);
        if (!root) throw std::runtime_error("Queue import requires an independently registered source");
        for (U offset : {0x2au, 0x5au, 0x8au}) {
            U queue = 0x690600 + offset;
            while (get32(queue + 44)) invoke(0x429900, queue, {});
            U blockCount = layout >= 2 ? 2 : 1;
            U capacity = layout == 3 ? 3 : 2, firstSlot = layout == 0 || layout == 3 ? 1 : 0;
            U map = invoke(0x597850, 0, {capacity*4});
            std::vector<U> blocks;
            for(U i=0;i<blockCount;++i) {
                U block=invoke(0x597850,0,{4096});blocks.push_back(block);set32(map+(firstSlot+i)*4,block);
                U count=i+1==blockCount?3:1024;
                for(U k=0;k<count;++k)set32(block+k*4,root);
            }
            set32(queue+4,blocks.front());set32(queue+8,blocks.front()+4096);set32(queue+12,blocks.front());set32(queue+16,map+firstSlot*4);
            set32(queue+20,blocks.back());set32(queue+24,blocks.back()+4096);set32(queue+28,blocks.back()+12);set32(queue+32,map+(firstSlot+blockCount-1)*4);
            set32(queue+36,map);set32(queue+40,capacity);set32(queue+44,(blockCount-1)*1024+3);
        }
        std::fprintf(stderr,"imported_queue_layout %u %u\n",variant,layout);
    }
    // Task creation/cadence is an explicitly supplied provider input. The real
    // grouping/pruning entries must preserve and transfer both ownership links.
    U bindRecipient(U node) {
        if (!node || get32(node+5)) throw std::runtime_error("Recipient input requires an unowned source");
        U task=invoke(0x597850,0,{48});set32(task+8,node);set32(node+5,task);return task;
    }
    void releaseRecipient(U task) {
        U node=get32(task+8);
        if (!node || get32(node+5)!=task) throw std::runtime_error("Recipient backlink lost");
        set32(node+5,0);set32(task+8,0);invoke(0x597870,0,{task});
    }
    void snapshotSession(U stage, U outcome = 0) {
        verifyHints();
        auto emit = [](const void* data, std::size_t size) {
            auto* bytes = static_cast<const unsigned char*>(data);
            while (size) { ssize_t count = write(STDOUT_FILENO, bytes, size); if (count < 0 && errno == EINTR) continue; if (count <= 0) _exit(89); bytes += count; size -= std::size_t(count); }
        };
        U header[] = {0x53534331, variant, stage, outcome, U(blocks.size()), U(events.size())}; emit(header, sizeof(header));
        emit(pointer(Base, 0x18000), 0x18000);
        for (auto range : Globals) emit(pointer(range[0], range[1]), range[1]);
        for (auto range : SessionGlobals) emit(pointer(range[0], range[1]), range[1]);
        for (const auto& block : blocks) {
            U fields[] = {block.at, block.size, U(block.alive)}; emit(fields, sizeof(fields));
            if (block.size == 0x8d) { U tag = 0; emit(pointer(block.at, 0x25), 0x25); emit(&tag, 1); emit(pointer(block.at + 0x26, block.size - 0x26), block.size - 0x26); }
            else emit(pointer(block.at, block.size), block.size);
        }
        emit(events.data(), events.size()*4); events.clear(); ++frames;
    }
};
static SessionMemory* sessionActive;
static U sessionStage;

#ifdef MNM_ORIGINAL_REFERENCE
extern "C" __attribute__((force_align_arg_pointer)) U session_provider(U id, U self, U* args, U second) {
    try {
        const ServiceProvider* spec = nullptr;
        for (const auto& s : ServiceProviders) if (s.id == id) spec = &s;
        if (!spec) _exit(84);
        if (!spec->receiver) self = 0;
        if (!spec->second) second = 0;
        U hint = ~0u;
        if (id == 0x598a10 && args[1] == 0x8d) {
            for (U i = 0; i < 128; ++i) if ((args[i] == 0x416847 || args[i] == 0x41691c || args[i] == 0x416ad0) && args[i+1] == 0x8d) {
                hint = args[i+14] & 255; break;
            }
            if (hint == ~0u) throw std::runtime_error("Source allocation stack anchor missing");
        }
        switch (spec->arguments) {
        case 0: return sessionActive->external(id,self,second,{},hint);
        case 1: return sessionActive->external(id,self,second,{args[0]},hint);
        case 2: return sessionActive->external(id,self,second,{args[0],args[1]},hint);
        case 3: return sessionActive->external(id,self,second,{args[0],args[1],args[2]},hint);
        case 4: return sessionActive->external(id,self,second,{args[0],args[1],args[2],args[3]},hint);
        case 5: return sessionActive->external(id,self,second,{args[0],args[1],args[2],args[3],args[4]},hint);
        case 6: return sessionActive->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5]},hint);
        case 7: return sessionActive->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5],args[6]},hint);
        case 8: return sessionActive->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]},hint);
        case 11: return sessionActive->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]},hint);
        default: throw std::runtime_error("Session provider arity");
        }
    } catch (const std::exception& error) { std::fprintf(stderr,"session provider %x: %s\n",id,error.what()); _exit(86); }
}
void installSessionProviders() {
    std::vector<std::array<U,2>> ranges; U index = 0;
    for (const auto& spec : ServiceProviders) {
        U at = spec.id >= Base ? spec.id : Code + index*32; ++index;
        if (at < Base+0x1f000 || at+26 > Base+0x24000) throw std::runtime_error("Session trampoline extent");
        for (auto range : ranges) if (at < range[1] && range[0] < at+26) throw std::runtime_error("Session trampoline overlap");
        ranges.push_back({at,at+26});
    }
    index = 0;
    for (const auto& spec : ServiceProviders) {
        U at = spec.id >= Base ? spec.id : Code + index*32; ++index;
        auto* p = reinterpret_cast<unsigned char*>(at);
        const unsigned char prefix[] = {0x55,0x89,0xe5,0x52,0x8d,0x45,0x08,0x50,0x51,0x68};
        std::memcpy(p,prefix,sizeof(prefix)); std::memcpy(p+10,&spec.id,4);
        p[14]=0xe8; U relative=U(reinterpret_cast<std::uintptr_t>(session_provider))-at-19; std::memcpy(p+15,&relative,4);
        const unsigned char tail[]={0x83,0xc4,0x10,0xc9,0xc2}; std::memcpy(p+19,tail,sizeof(tail));
        std::uint16_t pop=std::uint16_t(spec.pop); std::memcpy(p+24,&pop,2);
        if (spec.id < Base) {
            auto* target=reinterpret_cast<unsigned char*>(spec.id);
            if (std::memcmp(target,spec.bytes,8)) throw std::runtime_error("Session provider original entry guard");
            if (spec.id>=0x5c0000) std::memcpy(target,&at,4);
            else { target[0]=0xe9; relative=at-spec.id-5; std::memcpy(target+1,&relative,4); }
        }
    }
}
#endif
void sessionFault(int signal, siginfo_t*, void*) {
    if (sessionActive && (signal == SIGSEGV || signal == SIGFPE)) {
#ifdef MNM_ORIGINAL_REFERENCE
        if (!bodiesUnchanged()) _exit(87);
#endif
        sessionActive->snapshotSession(sessionStage,signal==SIGFPE?2:1); _exit(0);
    }
    _exit(128+signal);
}
} // namespace

int main(int argc, char** argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
    if (argc != 2) throw std::runtime_error("Expected pinned original");
    map_image(read(argv[1])); OriginalLinuxSeh seh;
    for(U i=0;i<250;++i)if(*reinterpret_cast<const U*>(0x5eb940+i*4)!=(i+1)%250)throw std::runtime_error("Original R250 successor data differs");
    if(*reinterpret_cast<const U*>(0x5c7268)!=0x52cbe0)throw std::runtime_error("Original actor callback deleting destructor differs");
    if (mmap(reinterpret_cast<void*>(Base),0x100000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED) throw std::runtime_error("Session owned map");
    installSessionProviders(); if (!bodiesUnchanged()) throw std::runtime_error("Session installation changed a compared body");
#else
    (void)argc; (void)argv;
#endif
    const U variants=std::getenv("MNM_SESSION_VARIANTS")?U(std::strtoul(std::getenv("MNM_SESSION_VARIANTS"),nullptr,10)):64;
    if(!variants||variants>64)throw std::runtime_error("Session variant extent");
    for (U seed=0; seed<variants; ++seed) {
        std::cout.flush(); pid_t child=fork(); if(child<0)throw std::runtime_error("Session fork");
        if(child){int status=0;if(waitpid(child,&status,0)!=child||!WIFEXITED(status)||WEXITSTATUS(status))throw std::runtime_error("Session case process failed");continue;}
        SessionMemory memory; sessionActive=&memory; active=&memory; memory.initialize(seed);
        struct sigaction action{};action.sa_sigaction=sessionFault;action.sa_flags=SA_SIGINFO;sigemptyset(&action.sa_mask);
        for(int signal:{SIGSEGV,SIGFPE,SIGILL,SIGABRT})sigaction(signal,&action,nullptr);
        s::ActorSession<SessionMemory> session(memory);
        auto checkpoint=[&](U stage){sessionStage=stage;memory.snapshotSession(stage);
#ifdef MNM_ORIGINAL_REFERENCE
            seh.check();if(!bodiesUnchanged())throw std::runtime_error("Original session body changed");
#endif
        };
        sessionStage=0;
        try {
            checkpoint(0); session.begin(4); checkpoint(1);
            U a=session.admit(seed&16?0:17,seed%7,1,1,1,true);checkpoint(2);
            if(seed&8)session.registerSourceFamily(a,1);
            // Authored owner/planner availability is an input notification.
            // The original planner cadence is not supplied by this session.
            if(seed&4)for(U offset:{0x678u,0x67cu})if(U node=memory.get32(a+offset))memory.set32(node+0x89,3);
            if(seed&32)memory.importQueueBacking(a,seed%4);
            U recipient=seed&16?memory.bindRecipient(memory.get32(a+0x678)):0;
            U recipientBefore=recipient?memory.get32(recipient+8):0;
            checkpoint(3);
            U b=session.admit(17,seed%7,3,1,1,true);
            if(seed&8)session.registerSourceFamily(b,1);
            for(U family=0;family<(seed&8?2u:1u);++family){
                U root=memory.invoke(0x5183f0,b,{family}),manager=family?0x644550:0x690600;
                memory.invoke(0x416fe0,manager,{root});memory.invoke(0x416fe0,manager,{root});
            }
            if(recipient){
                U root=memory.get32(recipient+8);
                if(memory.get32(root+5)!=recipient)throw std::runtime_error("Group recipient backlink lost");
                if(root!=recipientBefore){
                    if(memory.get32(recipientBefore+5))throw std::runtime_error("Group retained the transferred recipient");
                    std::fprintf(stderr,"recipient_transferred %u\n",seed);
                }
            }
            checkpoint(4);
            session.retire(b,seed&1);checkpoint(5);
            if(recipient){
                if(memory.invoke(0x5183f0,b,{0})==memory.get32(recipient+8)){
                    U allocations=memory.allocations;bool refused=false;
                    try{session.admit(17,seed%7,2,2,1,true);}catch(const std::logic_error&){refused=true;}
                    if(!refused||memory.allocations!=allocations)throw std::runtime_error("Recipient ancestor did not retain reusable actor");
                    std::fprintf(stderr,"recipient_reuse_refused %u\n",seed);
                }
                memory.releaseRecipient(recipient);
            }
            session.collectSources();checkpoint(6);
            U c=session.admit(17,seed%7,4,2,1,true);if(c!=b)throw std::runtime_error("Freed actor slot was not reused");
            if(seed&8)session.registerSourceFamily(c,1);
            checkpoint(7);
            U protectedRoot=memory.get32(a+0x678);if(!protectedRoot)throw std::runtime_error("Expected primary actor source");
            recipient=seed&16?memory.bindRecipient(memory.invoke(0x5183f0,a,{0})):0;
            memory.set32(0x690600+0x11daf,protectedRoot);
            session.retire(a,seed&2);checkpoint(8);
            bool refused=false;try{session.close();}catch(const std::logic_error&){refused=true;}
            if(!refused||!session.constructed()||!memory.get32(0x6def58))throw std::runtime_error("Protected source did not retain its actor pool");
            checkpoint(9);
            U allocations=memory.allocations;refused=false;
            try{session.admit(17,seed%7,2,2,1,true);}catch(const std::logic_error&){refused=true;}
            if(!refused||memory.allocations!=allocations)throw std::runtime_error("Source-retained slot was reused");
            checkpoint(10);
            memory.set32(0x690600+0x11daf,0);
            for(U offset:{0x678u,0x67cu})if(U node=memory.get32(a+offset))memory.set32(node+0x89,memory.get32(node+0x89)&1);
            if(recipient){
                refused=false;try{session.close();}catch(const std::logic_error&){refused=true;}
                if(!refused||!session.constructed()||!memory.get32(0x6def58))throw std::runtime_error("Recipient alone did not retain actor pool");
                std::fprintf(stderr,"recipient_close_refused %u\n",seed);
                memory.releaseRecipient(recipient);
            }
            checkpoint(11);session.close();checkpoint(12);
            session.begin(2);checkpoint(13);session.admit(17,seed%7,2,3,1,true);checkpoint(14);session.close();checkpoint(15);
            if(std::any_of(memory.blocks.begin(),memory.blocks.end(),[](const auto& block){return block.alive;}))throw std::runtime_error("Session teardown leaked an allocation");
            for(const auto& block:memory.blocks)if(block.size==0x8d){
#ifdef MNM_ORIGINAL_REFERENCE
                std::fprintf(stderr,"original_source_tag %u %x %u expected %u\n",seed,block.at,memory.get8(block.at+0x25),memory.originalHints.at(block.at));
#else
                std::fprintf(stderr,"native_source_tag %u %x %u\n",seed,block.at,memory.get8(block.at+0x25));
#endif
            }
#ifndef MNM_ORIGINAL_REFERENCE
            std::fprintf(stderr,"completed_session %u",seed);for(U entry:memory.completedServices)std::fprintf(stderr," %x",entry);std::fprintf(stderr,"\n");
#endif
            std::fprintf(stderr,"session_returned %u frames %u allocations %u live %zu\n",seed,memory.frames,memory.allocations,std::count_if(memory.blocks.begin(),memory.blocks.end(),[](const auto& b){return b.alive;}));
        } catch(const MemoryFault&) {memory.snapshotSession(sessionStage,1);}
          catch(const std::domain_error&) {memory.snapshotSession(sessionStage,2);}
        _exit(0);
    }
    std::fprintf(stderr,"sessions %u\n",variants);return 0;
}catch(const std::exception& error){std::fprintf(stderr,"%s\n",error.what());return 1;}

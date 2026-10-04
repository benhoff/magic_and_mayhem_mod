#include "save_world.hpp"
#include "persistence_internal.hpp"
#include <limits>
namespace mnm::assets {
namespace {
using namespace persistence_detail;
class Reader {
public:
    Cursor c; const WorldLimits& limits; SavedWorld result;
    std::optional<std::uint32_t> parent;
    std::uint64_t used=0;
    Reader(const Bytes& b,const WorldLimits& l):c{b},limits(l) {
        if(b.size()>l.inputBytes) fail(PersistenceErrorCode::limitExceeded,0,"World input limit exceeded");
        charge(b.size());
    }
    void charge(std::uint64_t n) {if(n>limits.decodedBytes-used) fail(PersistenceErrorCode::limitExceeded,c.p,"World decoded budget exceeded");used+=n;}
    void skip(std::uint64_t n) {if(n>c.b.size()-c.p) fail(PersistenceErrorCode::malformedData,c.p,"Truncated world block");c.p+=static_cast<std::size_t>(n);}
    std::uint32_t word() {return c.u32();}
    std::uint32_t count(std::uint32_t bound=0) {
        const auto n=word(); if(n>(bound?std::min(bound,limits.records):limits.records)) fail(PersistenceErrorCode::limitExceeded,c.p-4,"World count exceeds limit"); return n;
    }
    template<class F> void block(const char* name,F action) {
        if(result.blocks.size()>=limits.blocks) fail(PersistenceErrorCode::limitExceeded,c.p,"World block limit exceeded");
        charge(sizeof(WorldBlock)+std::char_traits<char>::length(name));
        const auto index=static_cast<std::uint32_t>(result.blocks.size());
        result.blocks.push_back({name,c.p,0,parent});auto previous=parent;parent=index;
        action();result.blocks[index].size=c.p-result.blocks[index].offset;parent=previous;
    }
    void fixed(const char* name,std::uint64_t bytes) {block(name,[&]{skip(bytes);});}
    void animation() {block("animation",[&]{if(word()!=0xffffffffU) skip(12);});}
    void array(const char* name,std::uint32_t stride) {block(name,[&]{const auto n=count();skip(std::uint64_t(n)*stride);});}
    void stateList(const char* name,std::uint32_t stride) {block(name,[&]{skip(16);const auto n=count();skip(std::uint64_t(n)*stride);});}
    void map() {
        block("map",[&]{const auto header=c.p;fixed("descriptor",0x4c);const auto cells=persistence_detail::word(c.b,header+0x14);
            if(cells>limits.records) fail(PersistenceErrorCode::limitExceeded,header+0x14,"World cell count exceeds limit");
            fixed("cells",std::uint64_t(cells)*12);skip(4);stateList("map-list-40-a",40);stateList("map-list-40-b",40);
            fixed("map-arrays",(75+120+120+28+123)*4);array("map-byte-state",1);
            const auto n=count();for(std::uint32_t i=0;i<n;++i) block("map-animation",[&]{skip(4);animation();});
            skip(20);stateList("map-list-72",72);
        });
    }
    void creature();
    void missile() {
        block("missile",[&]{skip(4);if(!word()) return;skip(8*4+0xfc+0x38);const auto attached=word();
            if(attached) {skip(4);animation();}skip(0x9a);
        });
    }
    void effect() {
        block("effect",[&]{skip(4);if(!word()) return;skip(7*4+2);const auto type=word();
            if(type!=0x32 && type!=0x3d) animation();
            skip(0x1c);
        });
    }
    void parse();
};
void Reader::parse() {
    block("world",[&]{
        result.mapPath=fixedString(c.b,c.p,256);charge(result.mapPath.size());fixed("map-path",256);
        for(auto& v:result.globals) v=word();
        array("resource-registry",264);result.counter=word();map();fixed("map-secondary-state",8);
        array("queued-cell-references",4);array("records-40",40);
        block("eight-state-slots",[&]{for(unsigned i=0;i<8;++i) block("state-slot",[&]{if(word()) skip(0x150);});skip(4);});
        block("creatures",[&]{skip(8);array("creature-header-records",24);const auto n=count();for(std::uint32_t i=0;i<n;++i) creature();
            skip(28*4);array("creature-list",4);skip(16*4);});
        skip(12);
        block("missiles",[&]{const auto n=count();skip(4);for(std::uint32_t i=0;i<n;++i) missile();skip(12);for(unsigned i=0;i<4;++i) array("missile-list",4);});
        block("effects",[&]{const auto n=count();for(std::uint32_t i=0;i<n;++i) effect();skip(8);});
        fixed("104-path-slots",104*256);
        block("optional-records-660",[&]{if(word()) {const auto n=count();skip(std::uint64_t(n)*660);skip(0x6878);}});
        block("optional-records-6",[&]{if(word()) {const auto n=count();skip(std::uint64_t(n)*6);skip(16);}});
        block("100-conditional-slots",[&]{for(unsigned i=0;i<100;++i) block("conditional-slot",[&]{if(word()) skip(12);});});
        block("records-1148",[&]{skip(4);const auto n=count();skip(std::uint64_t(n)*0x47c);skip(0x19c);});
        fixed("world-controls",4+8+4+4);
        block("three-resource-slots",[&]{const auto n=count(3);for(std::uint32_t i=0;i<n;++i) block("resource-slot",[&]{const auto length=count();skip(std::uint64_t(length)+1+9);});});
        skip(8);
    });
    if(c.p!=c.b.size()) fail(PersistenceErrorCode::malformedData,c.p,"Unexpected world trailing bytes");
    result.raw=c.b;
}
void Reader::creature() {
    block("creature",[&]{skip(4);if(!word()) return;
        skip(140);const auto type=word();
        if(type==0 || type==24 || type==25 || type==26) array("creature-name",1);
        animation();skip(4);const auto mode=word();skip(8);if(!mode) skip(12);
        skip(48);animation();skip(28);fixed("42-creature-entries",42*24);
        skip(469);if(c.u8()) animation();const auto kind=word();skip(4);if(kind!=0 && kind!=2) {skip(4);animation();}
        skip(25);if(word()) animation();if(word()) animation();if(word()) animation();
        skip(56);const auto commands=count();for(std::uint32_t i=0;i<commands;++i) block("creature-command",[&]{
            skip(36);
        });if(commands) skip(4);
        skip(737);
        block("creature-word-lists",[&]{skip(8);auto n=count(30);skip(std::uint64_t(n)*4);n=count(30);skip(std::uint64_t(n)*4);skip(4);});
        skip(4);if(word()) skip(204);skip(129);
    });
}
}
PersistenceResult<SavedWorld> decodeWorldState(const Bytes& b,const WorldLimits& limits) {
    return persistence_detail::guarded<SavedWorld>([&]{Reader reader(b,limits);reader.parse();return std::move(reader.result);});
}
PersistenceResult<SavedWorld> loadWorldState(AssetFile& file,const WorldLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max())) return PersistenceError{PersistenceErrorCode::limitExceeded,0,"World input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(auto* e=std::get_if<Error>(&bytes)) return PersistenceError{e->code==ErrorCode::limitExceeded?PersistenceErrorCode::limitExceeded:PersistenceErrorCode::assetInput,0,e->detail,*e};
    return decodeWorldState(std::get<Bytes>(bytes),limits);
}
PersistenceResult<SavedWorld> decodeSavedWorld(const SavedGame& save,const WorldLimits& limits) {
    if(save.worldMarker!=0x17) return PersistenceError{PersistenceErrorCode::malformedData,0,"Save has no world section",std::nullopt};
    return decodeWorldState(save.worldTail,limits);
}
}

#include "snapshot.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace mnm::game {
namespace {
constexpr std::array<std::uint8_t,8> magic{{'M','N','M','N','W','L','D',0}};
void word(Bytes& b,std::uint64_t v,unsigned n=4) {for(unsigned i=0;i<n;++i) b.push_back(static_cast<std::uint8_t>(v>>(8*i)));}
std::uint64_t hash(const Bytes& b,std::size_t from) {
    std::uint64_t h=14695981039346656037ULL;
    for(std::size_t i=from;i<b.size();++i) {h^=b[i];h*=1099511628211ULL;}
    return h;
}
void blob(Bytes& b,const Bytes& v) {
    if(v.size()>std::numeric_limits<std::uint32_t>::max()) throw std::invalid_argument("snapshot blob too large");
    word(b,v.size());b.insert(b.end(),v.begin(),v.end());
}
void handle(Bytes& b,Handle h) {word(b,h.slot);word(b,h.generation);}
void target(Bytes& b,const std::optional<Handle>& h) {b.push_back(h?1:0);if(h) handle(b,*h);}
void point(Bytes& b,Point p) {word(b,static_cast<std::uint32_t>(p.x));word(b,static_cast<std::uint32_t>(p.y));word(b,static_cast<std::uint32_t>(p.z));}
void motion(Bytes& b,const CreatureMotion& m) {
    word(b,static_cast<std::uint32_t>(m.action));point(b,m.origin);point(b,m.destination);target(b,m.goal);
    word(b,m.budget);word(b,m.next);word(b,m.route.size());
    for(const auto& p:m.route) {point(b,p.position);word(b,static_cast<std::uint32_t>(p.direction));word(b,static_cast<std::uint32_t>(p.verticalDelta));word(b,static_cast<std::uint32_t>(p.category));word(b,p.scalar);}
}
struct Reader {
    const Bytes& b;std::size_t p=0;std::uint64_t budget=0,charged=0;
    void charge(std::uint64_t n) {if(charged>budget || n>budget-charged) throw std::invalid_argument("snapshot decoded storage limit exceeded");charged+=n;}
    void require(std::size_t n) const {if(p>b.size() || n>b.size()-p) throw std::invalid_argument("truncated snapshot");}
    std::uint64_t word(unsigned n=4) {require(n);std::uint64_t v=0;for(unsigned i=0;i<n;++i) v|=std::uint64_t(b[p++])<<(8*i);return v;}
    bool flag() {const auto v=word(1);if(v>1) throw std::invalid_argument("invalid snapshot flag");return v!=0;}
    Bytes blob() {auto n=word();require(n);charge(n);Bytes out(b.begin()+p,b.begin()+p+n);p+=n;return out;}
    Handle handle() {Handle h;h.slot=word();h.generation=word();return h;}
    std::optional<Handle> target() {if(flag()) return handle();return std::nullopt;}
    std::int32_t signedWord() {auto v=word();return v<=0x7fffffffU?static_cast<std::int32_t>(v):static_cast<std::int32_t>(static_cast<std::int64_t>(v)-0x100000000LL);}
    Point point() {Point p;p.x=signedWord();p.y=signedWord();p.z=signedWord();return p;}
    CreatureMotion motion() {
        CreatureMotion m;m.action=static_cast<Action>(word());m.origin=point();m.destination=point();m.goal=target();
        m.budget=word();m.next=word();auto count=word();
        if(count>16) throw std::invalid_argument("snapshot route limit exceeded");
        require(count*28);charge(count*28);m.route.resize(count);
        for(auto& p:m.route) {p.position=point();p.direction=signedWord();p.verticalDelta=signedWord();p.category=signedWord();p.scalar=word();}
        return m;
    }
};
}
Bytes encodeSnapshot(const State& s,const Limits& limits) {
    World::validate(s,limits);
    const bool movement=bool(s.navigation);
    Bytes payload;
    word(payload,s.sequence);word(payload,s.tick);word(payload,s.phase20);word(payload,s.phase90);word(payload,s.expansionBudget);
    blob(payload,Bytes(s.map.begin(),s.map.end()));blob(payload,s.campaign);blob(payload,s.systems);
    if(movement) {payload.push_back(1);point(payload,s.navigation->dimensions);word(payload,s.navigation->fingerprint,8);}
    word(payload,s.slots.size());
    for(const auto& slot:s.slots) {
        word(payload,slot.generation);payload.push_back(slot.entity?1:0);
        if(slot.entity) {
            const auto& e=*slot.entity;
            word(payload,static_cast<std::uint32_t>(e.family));word(payload,e.type);word(payload,e.owner);
            word(payload,static_cast<std::uint32_t>(e.x));word(payload,static_cast<std::uint32_t>(e.y));word(payload,static_cast<std::uint32_t>(e.z));
            payload.push_back(e.cleaned?1:0);target(payload,e.target);blob(payload,e.state);
            if(movement) {payload.push_back(e.motion?1:0);if(e.motion) motion(payload,*e.motion);}
        }
    }
    word(payload,s.pending.size());
    for(const auto& c:s.pending) {
        word(payload,static_cast<std::uint32_t>(c.operation));handle(payload,c.subject);target(payload,c.target);
        if(movement) {payload.push_back(c.destination?1:0);if(c.destination) point(payload,*c.destination);}
    }
    if(payload.size()>limits.bytes || payload.size()>std::numeric_limits<std::uint32_t>::max()) throw std::invalid_argument("snapshot byte limit exceeded");
    Bytes b(magic.begin(),magic.end());word(b,movement?2:1);word(b,payload.size());word(b,hash(payload,0),8);b.insert(b.end(),payload.begin(),payload.end());return b;
}
State decodeSnapshot(const Bytes& b,const Limits& limits) {
    if(b.size()<24 || b.size()-24>limits.bytes) throw std::invalid_argument("snapshot size outside limits");
    if(!std::equal(magic.begin(),magic.end(),b.begin())) throw std::invalid_argument("invalid snapshot magic");
    Reader r{b,8,limits.bytes,0};const auto version=r.word();if(version!=1 && version!=2) throw std::invalid_argument("unsupported snapshot version");
    if(r.word()!=b.size()-24) throw std::invalid_argument("snapshot size mismatch");
    if(r.word(8)!=hash(b,24)) throw std::invalid_argument("snapshot checksum mismatch");
    State s;s.sequence=r.word();s.tick=r.word();s.phase20=r.word();s.phase90=r.word();s.expansionBudget=r.word();
    auto map=r.blob();s.map.assign(map.begin(),map.end());s.campaign=r.blob();s.systems=r.blob();
    if(version==2) {
        if(!r.flag()) throw std::invalid_argument("v2 movement snapshot requires map binding");
        NavigationBinding binding;binding.dimensions=r.point();binding.fingerprint=r.word(8);s.navigation=binding;
    }
    auto count=r.word();if(count>limits.slots || count>(b.size()-r.p)/5) throw std::invalid_argument("snapshot slot limit exceeded");
    r.charge(count*Limits::slotCharge);
    s.slots.resize(count);
    for(auto& slot:s.slots) {
        slot.generation=r.word();if(r.flag()) {
            Entity e;e.family=static_cast<Family>(r.word());e.type=r.word();e.owner=r.word();e.x=r.signedWord();e.y=r.signedWord();e.z=r.signedWord();
            e.cleaned=r.flag();e.target=r.target();e.state=r.blob();slot.entity=std::move(e);
            if(version==2 && r.flag()) slot.entity->motion=r.motion();
        }
    }
    count=r.word();if(count>limits.commands || count>(b.size()-r.p)/13) throw std::invalid_argument("snapshot command limit exceeded");
    r.charge(count*Limits::commandCharge);
    s.pending.resize(count);
    for(auto& c:s.pending) {
        c.operation=static_cast<Operation>(r.word());c.subject=r.handle();c.target=r.target();
        if(version==2 && r.flag()) c.destination=r.point();
    }
    if(r.p!=b.size()) throw std::invalid_argument("unexpected snapshot trailing bytes");
    World::validate(s,limits);return s;
}
State readSnapshot(const std::filesystem::path& path,const Limits& limits) {
    if(path.native().find('\0')!=std::string::npos) throw std::invalid_argument("snapshot path contains NUL");
    std::ifstream f(path,std::ios::binary);if(!f) throw std::runtime_error("cannot open snapshot");
    // Read incrementally so the bound also applies to growing files/nonregular input.
    Bytes b;std::array<char,4096> block{};
    while(f) {
        f.read(block.data(),block.size());auto n=static_cast<std::size_t>(f.gcount());
        const auto maximum=std::min<std::uint64_t>(limits.bytes,0xffffffffULL)+24;
        if(b.size()>maximum || n>maximum-b.size()) throw std::invalid_argument("snapshot file limit exceeded");
        b.insert(b.end(),block.begin(),block.begin()+n);
    }
    if(!f.eof()) throw std::runtime_error("cannot read snapshot");
    return decodeSnapshot(b,limits);
}
void restoreSnapshot(World& world,const std::filesystem::path& path) {world.restore(readSnapshot(path,world.limits()));}
}

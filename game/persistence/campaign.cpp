#include "campaign.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

namespace mnm::game {
namespace {
constexpr std::array<std::uint8_t,8> magic{{'M','N','M','C','A','M','P',0}};
[[noreturn]] void fail(const char* why) { throw std::invalid_argument(why); }
void bounded(std::size_t n,std::uint32_t maximum) { if(n>maximum) fail("campaign resource limit exceeded"); }
void text(const std::string& s,const CampaignLimits& l) {
    bounded(s.size(),l.stringBytes);
    if(s.empty() || s.find('\0')!=std::string::npos) fail("campaign ID must be nonempty and contain no NUL");
}
void unique(const std::string& id,std::set<std::string>& ids,const CampaignLimits& l) {
    text(id,l);if(!ids.insert(id).second) fail("duplicate campaign ID");
}
void strings(const std::vector<std::string>& values,const CampaignLimits& l,bool duplicates) {
    bounded(values.size(),l.entries);std::set<std::string> seen;
    for(const auto& v:values) {text(v,l);if(!duplicates && !seen.insert(v).second) fail("duplicate campaign list ID");}
}
void validate(const CampaignState& c,const CampaignLimits& l) {
    text(c.identity.id,l);text(c.identity.contentRevision,l);
    text(c.player,l);text(c.realm,l);text(c.region,l);
    if(c.difficulty>3) fail("unsupported native campaign difficulty");
    bounded(c.wizards.size(),l.wizards);bounded(c.realms.size(),l.realms);
    std::set<std::string> wizards,realms;
    for(const auto& w:c.wizards) {
        unique(w.id,wizards,l);strings(w.spells,l,false);strings(w.inventory,l,true);
    }
    if(!wizards.count(c.player)) fail("campaign player wizard is missing");
    std::uint64_t count=0;bool selected=false;
    for(const auto& realm:c.realms) {
        unique(realm.id,realms,l);count+=realm.regions.size();
        if(count>l.regions) fail("campaign region limit exceeded");
        std::set<std::string> regions;
        for(const auto& r:realm.regions) {
            unique(r.id,regions,l);
            if(r.owner && !wizards.count(*r.owner)) fail("campaign region owner is missing");
            if(realm.id==c.realm && r.id==c.region) selected=true;
        }
    }
    if(!selected) fail("campaign selected realm/region is missing");
    if(c.lastResult) {
        const auto& r=*c.lastResult;
        text(r.realm,l);text(r.region,l);
        if(!r.attempt || r.attempt>c.transition ||
           static_cast<std::uint32_t>(r.outcome)<1 || static_cast<std::uint32_t>(r.outcome)>3)
            fail("invalid campaign result identity/outcome");
        bool found=false;
        for(const auto& realm:c.realms) for(const auto& region:realm.regions)
            if(realm.id==r.realm && region.id==r.region) found=true;
        if(!found) fail("campaign result region is missing");
    }
    if(c.mission) {
        const auto& m=*c.mission;
        if(!c.battle || !c.battle->attempt || (m.version!=1 && m.version!=2)) fail("invalid mission continuation version/attempt");
        std::set<std::pair<unsigned,unsigned>> handles;
        const auto group=[&](const auto& values) {
            if(values.size()>8) fail("mission group limit");
            for(auto h:values) if(!h.generation || !handles.emplace(h.slot,h.generation).second) fail("invalid mission group identity");
        };
        if(m.version==1) {
            if(m.stage>2 || m.initial.empty() || (m.stage==0 && m.reinforcement) ||
               (m.reinforcement!=!m.wave.empty()) || (m.stage==2 && !m.reinforcement) ||
               !m.groups.empty() || !m.objectives.empty()) fail("invalid campaign mission progress");
            group(m.initial);group(m.wave);
        } else {
            if(m.stage || m.reinforcement || !m.initial.empty() || !m.wave.empty() || m.groups.size()>8 ||
               m.objectives.empty() || m.objectives.size()>16) fail("invalid objective progress shape");
            for(const auto& values:m.groups) group(values);
            for(const auto& o:m.objectives) if(o.elapsed>4096) fail("objective timer limit");
        }
    }
    if(c.battle) {
        const auto& b=*c.battle;text(b.realm,l);text(b.region,l);text(b.map,l);
        if(c.completed || b.realm!=c.realm || b.region!=c.region) fail("campaign battle conflicts with selection/completion");
        strings(b.participants,l,false);bounded(b.participants.size(),l.wizards);
        if(b.participants.empty()) fail("campaign battle requires participants");
        for(const auto& p:b.participants) if(!wizards.count(p)) fail("campaign battle participant is missing");
        if(std::find(b.participants.begin(),b.participants.end(),c.player)==b.participants.end()) fail("campaign battle lacks player");
        if(b.attempt>c.transition || (c.lastResult && b.attempt<=c.lastResult->attempt))
            fail("invalid campaign battle attempt");
    }
}
struct Writer {
    Bytes bytes;const CampaignLimits& limits;
    void word(std::uint64_t v,unsigned n=4) {
        if(bytes.size()>limits.bytes || n>limits.bytes-bytes.size()) fail("campaign byte limit exceeded");
        for(unsigned i=0;i<n;++i) bytes.push_back(static_cast<std::uint8_t>(v>>(8*i)));
    }
    void string(const std::string& s) {
        word(s.size());
        if(s.size()>limits.bytes-bytes.size()) fail("campaign byte limit exceeded");
        bytes.insert(bytes.end(),s.begin(),s.end());
    }
    void list(const std::vector<std::string>& v) {word(v.size());for(const auto& s:v) string(s);}
};
struct Reader {
    const Bytes& bytes;const CampaignLimits& limits;std::size_t pos=0;
    void require(std::size_t n) const {if(n>bytes.size()-pos) fail("truncated campaign data");}
    std::uint64_t word(unsigned n=4) {
        require(n);std::uint64_t v=0;for(unsigned i=0;i<n;++i) v|=std::uint64_t(bytes[pos++])<<(8*i);return v;
    }
    bool flag() {const auto v=word(1);if(v>1) fail("invalid campaign flag");return v!=0;}
    std::string string() {
        const auto n=word();bounded(n,limits.stringBytes);require(n);
        std::string s(bytes.begin()+pos,bytes.begin()+pos+n);pos+=n;text(s,limits);return s;
    }
    std::uint32_t count(std::uint32_t max,std::uint32_t minimum) {
        const auto n=word();bounded(n,max);require(n*minimum);return static_cast<std::uint32_t>(n);
    }
    std::vector<std::string> list(std::uint32_t max) {
        const auto n=count(max,5);std::vector<std::string> v;
        for(std::uint32_t i=0;i<n;++i) v.push_back(string());
        return v;
    }
};
void binding(const State& state,const CampaignState& c,const CampaignIdentity& expected) {
    if(c.identity.id!=expected.id || c.identity.contentRevision!=expected.contentRevision) fail("campaign content identity mismatch");
    if(c.battle && c.battle->map!=state.map) fail("campaign battle map differs from world");
}
}
Bytes encodeCampaign(const CampaignState& c,const CampaignLimits& l) {
    validate(c,l);Writer out{{},l};
    for(auto v:magic) out.word(v,1);
    const unsigned version=c.mission ? (c.mission->version==2 ? 4 : 3) : c.lastResult || (c.battle && c.battle->attempt) ? 2 : 1;
    out.word(version);out.string(c.identity.id);out.string(c.identity.contentRevision);
    out.word(c.difficulty);out.word(c.transition,8);out.string(c.player);out.string(c.realm);out.string(c.region);out.word(c.completed,1);
    out.word(c.wizards.size());
    for(const auto& w:c.wizards) {
        out.string(w.id);out.word(w.experience,8);
        for(auto v:w.attributes) out.word(static_cast<std::uint32_t>(v));
        out.list(w.spells);out.list(w.inventory);
    }
    out.word(c.realms.size());
    for(const auto& realm:c.realms) {
        out.string(realm.id);out.word(realm.regions.size());
        for(const auto& r:realm.regions) {out.string(r.id);out.word(r.owner.has_value(),1);if(r.owner) out.string(*r.owner);out.word(r.completed,1);}
    }
    out.word(c.battle.has_value(),1);
    if(c.battle) {
        const auto& b=*c.battle;out.string(b.realm);out.string(b.region);out.string(b.map);out.list(b.participants);
        if(version>=2) out.word(b.attempt,8);
    }
    if(version>=2) {
        out.word(c.lastResult.has_value(),1);
        if(c.lastResult) {
            const auto& r=*c.lastResult;out.word(r.attempt,8);out.string(r.realm);out.string(r.region);
            out.word(static_cast<std::uint32_t>(r.outcome));
        }
    }
    if(version==3) {
        const auto& m=*c.mission;out.word(m.stage);out.word(m.reinforcement,1);
        for(const auto* group:{&m.initial,&m.wave}) {out.word(group->size());for(auto h:*group) {out.word(h.slot);out.word(h.generation);}}
    }
    if(version==4) {
        const auto& m=*c.mission;out.word(m.groups.size());
        for(const auto& group:m.groups) {out.word(group.size());for(auto h:group) {out.word(h.slot);out.word(h.generation);}}
        out.word(m.objectives.size());
        for(const auto& o:m.objectives) {out.word(o.complete,1);out.word(o.elapsed);}
    }
    return out.bytes;
}
CampaignState decodeCampaign(const Bytes& bytes,const CampaignLimits& l) {
    bounded(bytes.size(),l.bytes);Reader in{bytes,l};
    for(auto v:magic) if(in.word(1)!=v) fail("invalid campaign signature");
    const auto version=in.word();if(version!=1 && version!=2 && version!=3 && version!=4) fail("unsupported campaign version");
    CampaignState c;c.identity={in.string(),in.string()};c.difficulty=in.word();c.transition=in.word(8);
    c.player=in.string();c.realm=in.string();c.region=in.string();c.completed=in.flag();
    auto n=in.count(l.wizards,45);
    for(std::uint32_t i=0;i<n;++i) {
        CampaignWizard w;w.id=in.string();w.experience=in.word(8);
        for(auto& v:w.attributes) {const auto raw=in.word();v=raw<=0x7fffffffU?static_cast<std::int32_t>(raw):static_cast<std::int32_t>(static_cast<std::int64_t>(raw)-0x100000000LL);}
        w.spells=in.list(l.entries);w.inventory=in.list(l.entries);c.wizards.push_back(std::move(w));
    }
    n=in.count(l.realms,9);std::uint32_t regions=0;
    for(std::uint32_t i=0;i<n;++i) {
        CampaignRealm realm;realm.id=in.string();const auto count=in.count(l.regions-regions,7);regions+=count;
        for(std::uint32_t j=0;j<count;++j) {CampaignRegion r;r.id=in.string();if(in.flag()) r.owner=in.string();r.completed=in.flag();realm.regions.push_back(std::move(r));}
        c.realms.push_back(std::move(realm));
    }
    if(in.flag()) {
        CampaignBattle b;b.realm=in.string();b.region=in.string();b.map=in.string();b.participants=in.list(std::min(l.wizards,l.entries));
        if(version>=2) b.attempt=in.word(8);
        c.battle=std::move(b);
    }
    if(version>=2) {
        if(in.flag()) {
            CampaignResult r;r.attempt=in.word(8);r.realm=in.string();r.region=in.string();
            r.outcome=static_cast<CampaignOutcome>(in.word());c.lastResult=std::move(r);
        }
        if(!c.lastResult && (!c.battle || !c.battle->attempt)) fail("version-2 campaign requires continuation data");
    }
    if(version==3) {
        MissionProgress m;m.stage=in.word();m.reinforcement=in.flag();
        for(auto* group:{&m.initial,&m.wave}) {const auto count=in.count(8,8);for(unsigned i=0;i<count;++i) group->push_back({unsigned(in.word()),unsigned(in.word())});}
        c.mission=std::move(m);
    }
    if(version==4) {
        MissionProgress m;m.version=2;
        const auto groups=in.count(8,4);
        for(unsigned i=0;i<groups;++i) {
            std::vector<Handle> group;const auto count=in.count(8,8);
            for(unsigned j=0;j<count;++j) group.push_back({unsigned(in.word()),unsigned(in.word())});
            m.groups.push_back(std::move(group));
        }
        const auto objectives=in.count(16,5);
        for(unsigned i=0;i<objectives;++i) {MissionProgress::Objective o;o.complete=in.flag();o.elapsed=in.word();m.objectives.push_back(o);}
        c.mission=std::move(m);
    }
    if(in.pos!=bytes.size()) fail("unexpected trailing campaign bytes");
    validate(c,l);return c;
}
namespace {
constexpr const char* checkpointTag="mnm-native-campaign-checkpoint-v1";
State envelope(const CampaignCheckpoint& checkpoint,const CampaignIdentity& expected,const Limits& limits,const CampaignLimits& l) {
    binding(checkpoint.world,checkpoint.campaign,expected);
    State outer;outer.map=checkpointTag;outer.campaign=encodeCampaign(checkpoint.campaign,l);
    outer.systems=encodeSnapshot(checkpoint.world,limits);
    World::validate(outer,limits);return outer;
}
CampaignCheckpoint unpack(State outer,const CampaignIdentity& expected,const Limits& limits,const CampaignLimits& l) {
    if(outer.map!=checkpointTag || outer.sequence || outer.tick || outer.phase20 || outer.phase90 ||
       outer.expansionBudget || !outer.slots.empty() || !outer.pending.empty() || outer.navigation || outer.animation)
        fail("invalid native campaign checkpoint envelope");
    CampaignCheckpoint checkpoint{decodeSnapshot(outer.systems,limits),decodeCampaign(outer.campaign,l)};
    binding(checkpoint.world,checkpoint.campaign,expected);return checkpoint;
}
}
void setCampaign(CampaignCheckpoint& checkpoint,const CampaignState& c,const Limits& limits,const CampaignLimits& l) {
    binding(checkpoint.world,c,c.identity);encodeCampaign(c,l);World::validate(checkpoint.world,limits);
    auto staged=c;checkpoint.campaign=std::move(staged);
}
Bytes encodeCampaignSnapshot(const CampaignCheckpoint& checkpoint,const CampaignIdentity& expected,const Limits& limits,const CampaignLimits& l) {
    return encodeSnapshot(envelope(checkpoint,expected,limits,l),limits);
}
CampaignCheckpoint decodeCampaignSnapshot(const Bytes& bytes,const CampaignIdentity& expected,const Limits& limits,const CampaignLimits& l) {
    return unpack(decodeSnapshot(bytes,limits),expected,limits,l);
}
CampaignCheckpoint readCampaignSnapshot(const std::filesystem::path& path,const CampaignIdentity& expected,const Limits& limits,const CampaignLimits& l) {
    return unpack(readSnapshot(path,limits),expected,limits,l);
}
CommitResult writeCampaignSnapshot(const std::filesystem::path& path,const CampaignCheckpoint& checkpoint,const CampaignIdentity& expected,bool overwrite,const Limits& limits,const CampaignLimits& l) {
    return writeSnapshot(path,envelope(checkpoint,expected,limits,l),overwrite,limits);
}
}

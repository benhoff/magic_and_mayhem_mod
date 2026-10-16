#include "campaign.hpp"
#include "trace.hpp"
#include "creature_runtime.hpp"
#include <charconv>
#include <iostream>
#include <sstream>
#include <string_view>

namespace {
template<class T> T number(const char* text) {
    const std::string_view s(text);T n{};
    const auto parsed=std::from_chars(s.data(),s.data()+s.size(),n);
    if(parsed.ec!=std::errc{} || parsed.ptr!=s.data()+s.size())
        throw std::invalid_argument("Invalid integer argument");
    return n;
}
void quoted(std::ostream& out,const std::string& text) {
    constexpr char hex[]="0123456789abcdef";out<<'"';
    for(unsigned char c:text) {
        if(c=='"' || c=='\\') out<<'\\'<<char(c);
        else if(c<32) out<<"\\u00"<<hex[c>>4]<<hex[c&15];
        else out<<char(c);
    }
    out<<'"';
}
void view(const mnm::game::CampaignSession& s,unsigned settlements=0) {
    const auto& p=s.progress();
    std::cout<<"{\"campaign\":";quoted(std::cout,p.identity.id);
    std::cout<<",\"content_revision\":";quoted(std::cout,p.identity.contentRevision);
    std::cout<<",\"realm\":";quoted(std::cout,p.realm);
    std::cout<<",\"region\":";quoted(std::cout,p.region);
    std::cout<<",\"transition\":"<<p.transition<<",\"completed\":"<<(p.completed?"true":"false")
             <<",\"settlements\":"<<settlements<<",\"last_result\":";
    if(p.lastResult) {
        const auto& r=*p.lastResult;
        std::cout<<"{\"attempt\":"<<r.attempt<<",\"realm\":";quoted(std::cout,r.realm);
        std::cout<<",\"region\":";quoted(std::cout,r.region);
        std::cout<<",\"outcome\":"<<static_cast<unsigned>(r.outcome)<<'}';
    } else std::cout<<"null";
    std::cout<<",\"attempt\":";
    if(p.battle) std::cout<<p.battle->attempt;else std::cout<<"null";
    std::cout<<",\"battle\":";
    if(s.battle()) mnm::skirmish::trace(std::cout,*s.battle());else std::cout<<"null";
    std::cout<<"}\n";
}
void save(const char* path,const mnm::game::CampaignSession& s) {
    const auto result=s.save(path);
    if(!result.durable) throw std::runtime_error("Checkpoint published; durability unconfirmed: "+result.detail);
}
}
int main(int argc,char** argv) try {
    using namespace mnm::game;
    std::shared_ptr<const CombatDriver> driver;
    if(argc>=4 && std::string_view(argv[1])=="--installation") {driver=mnm::skirmish::installedCreatureRuntime(argv[2]);argc-=2;argv+=2;}
    const std::string_view command=argc>1?argv[1]:"";
    if(command=="create" && argc==3) {
        auto s=mnm::skirmish::newCampaign(true,true,driver);save(argv[2],s);view(s);return 0;
    }
    if(command=="inspect" && argc==3) {view(mnm::skirmish::loadCampaign(argv[2],driver));return 0;}
    if(command=="start" && argc==4) {
        auto s=mnm::skirmish::loadCampaign(argv[2],driver);s.startMission();save(argv[3],s);view(s);return 0;
    }
    if(command=="cast" && (argc==5 || argc==6 || argc==7)) {
        auto s=mnm::skirmish::loadCampaign(argv[2],driver);
        if(!s.battle()) throw std::invalid_argument("Start a campaign mission before casting");
        const auto meta=s.battle()->state();SkirmishOrder order;order.subject=meta.wizards[0].actor;
        if(std::string_view(argv[4])=="bolt" && argc==5) {
            order.kind=SkirmishOrderKind::bolt;order.target=meta.wizards[1].actor;
        } else if((std::string_view(argv[4])=="cure" || std::string_view(argv[4])=="sphere") && argc==6) {
            order.kind=std::string_view(argv[4])=="sphere" ? SkirmishOrderKind::sphere : SkirmishOrderKind::cure;
            const auto slot=number<unsigned>(argv[5]);
            if(slot>=s.battle()->world().state().slots.size() || !s.battle()->world().state().slots[slot].entity) throw std::invalid_argument("Cure target is unavailable");
            order.target=Handle{slot,s.battle()->world().state().slots[slot].generation};
        } else if(std::string_view(argv[4])=="summon" && argc==7) {
            order.kind=SkirmishOrderKind::summon;order.destination=Point{number<int>(argv[5]),number<int>(argv[6]),1};
        } else throw std::invalid_argument("Use bolt, summon X Y or cure/sphere TARGET-SLOT");
        s.enqueue(order);save(argv[3],s);view(s);return 0;
    }
    if(command=="resume" && argc==5) {
        const auto ticks=number<unsigned>(argv[4]);
        if(ticks>4096) throw std::invalid_argument("Ticks must be 0..4096");
        auto s=mnm::skirmish::loadCampaign(argv[2],driver);unsigned settlements=0;
        for(unsigned i=0;i<ticks && s.battle();++i) if(s.step().result) ++settlements;
        save(argv[3],s);view(s,settlements);return 0;
    }
    throw std::invalid_argument("Usage: mnm-campaign-cli [--installation ROOT] create OUTPUT | inspect INPUT | start INPUT OUTPUT | "
                                "cast INPUT OUTPUT bolt | cast INPUT OUTPUT summon X Y | cast INPUT OUTPUT cure/sphere TARGET-SLOT | resume INPUT OUTPUT TICKS");
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}

#include "campaign.hpp"
#ifdef MNM_MAP_CAMPAIGN_CLI
#include "map_campaign.hpp"
#endif
#include "trace.hpp"
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
#ifdef MNM_MAP_CAMPAIGN_CLI
    if(argc<3) throw std::invalid_argument("Usage: mnm-map-campaign-cli ASSET_ROOT COMMAND ...");
    auto resources=mnm::skirmish::mapCampaignResources(argv[1]);--argc;++argv;
    const auto make=[&]{return mnm::skirmish::newMapCampaign(resources);};
    const auto load=[&](const char* path){return mnm::skirmish::loadMapCampaign(resources,path);};
#else
    const auto make=[]{return mnm::skirmish::newCampaign();};
    const auto load=[](const char* path){return mnm::skirmish::loadCampaign(path);};
#endif
    const std::string_view command=argc>1?argv[1]:"";
    if(command=="create" && argc==3) {
        auto s=make();save(argv[2],s);view(s);return 0;
    }
    if(command=="inspect" && argc==3) {view(load(argv[2]));return 0;}
    if(command=="start" && argc==4) {
        auto s=load(argv[2]);s.startMission();save(argv[3],s);view(s);return 0;
    }
    if(command=="cast" && (argc==5 || argc==7 || argc==8)) {
        auto s=load(argv[2]);
        if(!s.battle()) throw std::invalid_argument("Start a campaign mission before casting");
        const auto meta=s.battle()->state();SkirmishOrder order;order.subject=meta.wizards[0].actor;
        if(std::string_view(argv[4])=="bolt" && argc==5) {
            order.kind=SkirmishOrderKind::bolt;order.target=meta.wizards[1].actor;
        } else if(std::string_view(argv[4])=="summon" && (argc==7 || argc==8)) {
            order.kind=SkirmishOrderKind::summon;order.destination=Point{number<int>(argv[5]),number<int>(argv[6]),argc==8?number<int>(argv[7]):1};
        } else throw std::invalid_argument("Use bolt or summon X Y");
        s.enqueue(order);save(argv[3],s);view(s);return 0;
    }
    if(command=="move" && argc==7) {
        auto s=load(argv[2]);
        if(!s.battle()) throw std::invalid_argument("Start a campaign mission before moving");
        std::optional<Handle> actor;
        for(const auto &unit:s.battle()->state().units)
            if(unit.role==UnitRole::sentinel && s.battle()->world().find(unit.actor)->owner==0) {actor=unit.actor;break;}
        if(!actor) throw std::invalid_argument("Summon a friendly Sentinel before moving");
        s.enqueue({SkirmishOrderKind::move,*actor,{},Point{number<int>(argv[4]),number<int>(argv[5]),number<int>(argv[6])}});
        save(argv[3],s);view(s);return 0;
    }
    if(command=="resume" && argc==5) {
        const auto ticks=number<unsigned>(argv[4]);
        if(ticks>4096) throw std::invalid_argument("Ticks must be 0..4096");
        auto s=load(argv[2]);unsigned settlements=0;
        for(unsigned i=0;i<ticks && s.battle();++i) if(s.step().result) ++settlements;
        save(argv[3],s);view(s,settlements);return 0;
    }
    throw std::invalid_argument("Usage: mnm-campaign-cli create OUTPUT | inspect INPUT | start INPUT OUTPUT | "
                                "cast INPUT OUTPUT bolt | cast INPUT OUTPUT summon X Y | resume INPUT OUTPUT TICKS");
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}

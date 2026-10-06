#include "../reconstruction/rendering/surface_retry.hpp"
#include "../reconstruction/rendering/surface_bitmap.hpp"
#include <QCryptographicHash>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
using namespace mnm::reconstruction;
using U=std::uint32_t;
struct Adapter:SurfaceRetryAdapter {
    std::vector<std::array<U,6>> events;std::vector<U> draws;unsigned used=0;
    U rs=0,rd=0,kr=0;
    void event(U kind,U id=0,U h=0,U flags=0,U a=0,U b=0){events.push_back({kind,id,h,flags,a,b});}
    U draw(bool fast,bool fill,U flags,std::uint16_t color) override {
        if(used==draws.size())throw std::runtime_error("Finite schedule exhausted");
        const auto h=draws[used++];event(fast?2:1,2,h,flags,fill?color:0);return h;
    }
    U restore(RecoverySurface s) override {const auto h=s==RecoverySurface::Source?rs:rd;event(3,U(s),h);return h;}
    U setSourceKey(RecoverySurface s,std::uint16_t key) override {event(4,U(s),kr,8,key,key);return kr;}
    void reload(RecoverySurface s) override {event(5,U(s));}
    void report(U h) override {event(6,0,h);}
    void reloadCursorBitmap() override {event(7,3);}
};
static std::string hash(const std::uint8_t* data,std::size_t n){return QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(data),int(n)),QCryptographicHash::Sha256).toHex().toStdString();}
int main(int argc,char** argv)try{
    if(argc!=3 && argc!=4)throw std::runtime_error("Expected cases/assets and optional budget");
    const auto budget=argc==4?std::stoul(argv[3]):16;std::ifstream input(argv[1]);if(!input)throw std::runtime_error("Case input missing");
    std::cout<<"{\"rows\":[";bool first=true;U id,entry,fast,wait,key,sr,dr,rs,rd,kr,color,count;
    while(input>>id>>entry>>fast>>wait>>key>>sr>>dr>>rs>>rd>>kr>>color>>count){
        if(count>16||sr>2||dr>2)throw std::runtime_error("Input limits");
        Adapter a;a.rs=rs;a.rd=rd;a.kr=kr;
        for(U i=0,v;i<count;++i){if(!(input>>v))throw std::runtime_error("Short input");a.draws.push_back(v);}
        SurfaceRetryInput c;c.wrapper=entry==0x58bac0?SurfaceWrapper::PartialFill:entry==0x58bc10?SurfaceWrapper::FullFill:entry==0x58c4a0?SurfaceWrapper::OpaqueCopy:SurfaceWrapper::KeyedCopy;
        c.fast=fast;c.noWait=wait;c.keyEnabled=key;c.sourceRoute=SurfaceReloadRoute(sr);c.destinationRoute=SurfaceReloadRoute(dr);c.sourceKey=0x5678;c.destinationKey=0xe123;c.color=color;
        const auto result=runSurfaceRetry(c,a,budget);if(!first)std::cout<<',';first=false;
        std::cout<<"{\"id\":"<<id<<",\"draws_consumed\":"<<a.used<<",\"returned\":"<<(result.returned?"true":"false")<<",\"budget_exhausted\":"<<(result.budgetExhausted?"true":"false")<<",\"events\":[";
        bool f=true;for(const auto& e:a.events){if(!f)std::cout<<',';f=false;std::cout<<'[';for(unsigned i=0;i<6;++i){if(i)std::cout<<',';std::cout<<e[i];}std::cout<<']';}std::cout<<"]}";
    }
    if(!input.eof())throw std::runtime_error("Malformed input");
    std::cout<<"],\"assets\":[";first=true;
    for(const char* name:{"missing","short-file","bad-signature","short-info","short-palette","short-pixels","indexed1","indexed4","indexed8","rgb24","offset-gap","installed-cursors"}){
        std::ifstream f(std::string(argv[2])+"/"+name+".bmp",std::ios::binary);
        std::optional<std::vector<std::uint8_t>> bytes;if(f)bytes=std::vector<std::uint8_t>(std::istreambuf_iterator<char>(f),{});
        std::optional<mnm::render::DibInput> dib;
        auto reader=[&](std::string_view path){if(path!="bitmaps\\cursors.bmp")throw std::runtime_error("Wrong cursor path");return bytes;};
        auto draw=[&](const mnm::render::DibInput& data){dib=data;return 0x80004005u;};
        const auto loaded=loadSurfaceBitmap(reader,draw,true);
        if(!first)std::cout<<',';
        first=false;std::cout<<"{\"asset\":\""<<name<<"\",\"loaded\":"<<loaded;
        if(dib)std::cout<<",\"usage\":"<<dib->usage<<",\"info_sha256\":\""<<hash(dib->header.data(),40)<<"\",\"palette_sha256\":\""<<hash(dib->palette.data(),dib->palette.size())<<"\",\"pixels_sha256\":\""<<hash(dib->pixels.data(),dib->pixels.size())<<'"';
        if(reloadCursorBitmap(reader,draw,false)!=0)throw std::runtime_error("Sentinel return differs");
        std::cout<<'}';
    }
    std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

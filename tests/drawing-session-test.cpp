#include "../compat/legacy/drawing_session.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
using namespace mnm;
using namespace resource_test;
using namespace mnm::legacy;
int main(int argc,char **argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;
    require(directory.isValid(),"Session fixture directory");
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    const auto input=store(root);render::GlBlitter renderer;
    auto source=[&](const std::string &name){
        auto opened=input.open(name);require(std::holds_alternative<std::unique_ptr<assets::AssetFile>>(opened),"Session source missing");
        auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto loaded=assets::readWhole(*file,1048576);
        require(std::holds_alternative<Bytes>(loaded),"Session source read failed");return std::get<Bytes>(std::move(loaded));
    };
    auto record=[](unsigned sequence,unsigned kind,unsigned width=5,unsigned height=3){
        CanvasProducer c{};auto &r=c.fields;r[0]=96;r[1]=sequence;r[2]=kind;r[3]=7;r[5]=width;r[6]=height;
        if(kind==5){r[12]=width;r[13]=height;r[14]=0x1234;}
        if(kind==11||kind==12)r[14]=1;
        return c;
    };
    auto defined=[&](DrawingSession &s,unsigned segment,unsigned width=5,unsigned height=3){
        s.apply(s.session(),segment,record(1,1,width,height));s.apply(s.session(),segment,record(2,5,width,height));
    };
    auto published=[&](DrawingSession &s,unsigned segment,unsigned frame,unsigned width=5,unsigned height=3){
        return s.publish(s.session(),segment,frame,{2,0,7,width,height});
    };
    QJsonArray cases;
    {
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::World);defined(s,1);
        s.apply(1,1,record(3,11));auto draw=record(4,9);auto raw=sprite();
        draw.payload.assign(raw.begin()+36,raw.begin()+91);word(draw.payload,28,0);
        draw.fields[0]=96+draw.payload.size();draw.fields[18]=draw.payload.size();
        draw.fields[8]=2;draw.fields[12]=5;draw.fields[13]=3;draw.fields[15]=8;draw.fields[19]=55;
        s.apply(1,1,draw);s.apply(1,1,record(5,12));
        const auto frame=s.publish(1,1,1,{5,1,7,5,3});
        require(s.read(frame).pixels[7]==0xf800,"Session lost asset-backed native World raster");s.finish(1);
        cases.append("asset-backed-world-completion");
    }
    {
        DrawingSession s(renderer,input,source);require(s.state()==DrawingSessionState::Idle,"Session initial state");s.begin(41);
        unsigned i=0;
        for(auto stage:{DrawingStage::Menu,DrawingStage::Loading,DrawingStage::World,DrawingStage::Results,DrawingStage::Menu}){
            ++i;s.transition(41,i,stage);defined(s,i);const auto frame=published(s,i,i);
            require(frame.session==41&&frame.segment==i&&frame.presentation==i&&frame.stage==stage,"Session frame identity changed");
            require(s.read(frame).pixels==std::vector<std::uint32_t>(15,0x1234),"Stage lost complete native pixels");
        }
        const auto frame=*s.current();s.finish(41);require(s.state()==DrawingSessionState::Finished&&s.records()==10&&s.frames()==5,"Terminal session counts");
        rejected([&]{s.apply(41,5,record(3,5));});require(s.read(frame).pixels[0]==0x1234,"Terminal mutation changed frame");
        rejected([&]{s.begin(41);});s.stop();s.stop();rejected([&]{s.read(frame);});s.begin(42);
        require(s.segments()==0&&s.frames()==0&&s.records()==0&&!s.current(),"Restart inherited old state");
        cases.append("stage-order-terminal-restart");
    }
    {
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::Menu);defined(s,1);
        const auto old=published(s,1,1);auto lease=s.presentGpu(old);
        s.transition(1,2,DrawingStage::World);rejected([&]{s.read(old);});require(lease.valid(),"Transition destroyed retained frame lease");
        defined(s,2,6,4);const auto next=published(s,2,2,6,4);
        require(s.read(next).width==6&&s.read(next).pixels.size()==24,"Recreated canvas extent wrong");
        auto forged=next;forged.session=2;rejected([&]{s.read(forged);});forged=next;forged.stage=DrawingStage::Menu;rejected([&]{s.presentGpu(forged);});
        require(s.state()==DrawingSessionState::Running&&s.current()==std::optional<DrawingFrameIdentity>(next),"Stale query poisoned valid frame");
        s.stop();require(lease.valid(),"Shutdown destroyed retained GPU lease");lease={};
        require(renderer.stats().surfaces==0,"External lease release leaked storage");
        cases.append("generation-extent-stale-query-gpu-lease");
    }
    {
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::World);defined(s,1);
        s.apply(1,1,record(3,11));s.transition(1,2,DrawingStage::Menu);
        s.apply(1,2,record(1,1));rejected([&]{s.publish(1,2,1,{1,0,7,5,3});});
        require(s.state()==DrawingSessionState::Failed&&!s.current()&&renderer.stats().surfaces==0,"Undefined transition exposed old history");
        s.begin(2);s.transition(2,1,DrawingStage::Menu);defined(s,1);published(s,1,1);s.finish(2);
        cases.append("interrupted-world-undefined-history-fresh-recovery");
    }
    for(unsigned bad:{0u,1u,3u}){
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::Menu);defined(s,1);published(s,1,1);
        rejected([&]{s.transition(1,bad,DrawingStage::World);});require(s.state()==DrawingSessionState::Failed&&!s.current(),"Segment gap not poisoned");
    }
    cases.append("segment-gap-refusal");
    for(unsigned bad:{0u,1u,3u}){
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::Menu);defined(s,1);published(s,1,1);
        auto next=record(3,5);s.apply(1,1,next);rejected([&]{s.publish(1,1,bad,{3,0,7,5,3});});
        require(s.state()==DrawingSessionState::Failed,"Frame gap not poisoned");
    }
    cases.append("presentation-gap-refusal");
    for(unsigned fault:{0u,1u,2u,3u}){
        DrawingSession s(renderer,input,source);s.begin(2);s.transition(2,1,DrawingStage::Menu);defined(s,1);const auto frame=published(s,1,1);
        rejected([&]{
            if(fault==0)s.apply(1,1,record(3,5));
            if(fault==1)s.apply(2,2,record(3,5));
            if(fault==2)s.apply(2,1,record(4,5));
            if(fault==3)s.transition(2,2,static_cast<DrawingStage>(99));
        });
        require(s.state()==DrawingSessionState::Failed&&!s.current(),"Identity/source fault retained publication");rejected([&]{s.read(frame);});
        rejected([&]{s.begin(2);});s.begin(3);require(!s.current()&&s.records()==0,"Failed history reused on recovery");
    }
    cases.append("cross-session-cross-segment-source-gap-stage-refusal");
    {
        DrawingSession s(renderer,input,source);s.begin(1);rejected([&]{s.begin(2);});require(s.session()==1,"Active begin replaced session");
        rejected([&]{s.finish(1);});require(s.state()==DrawingSessionState::Failed,"Empty finish accepted");
        s.begin(2);s.transition(2,1,DrawingStage::Menu);defined(s,1);auto frame=published(s,1,1);
        s.apply(2,1,record(3,5));rejected([&]{s.read(frame);});rejected([&]{s.finish(2);});
        cases.append("active-begin-empty-and-stale-terminal-refusal");
    }
    for(unsigned limit:{0u,1u,2u}){
        DrawingSessionLimits limits;limits.segments=limits.frames=1;limits.records=2;
        DrawingSession s(renderer,input,source,limits);s.begin(1);s.transition(1,1,DrawingStage::Menu);defined(s,1);published(s,1,1);
        rejected([&]{
            if(limit==0)s.transition(1,2,DrawingStage::World);
            if(limit==1)s.publish(1,1,2,{2,0,7,5,3});
            if(limit==2)s.apply(1,1,record(3,5));
        });
        require(s.state()==DrawingSessionState::Failed&&renderer.stats().surfaces==0,"Budget exhaustion retained surfaces");
    }
    cases.append("segment-frame-record-budget-refusal");
    {
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::World);defined(s,1);
        s.apply(1,1,record(3,11));rejected([&]{s.publish(1,1,1,{3,0,7,5,3});});
        require(s.state()==DrawingSessionState::Failed,"Incomplete World frame published");
        cases.append("incomplete-world-refusal");
    }
    {
        DrawingSession s(renderer,input,source);s.begin(1);s.transition(1,1,DrawingStage::Menu);s.apply(1,1,record(1,1));
        auto image=record(2,20);const std::string missing="absent.bmp";image.payload.assign(missing.begin(),missing.end());image.payload.push_back(0);
        image.fields[0]=96+image.payload.size();image.fields[18]=image.payload.size();
        rejected([&]{s.apply(1,1,image);});require(s.state()==DrawingSessionState::Failed,"Missing asset did not refuse");
        cases.append("source-asset-failure");
    }
    for(unsigned invalid:{0u,65u}){DrawingSessionLimits limits;limits.segments=invalid;rejected([&]{DrawingSession s(renderer,input,source,limits);});}
    rejected([&]{DrawingSession s(renderer,input,{});});cases.append("configuration-refusal");
    require(renderer.stats().surfaces==0,"Session lifecycle leaked native storage");
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"cases",cases},{"remaining_surfaces",0},
        {"original_execution",false},{"driver",QString::fromStdString(renderer.driver().renderer)}}).toJson(QJsonDocument::Compact).toStdString()<<'\n';return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}

#include "../compat/legacy/drawing_scene.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char **argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;
    require(directory.isValid(),"Fixture directory");const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    auto bitmapBytes=bmp();word(bitmapBytes,22,1);bitmapBytes[58]=0;bitmapBytes[59]=255;write(root,"ui.bmp",bitmapBytes);
    const auto input=store(root);render::GlBlitter renderer;
    auto source=[&](const std::string &name){
        auto result=input.open(name);require(std::holds_alternative<std::unique_ptr<assets::AssetFile>>(result),"Source fixture unavailable");
        auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(result));auto data=assets::readWhole(*file,1048576);
        require(std::holds_alternative<Bytes>(data),"Source fixture read");return std::get<Bytes>(std::move(data));
    };
    auto create=[](unsigned id){legacy::CanvasProducer c{};c.fields[2]=1;c.fields[3]=id;c.fields[5]=5;c.fields[6]=3;return c;};
    auto fill=[](unsigned id,unsigned value){legacy::CanvasProducer c{};c.fields[2]=5;c.fields[3]=id;c.fields[12]=5;c.fields[13]=3;c.fields[14]=value;return c;};
    auto entry=[](){legacy::CanvasProducer c{};c.fields[2]=11;c.fields[3]=7;c.fields[5]=5;c.fields[6]=3;c.fields[14]=1;return c;};
    auto raster=[](){
        auto raw=sprite();legacy::CanvasProducer c{};auto &r=c.fields;r[2]=9;r[3]=7;r[5]=5;r[6]=3;r[8]=2;r[12]=5;r[13]=3;r[15]=8;r[19]=55;
        c.payload.assign(raw.begin()+36,raw.begin()+91);word(c.payload,28,0);return c;
    };
    auto submit=[](legacy::DrawingScene &scene,legacy::CanvasProducer c){c.fields[0]=96+c.bytes().size();c.fields[18]=c.bytes().size();c.fields[1]=scene.sequence()+1;return scene.apply(c);};
    auto startup=[&](legacy::DrawingScene &scene){submit(scene,create(7));submit(scene,fill(7,0x1234));};
    auto world=[&](legacy::DrawingScene &scene){submit(scene,entry());submit(scene,raster());auto returned=entry();returned.fields[2]=12;submit(scene,returned);};
    {
        legacy::DrawingScene scene(renderer,input,source);rejected([&]{scene.read();});rejected([&]{scene.presentGpu();});startup(scene);world(scene);
        // Native UI comes after World and must survive opaque final copies.
        auto hud=fill(7,0x4321);hud.fields[12]=1;hud.fields[13]=1;submit(scene,hud);
        auto bitmap=create(7);bitmap.fields[2]=20;bitmap.fields[8]=3;bitmap.fields[9]=0;
        const std::string path="ui.bmp";bitmap.payload.assign(path.begin(),path.end());bitmap.payload.push_back(0);submit(scene,bitmap);
        submit(scene,create(8));auto copy=create(8);copy.fields[2]=6;copy.fields[4]=7;copy.fields[12]=5;copy.fields[13]=3;copy.fields[14]=0xffffffff;submit(scene,copy);
        auto checkpoint=create(8);checkpoint.fields[2]=10;checkpoint.fields[14]=1;const auto completed=submit(scene,checkpoint);
        require(completed&&completed->image.pixels[0]==0x4321&&completed->image.pixels[7]==0xf800&&completed->image.pixels[4]==0xf800,"World, HUD, bitmap or final copy order changed");
        rejected([&]{scene.presentGpu();});scene.seal({scene.sequence(),1,8,5,3});
        require(scene.read().pixels==completed->image.pixels,"Complete GPU scene lost composed canvas");scene.presentGpu();
        rejected([&]{submit(scene,hud);});require(scene.read().pixels==completed->image.pixels,"Sealed frame changed");
    }
    require(renderer.stats().surfaces==0,"Sealed scene cleanup leaked storage");
    {
        legacy::DrawingScene scene(renderer,input,source);submit(scene,create(7));auto partial=fill(7,0);partial.fields[12]=1;submit(scene,partial);
        rejected([&]{submit(scene,entry());});rejected([&]{scene.seal({scene.sequence(),1,7,5,3});});rejected([&]{scene.presentGpu();});
    }
    {
        legacy::DrawingScene scene(renderer,input,source);startup(scene);submit(scene,entry());
        rejected([&]{scene.seal({scene.sequence(),1,7,5,3});});rejected([&]{scene.read();});
    }
    for(unsigned bad:{5u,11u,13u,99u}){
        legacy::DrawingScene scene(renderer,input,source);startup(scene);submit(scene,entry());auto unknown=fill(7,0);unknown.fields[2]=bad;
        rejected([&]{submit(scene,unknown);});rejected([&]{scene.presentGpu();});rejected([&]{submit(scene,raster());});
    }
    {
        legacy::DrawingScene scene(renderer,input,source);startup(scene);auto gap=entry();gap.fields[0]=96;gap.fields[1]=4;
        rejected([&]{scene.apply(gap);});rejected([&]{scene.read();});
    }
    for(unsigned changed:{0u,1u,2u,3u}){
        legacy::DrawingScene scene(renderer,input,source);startup(scene);world(scene);legacy::DrawingBoundary boundary{scene.sequence(),1,7,5,3};
        if(changed==0)++boundary.sequence;
        if(changed==1)++boundary.queues;
        if(changed==2)boundary.canvas=8;
        if(changed==3)++boundary.width;
        rejected([&]{scene.seal(boundary);});rejected([&]{scene.presentGpu();});
    }
    {
        legacy::DrawingScene scene(renderer,input,source);startup(scene);submit(scene,entry());submit(scene,raster());auto returned=entry();returned.fields[2]=12;returned.fields[3]=8;
        rejected([&]{submit(scene,returned);});rejected([&]{scene.presentGpu();});
    }
    {
        legacy::DrawingScene scene(renderer,input,source);startup(scene);submit(scene,entry());auto missing=raster();missing.payload[54]^=1;submit(scene,missing);
        auto returned=entry();returned.fields[2]=12;rejected([&]{submit(scene,returned);});rejected([&]{scene.presentGpu();});
    }
    {
        legacy::DrawingScene scene(renderer,input,source);startup(scene);world(scene);auto checkpoint=create(7);checkpoint.fields[2]=10;checkpoint.fields[14]=2;
        rejected([&]{submit(scene,checkpoint);});rejected([&]{scene.presentGpu();});
    }
    require(renderer.stats().surfaces==0,"Refused scene cleanup leaked storage");
    std::cout<<"Native drawing scene order, asset-backed composition, undefined history, boundaries, poisoning and cleanup pass\n";return 0;
}catch(const std::exception &error){std::cerr<<error.what()<<'\n';return 1;}

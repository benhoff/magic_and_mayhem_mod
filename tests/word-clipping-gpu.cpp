// Pixel-only native comparison. Reads encoded input and initial canvas;
// deliberately never reads the original/expected after-canvas in sample files.
#include "sprite.hpp"
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <limits>
#include <stdexcept>

static std::uint32_t word(const QByteArray& b,int at){
    if(at<0 || at>b.size()-4)throw std::runtime_error("input extent");
    const auto* p=reinterpret_cast<const unsigned char*>(b.constData()+at);
    return p[0]|std::uint32_t(p[1])<<8|std::uint32_t(p[2])<<16|std::uint32_t(p[3])<<24;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);
    if(argc!=2)throw std::runtime_error("manifest argument required");
    QFile manifest(argv[1]);if(!manifest.open(QIODevice::ReadOnly)||manifest.size()>1024*1024)throw std::runtime_error("manifest bounds");
    QJsonParseError error;const auto document=QJsonDocument::fromJson(manifest.readAll(),&error);
    if(error.error!=QJsonParseError::NoError||!document.isArray())throw std::runtime_error("manifest array required");
    const auto entries=document.array();if(entries.empty()||entries.size()>8192)throw std::runtime_error("manifest count");
    mnm::render::GlBlitter renderer;int count=0;
    for(const auto& entry:entries){
        const auto item=entry.toObject();QFile file(item["input"].toString());
        if(!file.open(QIODevice::ReadOnly))throw std::runtime_error("sample open");
        const auto header=file.read(208);
        if(header.size()!=208||header.left(8)!="MNMWRC01"||word(header,8)!=1)throw std::runtime_error("sample header");
        const auto size=word(header,32),bytes=word(header,36),width=word(header,40),height=word(header,44),stride=word(header,48);
        if(size<48||size>4*1024*1024||!width||!height||width>2048||height>2048||stride<width||stride>4096||bytes!=stride*height*2||file.size()!=208+size+bytes*2)throw std::runtime_error("sample bounds");
        const auto frame=file.read(size),before=file.read(bytes);file.close();
        if(frame.size()!=size||before.size()!=bytes||word(frame,0)!=size||word(frame,28)!=UINT32_MAX)throw std::runtime_error("word frame extent/storage");
        const auto fw=word(frame,4),fh=word(frame,8);
        if(!fw||!fh||fw>2048||fh>2048||40+fh*8>size)throw std::runtime_error("frame dimensions");
        mnm::render::PreparedSpriteFrame prepared;
        prepared.pixels={int(fw),int(fh),std::vector<std::uint32_t>(std::size_t(fw)*fh)};
        prepared.mask={int(fw),int(fh),std::vector<std::uint32_t>(std::size_t(fw)*fh)};
        prepared.originX=std::int32_t(word(frame,12));prepared.originY=std::int32_t(word(frame,16));
        for(std::uint32_t y=0;y<fh;++y){
            auto delta=word(frame,40+y*8),pixel=word(frame,44+y*8);std::uint32_t x=0;bool opaque=false;
            while(x<fw){
                if(delta>=size)throw std::runtime_error("control extent");
                const auto n=std::uint8_t(frame[int(delta++)]);
                if(n>fw-x||(opaque&&!n))throw std::runtime_error("run extent");
                if(opaque){
                    if(pixel>size||n>(size-pixel)/2)throw std::runtime_error("pixel extent");
                    for(unsigned i=0;i<n;++i){
                        const auto at=std::size_t(y)*fw+x+i;
                        prepared.pixels.pixels[at]=std::uint8_t(frame[int(pixel)])|std::uint32_t(std::uint8_t(frame[int(pixel+1)]))<<8;
                        prepared.mask.pixels[at]=1;pixel+=2;
                    }
                }
                x+=n;opaque=!opaque;
            }
        }
        mnm::render::Image initial{int(stride),int(height),std::vector<std::uint32_t>(std::size_t(stride)*height)};
        for(std::size_t i=0;i<initial.pixels.size();++i)initial.pixels[i]=std::uint8_t(before[int(i*2)])|std::uint32_t(std::uint8_t(before[int(i*2+1)]))<<8;
        const auto destination=renderer.create(initial,mnm::render::spriteFormat);
        {
            mnm::render::UploadedSpriteFrame sprite(renderer,std::move(prepared));
            auto budget=std::numeric_limits<std::size_t>::max();
            while(!sprite.advanceUpload(budget)){}
            const auto start=renderer.stats();
            sprite.drawClipped(destination,std::int32_t(word(header,52)),std::int32_t(word(header,56)),{0,0,int(width),int(height)});
            const auto end=renderer.stats();
            if(start.uploads!=end.uploads||start.nativeReadbacks!=end.nativeReadbacks)throw std::runtime_error("draw performed upload/readback");
        }
        const auto result=renderer.read(destination);renderer.destroy(destination);
        QByteArray output;output.reserve(int(bytes));
        for(const auto value:result.pixels){output.append(char(value&255));output.append(char(value>>8));}
        QFile target(item["output"].toString());
        if(!target.open(QIODevice::WriteOnly|QIODevice::NewOnly)||target.write(output)!=output.size()||!target.flush())throw std::runtime_error("output write");
        ++count;
    }
    const auto stats=renderer.stats();if(stats.surfaces||stats.pixels)throw std::runtime_error("resource leak");
    const auto driver=renderer.driver();
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"cases",count},{"surfaces",int(stats.surfaces)},
        {"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)},
        {"version",QString::fromStdString(driver.version)}}).toJson().constData();
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}

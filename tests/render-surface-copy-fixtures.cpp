#include "surface_copy.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <climits>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static Rect rect(const QJsonArray& a){require(a.size()==4,"Rectangle length");return {a[0].toInt(),a[1].toInt(),a[2].toInt(),a[3].toInt()};}
static Image image(const QJsonArray& a){require(a.size()==48,"Fixture image length");Image i{8,6,{}};for(auto p:a)i.pixels.push_back(std::uint32_t(p.toDouble()));return i;}
static QJsonArray pixels(const Image& i){QJsonArray a;for(auto p:i.pixels)a.append(qint64(p));return a;}
static const PixelFormat rgb565{16,{0xf800,0x7e0,31}};
static unsigned policyTests(GlBlitter& gl){
    unsigned count=0;
    const auto check=[&](bool ok,const char* text){require(ok,text);++count;};
    const auto rejected=[&](auto action){bool rejected=false;try{action();}catch(const std::runtime_error&){rejected=true;}check(rejected,"Unsupported policy accepted");};
    Image a{8,6,std::vector<std::uint32_t>(48,100)},b{8,6,std::vector<std::uint32_t>(48,200)};
    const auto s=gl.create(a,rgb565),d=gl.create(b,rgb565);
    SurfaceCopyRequest request;request.source=request.destination={0,0,8,6};
    ClipperState clip{true,std::vector<Rect>{{1,1,3,2}}};gl.setClipper(d,clip);clip.regions->clear();
    check(gl.surfaceCopy(s,d,request).pieces==1,"Clip state aliases caller storage");
    auto expected=b;expected.pixels[9]=expected.pixels[10]=100;
    check(gl.read(d).pixels==expected.pixels,"Stored clipping differs");
    gl.update(d,0,0,b);
    rejected([&]{gl.setClipper(d,{true,std::vector<Rect>{{0,0,3,3},{2,2,4,4}}});});
    rejected([&]{gl.setClipper(d,{true,std::vector<Rect>{{-1,0,3,3}}});});
    rejected([&]{gl.setClipper(d,{false,std::vector<Rect>{}});});
    rejected([&]{gl.setClipper(d,{true,std::vector<Rect>(33,Rect{0,0,1,1})});});
    check(gl.surfaceCopy(s,d,request).pieces==1,"Rejected setter changed clip state");
    check(gl.read(d).pixels==expected.pixels,"Update lost clipping");
    gl.update(d,0,0,b);gl.swapContents(s,d);gl.surfaceCopy(s,d,request);
    auto swapped=a;swapped.pixels[9]=swapped.pixels[10]=200;
    check(gl.read(d).pixels==swapped.pixels,"Swap moved clipper identity");
    gl.destroy(d);const auto replacement=gl.create(a,rgb565);gl.surfaceCopy(s,replacement,request);
    check(gl.read(replacement).pixels==b.pixels,"Replacement inherited clipping");
    check(gl.surfaceCopy(s,s,request).hresult==0,"Opaque self-copy rejected");
    rejected([&]{gl.copy(s,s,{0,0,8,6},0,0);});
    request.flags=0x8000;rejected([&]{gl.surfaceCopy(s,s,request);});request.flags=0;
    const auto other=gl.create(a,{16,{0x7c00,0x3e0,31}});
    rejected([&]{gl.surfaceCopy(s,other,request);});gl.destroy(other);
    request.destination={0,0,4,3};rejected([&]{gl.surfaceCopy(s,replacement,request);});
    request.destination={0,0,8,6};request.flags=3;rejected([&]{gl.surfaceCopy(s,replacement,request);});
    request.flags=0;request.api=SurfaceCopyApi::BltFast;request.destination={INT_MAX,INT_MAX,0,0};
    check(gl.surfaceCopy(s,replacement,request).hresult==surfaceStatus::invalidRect,"Overflow coordinate admission");
    request.source={INT_MIN,INT_MIN,INT_MAX,INT_MAX};
    check(gl.surfaceCopy(s,replacement,request).hresult==surfaceStatus::invalidRect,"Overflow rectangle admission");
    check(gl.read(replacement).pixels==b.pixels,"Overflow admission changed pixels");
    gl.destroy(s);gl.destroy(replacement);check(gl.stats().surfaces==0,"Policy resources leaked");return count;
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try {
        require(argc==3,"Expected input JSON and new output JSON");QFile input(QString::fromLocal8Bit(argv[1]));
        require(input.open(QIODevice::ReadOnly),"Input open");QJsonParseError error;
        const auto document=QJsonDocument::fromJson(input.readAll(),&error);
        require(error.error==QJsonParseError::NoError && document.isArray(),"Input JSON");
        GlBlitter gl;const auto policy=policyTests(gl);QJsonArray results;unsigned operations=0;
        for(auto entry:document.array()){
            const auto c=entry.toObject();const auto source=image(c["source_pixels"].toArray()),destination=image(c["destination_pixels"].toArray());
            const auto s=gl.create(source,rgb565),d=c["shared"].toBool()?s:gl.create(destination,rgb565);
            const auto region=c["clip"].toInt();ClipperState clip;
            if(region){clip.attached=true;if(region!=3){clip.regions=std::vector<Rect>{};if(region==1)clip.regions->push_back({2,1,6,5});
                else if(region==2)*clip.regions={{0,0,3,2},{5,3,8,6}};else if(region==5)*clip.regions={{0,0,8,3},{0,4,8,6}};else require(region==4,"Unknown fixture clip");}}
            gl.setClipper(d,clip);SurfaceCopyRequest request;
            request.api=c["fast"].toInt()?SurfaceCopyApi::BltFast:SurfaceCopyApi::Blt;
            request.source=rect(c["source"].toArray());request.destination=rect(c["destination"].toArray());
            request.flags=std::uint32_t(c["flags"].toDouble());request.sourceBusy=c["held"].toInt()==1;request.destinationBusy=c["held"].toInt()==2;
            const auto before=gl.stats();const auto result=gl.surfaceCopy(s,d,request);const auto after=gl.stats();
            require(after.nativeReadbacks==before.nativeReadbacks && after.rgbaReadbacks==before.rgbaReadbacks && after.uploads==before.uploads,"Ordinary clipping read back/uploaded pixels");
            results.append(QJsonObject{{"id",c["id"]},{"hresult",qint64(result.hresult)},{"pieces",int(result.pieces)},
                {"source_after",pixels(gl.read(s))},{"destination_after",pixels(gl.read(d))}});
            gl.destroy(s);if(s!=d)gl.destroy(d);++operations;
        }
        require(gl.stats().surfaces==0,"Fixture resources leaked");const auto driver=gl.driver();
        QJsonObject report{{"success",true},{"policy_tests",int(policy)},{"operations",int(operations)},{"ordinary_readbacks",0},{"ordinary_uploads",0},
            {"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)},{"cases",results}};
        QFile output(QString::fromLocal8Bit(argv[2]));require(output.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Output exists or unavailable");
        const auto data=QJsonDocument(report).toJson();require(output.write(data)==data.size() && output.flush(),"Output write");
        std::cout<<"Native Surface2 fixture operations: "<<operations<<", policy checks: "<<policy<<'\n';return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}

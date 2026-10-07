#include "commands.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
unsigned frames=0,sessions=0,rejections=0;
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
QByteArray native(const Image& image,unsigned bits){QByteArray out;for(auto p:image.pixels)for(unsigned i=0;i<bits/8;++i)out.append(char(p>>(8*i)));return out;}
QImage rgba(const Image& image,PixelFormat format,const std::vector<Rgb>& palette){
    QImage out(image.width,image.height,QImage::Format_RGBA8888);
    for(int y=0;y<image.height;++y)for(int x=0;x<image.width;++x){
        const auto p=image.pixels[std::size_t(y)*image.width+x];unsigned rgb[3];
        if(format.bits==8){const auto c=palette.at(p);rgb[0]=c.red;rgb[1]=c.green;rgb[2]=c.blue;}
        else for(unsigned i=0;i<3;++i){const auto mask=format.masks[i],low=mask&(~mask+1);rgb[i]=((p&mask)/low)*255/(mask/low);}
        if(format.bits==16 && format.masks==std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}){
            const auto r=(p>>11)&31,g=(p>>5)&63,b=p&31;
            rgb[0]=(r<<3)|(r>>2);rgb[1]=(g<<2)|(g>>4);rgb[2]=(b<<3)|(b>>2);
        }
        out.setPixelColor(x,y,QColor(rgb[0],rgb[1],rgb[2]));
    }
    return out;
}
QByteArray bytes(const QImage& image){return {reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes()};}
void compare(GlViewport& viewport,const QImage& expected){
    viewport.repaint();const auto actual=viewport.grabFramebuffer();
    require(viewport.ready() && viewport.error().isEmpty(),"GPU viewport failed");
    require(actual.size()==expected.size()*24,"Unexpected test viewport dimensions");
    for(int y=0;y<actual.height();++y)for(int x=0;x<actual.width();++x)
        require(actual.pixelColor(x,y)==expected.pixelColor(x/24,y/24),"Independent complete-frame mismatch");
    require(viewport.imageUploads()==0,"GPU batch presentation uploaded CPU pixels");++frames;
}
SurfaceCommand command(unsigned op,std::initializer_list<unsigned> words){SurfaceCommand c;c.operation=op;std::copy(words.begin(),words.end(),c.words.begin());return c;}
SurfaceCommand create(unsigned id,Image image,PixelFormat format){auto c=command(1,{id,unsigned(image.width),unsigned(image.height),format.bits,format.masks[0],format.masks[1],format.masks[2]});c.image=std::move(image);c.format=format;return c;}
struct Prefix {std::size_t surfaces=0,pixels=0;unsigned uploads=0,copies=0,palettes=0,checks=0,colorChecks=0,presents=0;};
struct Fixture {
    std::vector<SurfaceCommand> commands;
    std::vector<QImage> images;
    std::vector<Prefix> prefixes;
    void add(SurfaceCommand c){
        c.sequence=unsigned(commands.size()+1);auto prefix=prefixes.empty()?Prefix{}:prefixes.back();
        switch(c.operation){
        case 1:++prefix.surfaces;prefix.pixels+=c.image.pixels.size();++prefix.uploads;break;
        case 2:++prefix.uploads;break;case 3:++prefix.copies;break;case 4:++prefix.palettes;break;
        case 5:++prefix.checks;break;case 6:++prefix.presents;break;case 7:--prefix.surfaces;prefix.pixels-=12;break;
        case 10:++prefix.colorChecks;break;default:break;
        }
        commands.push_back(std::move(c));prefixes.push_back(prefix);
    }
};
Fixture fixture(PixelFormat format){
    Fixture f;Image a{4,3,{}},b{4,3,{}};
    const auto mask=format.bits==8?255u:format.bits==16?65535u:0xffffffu;
    for(unsigned i=0;i<12;++i){a.pixels.push_back((i*771+5)&mask);b.pixels.push_back((i*2137+17)&mask);}
    std::vector<Rgb> paletteA,paletteB;
    for(unsigned i=0;i<256;++i){paletteA.push_back({std::uint8_t(i),std::uint8_t(i*3),std::uint8_t(255-i)});paletteB.push_back({std::uint8_t(255-i),std::uint8_t(i*5),std::uint8_t(i)});}
    f.add(create(11,a,format));f.add(create(22,b,format));
    if(format.bits==8){for(auto id:{11u,22u}){auto c=command(4,{id,0,256});c.colors=id==11?paletteA:paletteB;f.add(c);}}
    const auto check=[&](unsigned id,const Image& image,const std::vector<Rgb>& palette){
        auto c=command(5,{id});c.expected=native(image,format.bits);f.add(c);
        c=command(10,{id});c.expected=bytes(rgba(image,format,palette));f.add(c);
    };
    for(unsigned i=0;i<8;++i){
        auto c=command(2,{11,i%3,i%3,2,1});c.image={2,1,{(i*151+3)&mask,(i*193+11)&mask}};f.add(c);
        a.pixels[(i%3)*4+i%3]=c.image.pixels[0];a.pixels[(i%3)*4+i%3+1]=c.image.pixels[1];
        const bool keyed=i%2;const auto key=keyed?a.pixels[0]:0;
        f.add(command(3,{11,22,0,0,3,2,1,1,unsigned(keyed),key}));
        for(unsigned y=0;y<2;++y)for(unsigned x=0;x<3;++x)if(!keyed || a.pixels[y*4+x]!=key)b.pixels[(y+1)*4+x+1]=a.pixels[y*4+x];
        if(format.bits==8){paletteB[i]={std::uint8_t(i*13),std::uint8_t(i*17),std::uint8_t(i*19)};auto p=command(4,{22,i,1});p.colors={paletteB[i]};f.add(p);}
        check(22,b,paletteB);f.add(command(6,{22}));f.images.push_back(rgba(b,format,paletteB));
        if(i%3==2){f.add(command(11,{11,22}));std::swap(a,b);check(22,b,paletteB);f.add(command(6,{22}));f.images.push_back(rgba(b,format,paletteB));}
    }
    f.add(command(7,{11}));f.add(command(7,{22}));f.add(create(33,b,format));
    if(format.bits==8){auto c=command(4,{33,0,256});c.colors=paletteB;f.add(c);}
    check(33,b,paletteB);f.add(command(6,{33}));f.images.push_back(rgba(b,format,paletteB));f.add(command(7,{33}));f.add(command(8,{}));return f;
}
void stats(const CommandResult& out,const Prefix& expected,bool verify){
    require(out.liveSurfaces==expected.surfaces && out.livePixels==expected.pixels,"Batch resource ownership changed");
    require(out.stats.surfaces==expected.surfaces && out.stats.pixels==expected.pixels,"Renderer resource state changed");
    require(out.stats.uploads==expected.uploads && out.stats.copies==expected.copies && out.stats.paletteUpdates==expected.palettes,"Batch GPU operations changed");
    require(out.presents==expected.presents && out.stats.gpuPresentations==expected.presents,"Presentation count changed");
    require(out.checks==(verify?expected.checks:0) && out.colorChecks==(verify?expected.colorChecks:0),"Diagnostic counts changed");
    require(out.skippedChecks==(verify?0:expected.checks) && out.skippedColorChecks==(verify?0:expected.colorChecks),"Skipped diagnostics not reported");
    require(out.stats.nativeReadbacks==(verify?expected.checks:0) && out.stats.rgbaReadbacks==(verify?expected.colorChecks:0),"Ordinary execution read back CHECK pixels");
    require(out.stats.presentations==expected.presents+(verify?expected.colorChecks:0),"Resolved presentation count changed");
}
void run(GlViewport& viewport,const Fixture& f,const std::vector<std::size_t>& batches,bool verify){
    viewport.setGpuFrame({});viewport.makeCurrent();GlBlitter renderer(viewport.context());unsigned displayed=0;
    CommandConsumer consumer(renderer,[&](GpuFrame frame){
        require(QOpenGLContext::currentContext()==viewport.context(),"Presentation callback retained renderer context");
        viewport.setGpuFrame(frame);compare(viewport,f.images.at(displayed++));viewport.makeCurrent();
    }, {verify?CommandDiagnostics::Verify:CommandDiagnostics::Skip,false});
    consumer.submit(nullptr,0);std::size_t cursor=0,batch=0;
    while(cursor<f.commands.size()){
        const auto count=std::min(batches[batch++%batches.size()],f.commands.size()-cursor);
        viewport.makeCurrent();
        consumer.submit(f.commands.data()+cursor,count);cursor+=count;stats(consumer.result(),f.prefixes[cursor-1],verify);
        require(QOpenGLContext::currentContext()==viewport.context(),"Command batch did not restore caller context");
        require(consumer.result().commands==cursor,"Batch command accounting changed");
        require(consumer.state()==(cursor==f.commands.size()?CommandConsumerState::Ended:CommandConsumerState::Active),"Batch lifecycle changed");
        QApplication::processEvents();
    }
    consumer.finish();consumer.finish();require(displayed==f.images.size(),"Missing or extra frames");
    bool refused=false;try {consumer.submit(f.commands.front());}catch(const std::runtime_error&){refused=true;}
    require(refused && consumer.state()==CommandConsumerState::Ended,"Closed consumer accepted commands");++sessions;
}
void batchContexts(GlViewport& viewport){
    viewport.setGpuFrame({});viewport.makeCurrent();GlBlitter renderer(viewport.context());
    QOpenGLFunctions_3_3_Core caller;require(caller.initializeOpenGLFunctions(),"Caller GL functions unavailable");
    GLint framebuffer=0;caller.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&framebuffer);
    const auto restored=[&]{
        require(QOpenGLContext::currentContext()==viewport.context(),"Batch context restoration failed");
        GLint actual=0;caller.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&actual);
        require(actual==framebuffer,"Batch changed caller framebuffer state");
    };
    const PixelFormat format{16,{0xf800,0x7e0,0x1f}};
    SurfaceId source=0,destination=0;
    renderer.batch([&]{
        auto* backend=QOpenGLContext::currentContext();require(backend && backend!=viewport.context(),"Batch did not enter renderer context");
        source=renderer.create({2,1,{0xf800,0x07e0}},format);destination=renderer.create({2,1,{0,0}},format);
        renderer.batch([&]{renderer.copy(source,destination,{0,0,2,1},0,0);});
        require(QOpenGLContext::currentContext()==backend,"Nested batch released renderer context");
        require(renderer.read(destination).pixels==std::vector<std::uint32_t>({0xf800,0x07e0}),"Nested batch pixels differ");
    });restored();
    bool refused=false;
    try {renderer.batch([&]{renderer.update(destination,0,0,{1,1,{0x001f}});renderer.update(destination,2,0,{1,1,{0}});});}
    catch(const std::runtime_error&){refused=true;}
    require(refused,"Invalid operation in batch accepted");restored();
    require(renderer.read(destination).pixels==std::vector<std::uint32_t>({0x001f,0x07e0}),"Failed batch rolled back preceding update");restored();
    // Fault injection: Release checks must still catch driver errors when a
    // batch returns, even though its ordinary operation checks are deferred.
    refused=false;
    try {renderer.batch([&]{
        QOpenGLFunctions_3_3_Core backend;require(backend.initializeOpenGLFunctions(),"Backend GL functions unavailable");
        backend.glEnable(GLenum(0xffffffff));
    });}catch(const std::runtime_error&){refused=true;}
    require(refused,"Batch boundary ignored GL error");restored();
    renderer.batch([&]{renderer.destroy(source);renderer.destroy(destination);});restored();
    viewport.doneCurrent();renderer.batch([&]{source=renderer.create({1,1,{0}},format);});
    require(!QOpenGLContext::currentContext(),"Batch did not restore absent context");renderer.destroy(source);
}
void failures(GlViewport& viewport){
    const PixelFormat format{8,{}};const auto first=create(1,{1,1,{3}},format);
    std::vector<SurfaceCommand> invalid;
    invalid.push_back(command(2,{2,0,0,1,1}));invalid.back().image={1,1,{1}};
    invalid.push_back(first);invalid.push_back(command(6,{2}));invalid.push_back(command(11,{1,1}));
    invalid.push_back(command(3,{1,1,0,0,1,1,0,0,0,0}));
    invalid.push_back(command(2,{1,1,0,1,1}));invalid.back().image={1,1,{1}};
    invalid.push_back(command(4,{1,255,2}));invalid.back().colors={{1,2,3},{4,5,6}};
    invalid.push_back(command(5,{1}));invalid.back().expected="\x04";
    invalid.push_back(command(10,{1}));invalid.back().expected=QByteArray(4,'\0');
    invalid.push_back(command(9,{7}));invalid.push_back(command(99,{}));
    invalid.push_back(command(8,{}));invalid.push_back(command(7,{2}));
    auto huge=create(2,{1,1,{1}},format);huge.words[1]=2049;invalid.push_back(huge);
    auto badFormat=create(2,{1,1,{1}},format);badFormat.format.bits=7;badFormat.words[3]=7;invalid.push_back(badFormat);
    auto badPixels=create(2,{1,1,{256}},format);invalid.push_back(badPixels);
    for(auto bad:invalid){
        viewport.setGpuFrame({});GlBlitter renderer(viewport.context());
        const auto external=renderer.create({1,1,{1}},format);
        CommandConsumer consumer(renderer,[](GpuFrame){},{CommandDiagnostics::Verify,false});
        auto c=first;c.sequence=1;consumer.submit(c);bad.sequence=2;
        bool refused=false;try {consumer.submit(bad);}catch(const std::runtime_error&){refused=true;}
        require(refused && consumer.state()==CommandConsumerState::Failed,"Invalid command did not terminate session");
        require(consumer.result().liveSurfaces==0 && renderer.stats().surfaces==1,"Failure destroyed external surface or leaked owned surface");
        renderer.destroy(external);++rejections;
    }
    // Skipped comparisons never imply a match; malformed CHECK lengths still fail.
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);
     auto bad=command(5,{1});bad.sequence=2;bad.expected="\x04";consumer.submit(bad);require(consumer.result().skippedChecks==1,"Skipped check not counted");
     bad.sequence=3;bad.expected.clear();bool refused=false;try {consumer.submit(bad);}catch(const std::runtime_error&){refused=true;}
     require(refused && renderer.stats().surfaces==0,"Malformed skipped CHECK accepted");++rejections;}
    // Ordinal gaps, stale/reused IDs and trailing commands remain explicit failures.
    for(unsigned which=0;which<4;++which){
        GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);
        auto destroy=command(7,{1});destroy.sequence=2;
        SurfaceCommand bad;
        if(which==0){bad=command(6,{1});bad.sequence=3;}
        else {consumer.submit(destroy);bad=which==1?first:command(6,{1});bad.sequence=3;}
        if(which==3){bad=first;bad.sequence=2;}
        bool refused=false;try {consumer.submit(bad);}catch(const std::runtime_error&){refused=true;}
        require(refused && consumer.state()==CommandConsumerState::Failed && renderer.stats().surfaces==0,"Sequence or identity failure leaked");++rejections;
    }
    // Partial sessions own resources until explicitly interrupted or destroyed.
    {GlBlitter renderer(viewport.context());{CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);
        require(renderer.stats().surfaces==1,"Prefix resource vanished");consumer.abort();require(consumer.state()==CommandConsumerState::Aborted && renderer.stats().surfaces==0,"Abort leaked");}
     {CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);}require(renderer.stats().surfaces==0,"Destructor leaked");}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);
     bool refused=false;try {consumer.finish();}catch(const std::runtime_error&){refused=true;}require(refused && consumer.state()==CommandConsumerState::Failed && renderer.stats().surfaces==0,"Interrupted finish accepted");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){throw std::runtime_error("Callback failure");});auto c=first;c.sequence=1;consumer.submit(c);auto p=command(6,{1});p.sequence=2;
     bool refused=false;try {consumer.submit(p);}catch(const std::runtime_error&){refused=true;}require(refused && renderer.stats().surfaces==0,"Callback failure leaked");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer* recursive=nullptr;
     CommandConsumer consumer(renderer,[&](GpuFrame){recursive->abort();});recursive=&consumer;auto c=first;c.sequence=1;consumer.submit(c);auto p=command(6,{1});p.sequence=2;
     bool refused=false;try {consumer.submit(p);}catch(const std::runtime_error&){refused=true;}require(refused && consumer.state()==CommandConsumerState::Failed && renderer.stats().surfaces==0,"Recursive lifecycle mutation accepted");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);
     bool refused=false;try {consumer.submit(nullptr,1);}catch(const std::runtime_error&){refused=true;}
     require(refused && consumer.state()==CommandConsumerState::Failed && renderer.stats().surfaces==0,"Null batch did not clean up");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});
     auto c=first;c.sequence=1;auto present=command(6,{1});present.sequence=2;auto destroy=command(7,{1});destroy.sequence=3;
     auto end=command(8,{});end.sequence=4;auto trailing=first;trailing.sequence=5;
     const SurfaceCommand batch[]={c,present,destroy,end,trailing};bool refused=false;
     try {consumer.submit(batch,5);}catch(const std::runtime_error&){refused=true;}
     require(refused && consumer.state()==CommandConsumerState::Failed && renderer.stats().surfaces==0,"Trailing command batch accepted");++rejections;}
    // A new consumer admits a fresh session after interruption; displayed leases survive.
    {GlBlitter renderer(viewport.context());const auto corpus=fixture(PixelFormat{8,{}});unsigned displayed=0;
     {CommandConsumer consumer(renderer,[&](GpuFrame frame){viewport.setGpuFrame(frame);compare(viewport,corpus.images.at(displayed++));});
      std::size_t prefix=0;while(corpus.commands[prefix].operation!=6)++prefix;
      consumer.submit(corpus.commands.data(),prefix+1);consumer.abort();
      require(consumer.state()==CommandConsumerState::Aborted && renderer.stats().surfaces==0,"Presented interruption leaked");
      compare(viewport,corpus.images[0]);bool refused=false;try {consumer.submit(corpus.commands[0]);}catch(const std::runtime_error&){refused=true;}
      require(refused && consumer.state()==CommandConsumerState::Aborted,"Aborted session accepted work");}
     {CommandConsumer fresh(renderer,[](GpuFrame){});auto c=first;c.sequence=1;fresh.submit(c);fresh.abort();}
     require(renderer.stats().surfaces==0,"Fresh session after abort leaked");}
    // Record and live-surface budgets apply across batches.
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});auto c=first;c.sequence=1;consumer.submit(c);
     for(unsigned i=2;i<=4096;++i){auto check=command(5,{1});check.sequence=i;check.expected="\x03";consumer.submit(check);}
     c=command(7,{1});c.sequence=4097;bool refused=false;try {consumer.submit(c);}catch(const std::runtime_error&){refused=true;}
     require(refused && renderer.stats().surfaces==0,"Record bound accepted");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});bool refused=false;
     try {for(unsigned i=1;i<=65;++i){auto c=create(i,{1,1,{1}},format);c.sequence=i;consumer.submit(c);}}catch(const std::runtime_error&){refused=true;}
     require(refused && renderer.stats().surfaces==0,"Surface bound accepted");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});bool refused=false;
     try {auto c=create(1,{2048,2048,std::vector<std::uint32_t>(2048*2048)},format);c.sequence=1;consumer.submit(c);
          QByteArray expected(2048*2048,'\0');for(unsigned i=2;i<20;++i){auto check=command(5,{1});check.sequence=i;check.expected=expected;consumer.submit(check);}}
     catch(const std::runtime_error&){refused=true;}
     require(refused && renderer.stats().surfaces==0,"Byte budget accepted");++rejections;}
    {GlBlitter renderer(viewport.context());CommandConsumer consumer(renderer,[](GpuFrame){});bool refused=false;
     try {for(unsigned i=1;i<=5;++i){auto c=create(i,{2048,2048,std::vector<std::uint32_t>(2048*2048)},format);c.sequence=i;consumer.submit(c);}}
     catch(const std::runtime_error&){refused=true;}
     require(refused && renderer.stats().surfaces==0,"Pixel budget accepted");++rejections;}
}
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    try {
        GlViewport viewport;viewport.setMinimumSize(1,1);viewport.resize(96,72);viewport.show();app.processEvents();
        const PixelFormat formats[]={{8,{}},{16,{0xf800,0x7e0,0x1f}},{24,{0xff0000,0xff00,0xff}},{32,{0xff0000,0xff00,0xff}}};
        for(auto f:formats){const auto corpus=fixture(f);for(bool verify:{false,true})
            for(const auto& batches:std::vector<std::vector<std::size_t>>{{4096},{1},{7},{1,3,11,2,17}})run(viewport,corpus,batches,verify);}
        batchContexts(viewport);failures(viewport);QJsonObject result{{"success",true},{"full_frame_comparisons",int(frames)},{"partitioned_sessions",int(sessions)},
            {"failure_cases",int(rejections)},{"ordinary_readbacks",0},{"gpu_viewport_uploads",0},{"prefix_resources_checked",true}};
        result.insert("batch_context_restoration",true);result.insert("batch_error_boundary",true);
        std::puts(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"Incremental consumer test failed: %s\n",e.what());return 1;}
}

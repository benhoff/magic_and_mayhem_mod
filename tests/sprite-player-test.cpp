#include "sprite_window.hpp"
#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QTemporaryDir>
#include <QTest>
#include <fstream>
#include <iostream>
#include <map>
using namespace mnm;
namespace {
unsigned checks=0,refusals=0,pixelCases=0,windowActions=0,installedWindows=0;
void require(bool ok,const char* detail){++checks;if(!ok)throw std::runtime_error(detail);}
template<class F> void refuses(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught,"Expected refusal");++refusals;}
void put(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t v){for(unsigned j=0;j<4;++j)b.at(at+j)=std::uint8_t(v>>(8*j));}
void save(const std::filesystem::path& p,const std::vector<std::uint8_t>& b){std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());require(bool(f),"Fixture write failed");}
const sprites::Name first={'A',0,255,0,0,0,0,1},second={'A',0,255,0,0,0,0,2};
std::pair<std::vector<std::uint8_t>,std::vector<std::uint8_t>> fixture(){
    const unsigned count=6,base=24+768+count*4,extent=52;
    std::vector<std::uint8_t> spr(base+extent*count),tag(count*12);
    put(spr,0,0x00525053);put(spr,4,spr.size());put(spr,8,4);put(spr,12,count);put(spr,16,1);
    spr[24+3]=255;spr[24+7]=255; // Palette1 red, palette2 green.
    for(unsigned i=0;i<count;++i){const auto off=base+extent*i;const auto& name=i%2?second:first;
        put(spr,24+768+i*4,extent*i);put(spr,off,extent);put(spr,off+4,2);put(spr,off+8,1);
        put(spr,off+12,1);put(spr,off+16,0xffffffffu);
        std::copy(name.begin(),name.end(),spr.begin()+off+20);put(spr,off+40,48);put(spr,off+44,50);
        spr[off+49]=2;spr[off+50]=1;spr[off+51]=2;
        std::copy(name.begin(),name.end(),tag.begin()+12*i);put(tag,12*i+8,i/2);
    }return {spr,tag};
}
render::Image oracle(const render::SpriteDrawPlan& plan){
    render::Image image{sprites::previewWidth,sprites::previewHeight,std::vector<std::uint32_t>(sprites::previewWidth*sprites::previewHeight,0x2104)};
    for(const auto& p:plan.placements){const auto& s=*plan.source;const auto& f=s.frames.at(p.frame);
        for(std::uint32_t y=0;y<f.height;++y)for(std::uint32_t x=0;x<f.width;++x){const auto n=std::size_t(y)*f.width+x;
            const auto dx=std::int64_t(p.x)-f.originX+x,dy=std::int64_t(p.y)-f.originY+y;
            if(!f.opaqueMask.at(n) || dx<0 || dy<0 || dx>=image.width || dy>=image.height)continue;
            unsigned word=0;
            if(s.storage==assets::SpriteStorage::rgb565)word=std::get<std::vector<std::uint16_t>>(f.pixels).at(n);
            else {const auto c=s.palettes.at(*f.paletteIndex).at(std::get<std::vector<std::uint8_t>>(f.pixels).at(n));word=((c.red>>3)<<11)|((c.green>>2)<<5)|(c.blue>>3);}
            image.pixels.at(std::size_t(dy)*image.width+dx)=word;
        }
    }return image;
}
void pixels(render::GlBlitter& gl,const render::SpriteDrawPlan& plan){const auto before=gl.stats();const auto expected=oracle(plan);
    require(sprites::renderPreview(gl,plan).pixels==expected.pixels,"GPU pixels differ from direct decoded-plane oracle");
    require(gl.stats().surfaces==before.surfaces,"Preview leaked a surface");++pixelCases;
}
template<class T>T* child(QWidget& w,const char* name){auto* p=w.findChild<T*>(name);require(p!=nullptr,"Missing UI control");return p;}
void click(QWidget& w,const char* name){QTest::mouseClick(child<QPushButton>(w,name),Qt::LeftButton);++windowActions;}
void synthetic(){
    QTemporaryDir tmp;require(tmp.isValid(),"Temporary fixture directory failed");const auto root=std::filesystem::path(tmp.path().toStdString());
    auto [spr,tag]=fixture();save(root/"sheet.spr",spr);save(root/"sheet.tag",tag);save(root/"Terrain.spr",spr);
    sprites::SpritePlayer player(root);refuses([&]{player.plan();});refuses([&]{player.reload();});
    player.open("SHEET.TAG");require(player.groups().size()==2 && player.groups()[0].count==3,"Raw groups differ");
    require(player.groups()[0].name==first && player.groups()[1].name==second,"Names after NUL were merged");
    player.select(1,sprites::TimelineMode::pingPong,true);player.advance(150);player.pause(true);
    auto saved=player.timeline().checkpoint();auto retained=player.plan();render::GlBlitter gl;pixels(gl,retained);
    for(const auto& path:{"../sheet.tag","missing.tag","sheet.spr"})refuses([&]{player.open(path);});
    for(unsigned length=0;length<tag.size();++length){save(root/"sheet.tag",{tag.begin(),tag.begin()+length});refuses([&]{player.reload();});
        require(player.timeline().checkpoint().clip==saved.clip && player.timeline().checkpoint().phase==saved.phase,"Failed reload changed state");}
    save(root/"sheet.tag",tag);auto bad=spr;bad[0]^=255;save(root/"sheet.spr",bad);
    refuses([&]{player.reload();});require(player.spritePath()=="SHEET.spr","Corrupt same-stem fell through to terrain");
    save(root/"sheet.spr",spr);player.reload();auto after=player.timeline().checkpoint();
    require(after.clip!=saved.clip && after.phase==saved.phase && after.paused && after.reverse && after.mode==saved.mode,"Reload lost timing or did not replace source");
    require(retained.source!=player.plan().source,"Old plan did not retain old resource");pixels(gl,retained);pixels(gl,player.plan());
    refuses([&]{player.select(2);});refuses([&]{player.seek(4);});
    std::filesystem::remove(root/"sheet.spr");player.reload();require(player.spritePath()=="Terrain.spr","Missing same-stem did not resolve Terrain.spr");
    std::filesystem::remove(root/"sheet.tag");pixels(gl,player.plan());pixels(gl,retained);
    save(root/"sheet.spr",spr);save(root/"sheet.tag",tag);
    std::filesystem::create_directory(root/"Realm");save(root/"Realm"/"Terrain.spr",spr);save(root/"Realm"/"section.tag",tag);
    player.open("Realm\\section.tag");require(player.spritePath()=="Realm/Terrain.spr","Windows separator fallback differs");
    {
        ui::SpriteWindow window(root,"sheet.tag");window.show();require(QTest::qWaitForWindowExposed(&window),"Window not exposed");click(window,"pause");
        require(window.player().timeline().checkpoint().paused,"Pause control did not reach service");
        auto* names=child<QComboBox>(window,"names");require(names->count()==2,"Window did not list exact names");names->setCurrentIndex(1);++windowActions;
        child<QComboBox>(window,"mode")->setCurrentIndex(2);++windowActions;click(window,"reverse");
        require(window.player().timeline().checkpoint().reverse && window.player().timeline().checkpoint().mode==sprites::TimelineMode::pingPong,"Mode/reverse controls failed");
        child<QSlider>(window,"seek")->setValue(2);++windowActions;require(window.player().timeline().position().scheduleEntry==2,"Seek control failed");
        const auto shown=child<QLabel>(window,"preview")->pixmap().toImage();require(!shown.isNull(),"No rendered window image");
        const auto expected=oracle(window.player().plan());const auto word=expected.pixels[121*sprites::previewWidth+159];
        require(word==0xf800 && shown.pixelColor(159,121)==QColor(255,0,0),"Window presentation pixel differs");
        const auto phase=window.player().timeline().checkpoint().phase;save(root/"sheet.tag",{1});click(window,"reload");
        require(window.player().timeline().checkpoint().phase==phase && child<QLabel>(window,"status")->text().contains("Could not"),"Window reload error lost state");
        require(child<QLabel>(window,"preview")->pixmap().toImage()==shown,"Refused reload replaced visible frame");
        save(root/"sheet.tag",tag);click(window,"reload");require(!child<QLabel>(window,"status")->text().contains("Could not"),"Window did not recover after valid reload");
        child<QLineEdit>(window,"tagPath")->setText("../sheet.tag");click(window,"open");require(window.player().tagPath()=="sheet.tag","Bad open changed active document");
        child<QComboBox>(window,"mode")->setCurrentIndex(1);child<QSlider>(window,"seek")->setValue(2);click(window,"pause");QTest::qWait(180);
        require(window.player().timeline().position().finished,"Window timer did not complete one-shot playback");
        window.close();
    }
}
QJsonArray installed(const std::filesystem::path& root){
    std::vector<std::filesystem::path> files;for(const auto& f:std::filesystem::recursive_directory_iterator(root))
        if(f.is_regular_file() && (f.path().extension()==".tag" || f.path().extension()==".TAG"))files.push_back(f.path());
    std::sort(files.begin(),files.end());QJsonArray records;render::GlBlitter gl;bool windows[2]={false,false};
    for(const auto& path:files){const auto relative=path.lexically_relative(root).generic_string();sprites::SpritePlayer player(root);player.open(relative);
        QJsonArray names;const auto table=player.timeline().checkpoint().clip->table();std::map<sprites::Name,unsigned> count;
        for(std::size_t ordinal=0;ordinal<table->sprite()->frames.size();++ordinal){const auto& name=table->sprite()->frames[ordinal].name;
            const auto occurrence=count[name]++;require(table->find(name,occurrence)==ordinal,"Installed exact occurrence differs");
            names.append(QJsonObject{{"name",QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(name.data()),8).toHex())},{"occurrence",int(occurrence)}});}
        for(const auto group:{std::size_t(0),player.groups().size()/2,player.groups().size()-1}){
            player.select(group);player.seek(player.groups()[group].count-1);pixels(gl,player.plan());}
        player.pause(true);const auto phase=player.timeline().checkpoint().phase;player.reload();require(player.timeline().checkpoint().phase==phase,"Installed reload lost selection");
        const unsigned kind=path.stem()!=std::filesystem::path(player.spritePath()).stem();
        if(!windows[kind]) {
            ui::SpriteWindow window(root,QString::fromStdString(relative));window.show();
            require(QTest::qWaitForWindowExposed(&window),"Installed window not exposed");click(window,"pause");
            auto* selector=child<QComboBox>(window,"names");selector->setCurrentIndex(selector->count()-1);++windowActions;
            child<QSlider>(window,"seek")->setValue(int(window.player().groups().back().count-1));++windowActions;
            const auto expected=oracle(window.player().plan());QImage golden(expected.width,expected.height,QImage::Format_RGB32);
            for(int y=0;y<expected.height;++y)for(int x=0;x<expected.width;++x) {
                const auto word=expected.pixels[std::size_t(y)*expected.width+x];
                const unsigned r=word>>11,g=(word>>5)&63,b=word&31;
                golden.setPixel(x,y,qRgb((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2)));
            }
            require(child<QLabel>(window,"preview")->pixmap().toImage()==golden,"Installed TAG to Qt window pixels differ");
            const auto phase=window.player().timeline().checkpoint().phase;click(window,"reload");
            require(window.player().timeline().checkpoint().phase==phase && child<QLabel>(window,"preview")->pixmap().toImage()==golden,
                    "Installed window reload changed paused selection or presentation");
            require(!child<QLabel>(window,"status")->text().contains("Could not"),"Installed window reported an error");
            window.close();windows[kind]=true;++installedWindows;++pixelCases;
        }
        records.append(QJsonObject{{"path",QString::fromStdString(relative)},{"sprite",QString::fromStdString(player.spritePath())},{"entries",names}});
    }require(!records.empty(),"No installed TAG inputs");require(windows[0] && windows[1],"Missing same-stem or terrain installed window journey");return records;
}
}
int main(int argc,char** argv)try {
    QApplication app(argc,argv);synthetic();QJsonArray corpus;
    if(app.arguments().size()==2)corpus=installed(std::filesystem::path(app.arguments()[1].toStdString()));
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"checks",int(checks)},{"refusals",int(refusals)},
        {"gpu_pixel_cases",int(pixelCases)},{"window_actions",int(windowActions)},{"installed_window_journeys",int(installedWindows)},{"installed",corpus}}).toJson(QJsonDocument::Compact).toStdString()<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

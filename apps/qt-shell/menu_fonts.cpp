#include "menu_fonts.hpp"
#include "sft.hpp"
#include <QCryptographicHash>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QMap>
#include <QApplication>
#include <QEvent>
#include <QToolTip>
#include <QVariant>
#include <QWidget>
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace mnm::ui {
namespace {
constexpr int units=32;
void word(QByteArray& b,int v){b.append(char((v>>8)&255));b.append(char(v&255));}
void longWord(QByteArray& b,quint32 v){word(b,int(v>>16));word(b,int(v&65535));}
void pad(QByteArray& b){while(b.size()%4)b.append(char(0));}
quint32 checksum(QByteArray b){pad(b);quint32 s=0;for(qsizetype i=0;i<b.size();i+=4){quint32 w=0;for(int j=0;j<4;++j)w=(w<<8)|quint8(b[i+j]);s+=w;}return s;}
void patchLong(QByteArray& b,int offset,quint32 v){QByteArray w;longWord(w,v);b.replace(offset,4,w);}
struct Point{int x,y;};
struct Glyph{QByteArray bytes;int advance=0,bearing=0,points=0,contours=0;};
Glyph outline(const mnm::assets::SpriteFrame& f,int advance){
    if(f.width>128||f.height>128||std::abs(std::int64_t(f.originX))>128||std::abs(std::int64_t(f.originY))>128||f.opaqueMask.size()!=std::size_t(f.width)*f.height)
        throw std::runtime_error("Menu font glyph outside outline limits");
    std::vector<Point> points;
    for(unsigned y=0;y<f.height;++y)for(unsigned x=0;x<f.width;){
        if(!f.opaqueMask[y*f.width+x]){++x;continue;}
        const unsigned start=x;while(x<f.width&&f.opaqueMask[y*f.width+x])++x;
        const int l=(int(start)-f.originX)*units,r=(int(x)-f.originX)*units,t=(f.originY-int(y))*units,b=t-units;
        // Clockwise, disjoint row rectangles preserve every opaque pixel,
        // including opaque palette index zero. Qt supplies outline antialiasing.
        points.insert(points.end(),{{l,t},{r,t},{r,b},{l,b}});
    }
    if(points.size()>32760)throw std::runtime_error("Menu font contour limit exceeded");
    Glyph g;g.advance=advance*units;g.bearing=-f.originX*units;g.points=int(points.size());g.contours=g.points/4;
    if(points.empty())return g;
    int xmin=points[0].x,xmax=xmin,ymin=points[0].y,ymax=ymin;
    for(auto p:points){xmin=std::min(xmin,p.x);xmax=std::max(xmax,p.x);ymin=std::min(ymin,p.y);ymax=std::max(ymax,p.y);}
    g.bearing=xmin;
    word(g.bytes,g.contours);for(int v:{xmin,ymin,xmax,ymax})word(g.bytes,v);
    for(int i=0;i<g.contours;++i)word(g.bytes,i*4+3);
    word(g.bytes,0);g.bytes.append(QByteArray(g.points,char(1)));
    int previous=0;for(auto p:points){word(g.bytes,p.x-previous);previous=p.x;}
    previous=0;for(auto p:points){word(g.bytes,p.y-previous);previous=p.y;}
    pad(g.bytes);return g;
}
int profileAdvance(const mnm::assets::SftFont& font,unsigned index){
    int advance=0;
    if(index<font.metricGlyphCount)for(unsigned row=0;row<font.rowCount;++row){
        const auto metric=font.rowMetrics[index*font.rowCount+row];
        if(metric.leading < -128||metric.leading>128||metric.trailing < -128||metric.trailing>128)
            throw std::runtime_error("Menu font profile outside limits");
        advance=std::max(advance,metric.trailing);
    }
    else advance=int(font.glyphs.frames[index].width);
    return std::clamp(advance+2,1,256);
}
QByteArray nameTable(const QString& family){
    const std::array<QString,4> values{family,QStringLiteral("Regular"),family,family};
    const std::array<int,4> ids{1,2,4,6};QByteArray table,strings;word(table,0);word(table,4);word(table,6+4*12);
    for(int i=0;i<4;++i){QByteArray value;for(auto c:values[i])word(value,c.unicode());for(int v:{3,1,0x409,ids[i],int(value.size()),int(strings.size())})word(table,v);strings+=value;}
    return table+strings;
}
}
QByteArray menuFontData(const mnm::assets::SftFont& font,const QString& family){
    const auto count=font.glyphs.frames.size();
    if(count==0||count>223||font.ascent<=0||font.descent<0||std::int64_t(font.ascent)+font.descent>128||font.rowCount>128||font.metricGlyphCount>count||font.rowMetrics.size()!=std::size_t(font.rowCount)*font.metricGlyphCount||family.isEmpty()||family.size()>128)
        throw std::runtime_error("Invalid menu font metrics");
    // .notdef is deliberately empty: unsupported Unicode can use Qt font merging.
    std::vector<Glyph> glyphs(2);
    glyphs[0].advance=font.ascent*units/2;
    glyphs[1].advance=profileAdvance(font,unsigned(std::min<std::size_t>(64,count-1)))*units;
    for(unsigned i=0;i<count;++i)glyphs.push_back(outline(font.glyphs.frames[i],profileAdvance(font,i)));
    QMap<QByteArray,QByteArray> tables;QByteArray glyf,loca,hmtx;int maxPoints=0,maxContours=0,maxAdvance=0;
    for(const auto& g:glyphs){longWord(loca,quint32(glyf.size()));glyf+=g.bytes;word(hmtx,g.advance);word(hmtx,g.bearing);maxPoints=std::max(maxPoints,g.points);maxContours=std::max(maxContours,g.contours);maxAdvance=std::max(maxAdvance,g.advance);}
    longWord(loca,quint32(glyf.size()));tables["glyf"]=glyf;tables["loca"]=loca;tables["hmtx"]=hmtx;
    const int em=(font.ascent+font.descent)*units;
    QByteArray head;longWord(head,0x10000);longWord(head,0x10000);longWord(head,0);longWord(head,0x5f0f3cf5);word(head,3);word(head,em);head.append(QByteArray(16,0));
    for(int v:{-128*units,-128*units,256*units,128*units,0,8,2,1,0})word(head,v);
    tables["head"]=head;
    QByteArray hhea;longWord(hhea,0x10000);for(int v:{font.ascent*units,-font.descent*units,0,maxAdvance,-128*units,-128*units,256*units,1,0,0,0,0,0,0,0,int(glyphs.size())})word(hhea,v);tables["hhea"]=hhea;
    QByteArray maxp;longWord(maxp,0x10000);for(int v:{int(glyphs.size()),maxPoints,maxContours,0,0,2,0,0,0,0,0,0,0,0})word(maxp,v);tables["maxp"]=maxp;
    QByteArray os;word(os,0);word(os,maxAdvance/2);word(os,400);word(os,5);word(os,0);
    for(int v:{em/2,em/2,0,0,em/2,em/2,0,0,units,em/3,0})word(os,v);
    os.append(QByteArray(10,0));for(int i=0;i<4;++i)longWord(os,i==0?3:0);os.append("MNM ",4);
    for(int v:{0x40,32,int(count+32),font.ascent*units,-font.descent*units,0,font.ascent*units,font.descent*units})word(os,v);
    tables["OS/2"]=os;
    QByteArray post;longWord(post,0x30000);post.append(QByteArray(28,0));tables["post"]=post;tables["name"]=nameTable(family);
    // Two contiguous format-4 segments: Latin-1 source bytes and sentinel.
    QByteArray sub;for(int v:{4,32,0,4,4,1,0,int(count+32),65535,0,32,65535,-31,1,0,0})word(sub,v);
    QByteArray cmap;word(cmap,0);word(cmap,1);word(cmap,3);word(cmap,1);longWord(cmap,12);cmap+=sub;tables["cmap"]=cmap;
    QByteArray result;longWord(result,0x10000);const int n=int(tables.size());int power=1,exponent=0;while(power*2<=n){power*=2;++exponent;}
    for(int v:{n,power*16,exponent,n*16-power*16})word(result,v);
    QByteArray payload;int headOffset=0;
    for(auto it=tables.cbegin();it!=tables.cend();++it){const int offset=12+n*16+int(payload.size());result+=it.key();longWord(result,checksum(it.value()));longWord(result,offset);longWord(result,quint32(it.value().size()));if(it.key()=="head")headOffset=offset;payload+=it.value();pad(payload);}
    result+=payload;patchLong(result,headOffset+8,0xb1b0afba-checksum(result));return result;
}
class MenuFonts {
public:
    std::array<QString,4> families{};
    std::array<int,4> sizes{32,22,15,14};
    std::vector<int> registrations;
    ~MenuFonts(){if(QGuiApplication::instance())for(auto id:registrations)QFontDatabase::removeApplicationFont(id);}
};
namespace {
class FontOwner final:public QObject {
public:
    FontOwner(QWidget* parent,MenuFontSet value):QObject(parent),fonts(std::move(value)){
        setObjectName("mnmMenuFonts");qApp->installEventFilter(this);
    }
    bool eventFilter(QObject* object,QEvent* event) override {
        if(event->type()==QEvent::ToolTip){
            auto* widget=qobject_cast<QWidget*>(object);
            auto* menu=static_cast<QWidget*>(parent());
            if(widget&&(widget==menu||menu->isAncestorOf(widget)))
                QToolTip::setFont(menuFont(menu,MenuFontRole::Tooltip,std::min(menu->width()/800.0,menu->height()/600.0)));
        }
        return false;
    }
    MenuFontSet fonts;
};
const MenuFonts* fontsFor(const QWidget* widget){
    for(auto* w=widget;w;w=w->parentWidget())for(auto* child:w->children())if(auto* owner=dynamic_cast<FontOwner*>(child))return owner->fonts.get();
    return nullptr;
}
}
MenuFontSet loadMenuFonts(const QString& root){
    auto created=mnm::assets::AssetStore::create(std::filesystem::path(root.toStdString()));
    if(auto* error=std::get_if<mnm::assets::Error>(&created))throw std::runtime_error(error->detail);
    const auto& store=std::get<mnm::assets::AssetStore>(created);
    const std::array<std::string,4> paths{"Sprites/heading text 800.sft","Sprites/body text 800.sft","Sprites/Tooltip Text.sft","Sprites/yellowtext.sft"};
    std::array<QByteArray,4> bytes;std::array<int,4> sizes{};int absent=0;
    for(int i=0;i<4;++i){
        auto opened=store.open(paths[i]);
        if(auto* error=std::get_if<mnm::assets::Error>(&opened)){if(error->code==mnm::assets::ErrorCode::notFound){++absent;continue;}throw std::runtime_error(paths[i]+": "+error->detail);}
        mnm::assets::SftLimits limits;limits.glyphs.inputBytes=2*1024*1024;limits.glyphs.frames=223;limits.glyphs.width=128;limits.glyphs.height=128;limits.glyphs.pixels=1024*1024;limits.glyphs.decodedBytes=4*1024*1024;limits.metricsBytes=512*1024;
        auto decoded=mnm::assets::loadSft(*std::get<std::unique_ptr<mnm::assets::AssetFile>>(opened),limits);
        if(auto* error=std::get_if<mnm::assets::SftError>(&decoded))throw std::runtime_error(paths[i]+": "+error->detail);
        const auto& font=std::get<mnm::assets::SftFont>(decoded);sizes[i]=font.ascent+font.descent;
        // Content-derived names avoid collisions between different installation roots.
        bytes[i]=menuFontData(font,QString("MNM%1").arg(i));
        const auto digest=QCryptographicHash::hash(bytes[i],QCryptographicHash::Sha256).toHex().left(16);
        bytes[i]=menuFontData(font,QString("MNM%1%2").arg(i).arg(QString::fromLatin1(digest)));
    }
    if(absent==4)return std::make_shared<MenuFonts>();
    if(absent)throw std::runtime_error("Incomplete menu SFT font catalog");
    QByteArray key;for(const auto& b:bytes)key+=QCryptographicHash::hash(b,QCryptographicHash::Sha256);
    static QMap<QByteArray,std::weak_ptr<const MenuFonts>> cache;
    if(auto existing=cache.value(key).lock())return existing;
    for(auto it=cache.begin();it!=cache.end();)if(it.value().expired())it=cache.erase(it);else ++it;
    auto fonts=std::make_shared<MenuFonts>();fonts->sizes=sizes;
    for(int i=0;i<4;++i){const int id=QFontDatabase::addApplicationFontFromData(bytes[i]);if(id<0)throw std::runtime_error("Qt rejected converted menu font");fonts->registrations.push_back(id);const auto families=QFontDatabase::applicationFontFamilies(id);if(families.size()!=1)throw std::runtime_error("Invalid converted menu font family");fonts->families[i]=families[0];}
    cache[key]=fonts;return fonts;
}
void installMenuFonts(QWidget* widget,MenuFontSet fonts){
    auto* owner=new FontOwner(widget,std::move(fonts));
    const auto children=widget->children();for(auto* child:children)if(child!=owner&&dynamic_cast<FontOwner*>(child))delete child;
    widget->setProperty("menuFontSource",owner->fonts->registrations.empty()?"fallback":"SFT");
    const auto body=menuFont(widget,MenuFontRole::Body);
    widget->setFont(body);
    // Qt scroll-area helpers do not always inherit the surrounding font.
    // Seed existing descendants; each screen then assigns its explicit roles.
    for(auto* child:widget->findChildren<QWidget*>())child->setFont(body);
}
QFont menuFont(const QWidget* widget,MenuFontRole role,double scale){
    const int i=int(role);const auto* fonts=fontsFor(widget);QFont font(fonts&&!fonts->families[i].isEmpty()?fonts->families[i]:QStringLiteral("serif"));
    const std::array<int,4> defaults{32,22,15,14};const int size=fonts?fonts->sizes[i]:defaults[i];
    font.setPixelSize(qMax(1,qRound(size*std::clamp(scale,0.05,8.0))));font.setBold(false);font.setKerning(false);return font;
}
}

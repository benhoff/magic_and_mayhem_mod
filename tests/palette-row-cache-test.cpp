#include "../renderer/blit.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::array<std::uint16_t,256> colours(unsigned n){std::array<std::uint16_t,256> p{};for(unsigned i=0;i<256;++i)p[i]=std::uint16_t(n*997+i*257);return p;}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);const bool cached=qEnvironmentVariableIntValue("MNM_PALETTE_ROW_CACHE_BASELINE")!=1;GlBlitter renderer;Image base{8,4,std::vector<std::uint32_t>(32,0x2345)};auto expected=base;
 const auto destination=renderer.create(base,{16,{0xf800,0x7e0,31}}),source=renderer.create({4,1,{0,17,128,255}},{16,{0xf800,0x7e0,31}}),mask=renderer.create({4,1,{1,1,0,1}},{8,{}});
 unsigned checks=0;bool coverMiddle=false;
 const auto draw=[&](unsigned n,int x,int y){auto p=colours(n);renderer.composite(source,destination,{0,0,4,1},x,y,mask,{},&p);const unsigned indices[]{0,17,128,255};for(unsigned i=0;i<4;++i)if(i!=2||coverMiddle)expected.pixels[std::size_t(y)*8+x+i]=p[indices[i]];};
 const auto check=[&]{require(renderer.read(destination).pixels==expected.pixels,"Palette cache changed independently calculated ordered/masked native words");++checks;};
 renderer.batch([&]{for(unsigned n=0;n<32;++n)draw(n,n%3,n%4);});check();
 const auto before=renderer.stats();renderer.batch([&]{for(unsigned n=0;n<32;++n)draw(n,n%3,n%4);});auto after=renderer.stats();if(cached)require(after.paletteTextureUploadCalls==before.paletteTextureUploadCalls&&after.paletteCacheHits-before.paletteCacheHits==32,"Warm palette rows were uploaded or not reused");else require(after.paletteTextureUploadCalls>before.paletteTextureUploadCalls&&!after.paletteCacheHits,"Baseline palette path was not exercised");check();
 // A single batch exceeds row capacity; eviction must flush every queued reader
 // before a GPU palette row is overwritten, including overlapping destinations.
 // The sentinel's last pixel is outside every later draw. Its palette must
 // still be sampled correctly when its queued row is evicted in this batch.
 renderer.batch([&]{draw(42,4,0);for(unsigned n=100;n<1300;++n)draw(n,n%3,n%4);});check();if(cached)require(renderer.stats().paletteCacheRows==1024&&renderer.stats().paletteCacheEvictions>0,"Palette row storage/eviction bound");else require(!renderer.stats().paletteCacheRows,"Baseline retained cache rows");
 renderer.batch([&]{for(unsigned n=0;n<32;++n)draw(n,n%3,n%4);});check();
 renderer.update(mask,0,0,{4,1,{1,1,1,1}});coverMiddle=true;renderer.batch([&]{draw(0,1,2);});check();
 auto changed=colours(0);changed[128]^=65535;auto prior=renderer.stats();renderer.batch([&]{renderer.composite(source,destination,{0,0,4,1},1,2,mask,{},&changed);});expected.pixels[2*8+3]=changed[128];check();if(cached)require(renderer.stats().paletteCacheMisses==prior.paletteCacheMisses+1,"Owned exact key ignored palette mutation");
 const auto stable=renderer.read(destination).pixels;bool refused=false;try{renderer.batch([&]{renderer.composite(source,destination,{0,0,5,1},0,0,mask,{},&changed);});}catch(const std::exception&){refused=true;}require(refused&&renderer.read(destination).pixels==stable,"Bad rectangle changed completed native pixels");
 const auto stats=renderer.stats();renderer.destroy(source);renderer.destroy(mask);renderer.destroy(destination);require(!renderer.stats().surfaces,"Palette fixture leaked native surfaces");
 std::cout<<QJsonDocument(QJsonObject{{"success",true},{"cache_enabled",cached},{"independent_pixel_checks",int(checks)},{"words_per_check",32},{"hits",double(stats.paletteCacheHits)},{"misses",double(stats.paletteCacheMisses)},{"evictions",double(stats.paletteCacheEvictions)},{"rows",double(stats.paletteCacheRows)},{"warm_palette_uploads",double(after.paletteTextureUploadCalls-before.paletteTextureUploadCalls)},{"refusals",1},{"remaining_surfaces",0}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

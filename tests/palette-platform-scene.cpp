#include "palette_binding.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <cstring>
#include <fstream>
#include <iostream>
using namespace mnm;
using resource_test::require;
using resource_test::rejected;
using Bytes=std::vector<std::uint8_t>;
static Bytes readFile(const std::filesystem::path& path){
 std::ifstream in(path,std::ios::binary);require(bool(in),"Input file");
 return Bytes(std::istreambuf_iterator<char>(in),{});
}
struct Reader {
 Bytes bytes;std::size_t at=0;
 unsigned word(){require(at+4<=bytes.size(),"Oracle word extent");unsigned v=0;for(unsigned i=0;i<4;++i)v|=unsigned(bytes[at++])<<(i*8);return v;}
 reconstruction::PaletteRgb rgb(){require(at+768<=bytes.size(),"Oracle RGB extent");reconstruction::PaletteRgb v;std::memcpy(v.data(),bytes.data()+at,768);at+=768;return v;}
};
struct Expected {unsigned shift,neutral;reconstruction::PaletteRgb source;std::vector<reconstruction::PaletteWords> tables;};
int main(int argc,char** argv)try{
 QGuiApplication application(argc,argv);require(argc==3,"Expected input directory and oracle");
 const auto root=std::filesystem::path(argv[1]);Reader oracle{readFile(argv[2])};
 require(oracle.word()==0x31504650,"Palette oracle version");const auto count=oracle.word();require(count>=1&&count<=4,"Oracle palettes");
 std::vector<Expected> expected;
 for(unsigned f=0;f<2;++f)for(unsigned p=0;p<count;++p)for(unsigned m=0;m<10;++m){
  require(oracle.word()==f&&oracle.word()==p&&oracle.word()==m,"Original case identity");Expected e;e.shift=oracle.word();e.neutral=oracle.word();const auto n=oracle.word();require(n==(m<8?15u:8u),"Original table count");e.source=oracle.rgb();
  e.tables.resize(n);for(auto& table:e.tables)for(auto& colour:table){const auto v=oracle.word();require(v<=65535,"Original packed word");colour=std::uint16_t(v);}expected.push_back(e);
 }
 require(oracle.at==oracle.bytes.size(),"No oracle trailing data");
 const auto bytes=std::filesystem::exists(root/"cfg/tables.cfg")?readFile(root/"cfg/tables.cfg"):Bytes{};
 const auto snapshot=assets::ProfileSnapshot::parse(bytes);const auto profile=scene::paletteEffectProfile(snapshot);
 Reader platform{readFile(root/"profile.bin")};platform.at=260;
 const char* sections[]={"STONE_EFFECT","STONE_EFFECT","STONE_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","REDTINT_EFFECT"};
 const char* keys[]={"Reducer","SpecalValue","Darkest","Reducer","SpecalValue","Darkest","BlueTint","Contrast","Reducer"};
 for(unsigned i=0;i<9;++i){const auto n=platform.word();require(n<256&&platform.at+256<=platform.bytes.size(),"Platform string bound");const std::string text(reinterpret_cast<const char*>(platform.bytes.data()+platform.at),n);require(snapshot.value(sections[i],keys[i],256)==text,"Native profile differs from actual Wine service");platform.at+=256;}
 assets::ResourceManager resources(resource_test::store(root));const assets::ResourceId id{assets::ResourceKind::effect,"platform-palette"};auto recipe=resource_test::spr("atlas.spr");recipe.animationBytes=resource_test::animation();resources.bind(id,recipe);resources.load(id);
 const auto& sprite=std::get<assets::Sprite>(resources.resident(id).image);const auto inputs=scene::paletteSources(sprite);require(inputs.size()==count,"Independent SPR palette count");
 scene::ScenePaletteBindings binding(resources);binding.bind(id);render::GlBlitter renderer;unsigned frames=0,words=0,refusals=0;
 {
  const render::Image background{16,16,std::vector<std::uint32_t>(256,0x1234)};render::SceneRenderer output(renderer,resources,background);
  for(unsigned format=0;format<2;++format){reconstruction::PaletteEffectChain chain(inputs);
   for(unsigned mode=0;mode<10;++mode){const reconstruction::PaletteShadingConfig lighting{16,30,40,1,1};require(chain.append(mode,lighting,profile,format!=0)==mode,"Native chain ordinal");if(!format)require(binding.append(id,mode,lighting,profile)==mode,"Resource chain ordinal");
    for(unsigned p=0;p<count;++p){const auto& e=expected[format*count*10+p*10+mode];const auto& actual=chain.tableSet(mode,p);require(actual.shift==e.shift&&actual.neutral==e.neutral&&actual.tables==e.tables,"Complete independent original/native table mismatch");require(chain.source(p)==e.source,"Original/native metal mutation mismatch");words+=unsigned(e.tables.size()*256);
     if(!format)for(int shade:{-112,0,mode<8?112:0}){render::SceneDraw draw{id,p,0,0};binding.apply(draw,mode,shade);
      // Independently select the original table with signed arithmetic shift.
      const int q=shade<0?-int((unsigned(-shade)+(1u<<e.shift)-1)>>e.shift):shade>>e.shift;
      const int index=int(e.neutral)+q+(q<0&&e.shift?1:0);require(index>=0&&unsigned(index)<e.tables.size(),"Original shade admission");const auto& colours=e.tables[unsigned(index)];require(draw.colours==colours,"Resource palette selection");
      const auto before=renderer.stats();output.draw({draw});const auto after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Draw consumed original pixels");const auto image=output.read();for(unsigned c=0;c<256;++c)require(image.pixels[c]==colours[c],"Independent original/GPU colour mismatch");++frames;
     }
    }
   }
  }
  // Every failure preserves the published tables, source and budget; a valid
  // append afterwards proves the failed transaction did not poison the owner.
  scene::PaletteBindingLimits limits;limits.perResource.effects=1;scene::ScenePaletteBindings bounded(resources,limits);bounded.bind(id);bounded.append(id,0,{2,30,40,1,1},profile);
  render::SceneDraw draw{id,0,0,0};bounded.apply(draw,0,0);const auto old=draw.colours;const auto charge=bounded.retainedBytes();
  rejected([&]{bounded.append(id,5,{},profile);});++refusals;require(bounded.retainedBytes()==charge,"Failed append budget mutation");bounded.apply(draw,0,0);require(draw.colours==old,"Failed append changed published colours");
  rejected([&]{binding.append(id,2,{17,30,40,1,1},profile);});++refusals;
  for(const auto* text:{"nan","inf","-inf","1001","-1001","1e9999"}){const std::string cfg=std::string("[REDTINT_EFFECT]\nReducer=")+text+"\n";rejected([&]{scene::paletteEffectProfile(assets::ProfileSnapshot::parse(Bytes(cfg.begin(),cfg.end())));});++refusals;}
  binding.clear();bounded.clear();require(!binding.retainedBytes()&&!bounded.retainedBytes(),"Palette retirement");
 }
 resources.unloadAll();require(renderer.stats().surfaces==0,"GPU retirement");
 std::cout<<"{\"success\":true,\"modes\":10,\"palettes\":"<<count<<",\"table_words\":"<<words<<",\"gpu_frames\":"<<frames<<",\"compared_pixels\":"<<frames*256<<",\"profile_fields\":9,\"refusals\":"<<refusals<<",\"remaining_surfaces\":0}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

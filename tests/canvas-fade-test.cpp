#include "../renderer/canvas_sequence.hpp"
#include "../reconstruction/rendering/fade_span.hpp"
#include <iostream>
static void require(bool v){if(!v)throw std::runtime_error("Fade test assertion");}
struct Mirror final : mnm::render::CanvasMirror {
  mnm::render::Image image;
  void create(std::uint32_t,int w,int h) override {image={w,h,std::vector<std::uint32_t>(std::size_t(w)*h,0x1234)};}
  void release(std::uint32_t) override {}
  void update(std::uint32_t id,int x,int y,const mnm::render::Image &patch) override {
    updateMasked(id,x,y,patch,std::vector<std::uint8_t>(patch.pixels.size(),1));
  }
  void updateMasked(std::uint32_t,int x,int y,const mnm::render::Image &patch,const std::vector<std::uint8_t> &mask) override {
    require(x>=0 && y>=0 && x+patch.width<=image.width && y+patch.height<=image.height && mask.size()==patch.pixels.size());
    for(int j=0;j<patch.height;++j)for(int i=0;i<patch.width;++i) {
      const auto at=std::size_t(j)*patch.width+i;
      if(mask[at])image.pixels[std::size_t(y+j)*image.width+x+i]=patch.pixels[at];
    }
  }
  void copy(std::uint32_t,std::uint32_t,mnm::render::Rect,int,int,std::optional<std::uint16_t>) override {throw std::runtime_error("Unexpected fade mirror copy");}
};
int main() try {
  using mnm::render::CanvasSequence;using mnm::reconstruction::rendering::fadePhysicalWords;
  unsigned refusals=0;
  CanvasSequence c;c.create(1,3,3);c.fill(1,{0,0,3,2},0xabcd);
  try{c.fade(1,3,8);throw std::runtime_error("Missing unknown refusal");}catch(const std::runtime_error &e){if(std::string(e.what())=="Missing unknown refusal")throw;++refusals;}
  c.fill(1,{0,2,3,3},0x9876);const auto before=c.read(1).pixels;
  require(before[0]==0xabcd); // Late undefined input cannot partially dim earlier words.
  for(auto invalid : {0u,2u,4097u}){
    try{c.fade(1,invalid,10);throw std::runtime_error("Missing extent refusal");}catch(const std::invalid_argument &){++refusals;}
    require(c.read(1).pixels==before);
  }
  c.fade(1,5,6);auto pixels=c.read(1).pixels;
  require(pixels[0]==((0xabcd>>1)&0x7bef) && pixels[3]==((0xabcd>>1)&0x7bef) && pixels[4]==0xabcd && pixels[6]==0x9876);
  CanvasSequence untouched;untouched.create(1,1,2);untouched.fade(1,2,0);untouched.fill(1,{0,0,1,2},0x9988);require(untouched.read(1).pixels[0]==0x9988);
  std::vector<std::uint16_t> plane(9,0xabcd);const auto original=plane;
  for(auto dims: {std::array<unsigned,3>{0,3,3},{3,0,3},{2049,3,2049},{3,3,2},{3,3,4097},{3,2,3}}){
    try{fadePhysicalWords(plane,dims[0],dims[1],dims[2],0);throw std::runtime_error("Missing model refusal");}catch(const std::invalid_argument &){++refusals;}
    require(plane==original);
  }
  unsigned mirrored=0;
  for(unsigned w : {1u,3u,8u,13u})for(unsigned stride : {w,w+3})for(std::uint16_t mask : {0x7bef,0x7def})
    for(std::size_t count : {std::size_t(0),std::size_t(1),std::size_t(w-1),std::size_t(w),std::size_t(w+1),std::size_t(stride),std::size_t(2*stride+2),std::size_t(3*stride)}) {
      if(count>3*stride)continue;
      Mirror mirror;CanvasSequence canvas(&mirror);canvas.create(1,w,3);
      std::vector<std::uint32_t> expected(3*w);
      for(unsigned y=0;y<3;++y)for(unsigned x=0;x<w;++x) {
        const auto colour=std::uint16_t((y*w+x)*1273+0xabcd);
        canvas.fill(1,{int(x),int(y),int(x+1),int(y+1)},colour);
        expected[y*w+x]=colour;
      }
      canvas.publishWrites();require(mirror.image.pixels==expected);
      for(unsigned y=0;y<3;++y)for(unsigned x=0;x<w;++x)
        if(std::size_t(y)*stride+x<count)expected[y*w+x]=(std::uint16_t(expected[y*w+x])>>1)&mask;
      canvas.fade(1,stride,count,mask);canvas.publishWrites();
      require(canvas.read(1).pixels==expected && mirror.image.pixels==expected);++mirrored;
    }
  // Unknown pixels outside the physical prefix stay untouched in the mirror;
  // a later refusal must not dim the earlier initialized row or publish damage.
  Mirror mirror;CanvasSequence partial(&mirror);partial.create(1,3,3);
  partial.fill(1,{0,0,3,1},0xabcd);partial.publishWrites();
  partial.fade(1,5,5);partial.publishWrites();auto snapshot=mirror.image.pixels;
  require(snapshot[0]==((0xabcd>>1)&0x7bef) && snapshot[3]==0x1234);
  try{partial.fade(1,5,6);throw std::runtime_error("Missing mirror refusal");}catch(const std::runtime_error &e){if(std::string(e.what())=="Missing mirror refusal")throw;++refusals;}
  partial.publishWrites();require(mirror.image.pixels==snapshot);
  partial.fill(1,{0,1,3,3},0x9876);require(partial.read(1).pixels[0]==snapshot[0]);
  std::cout<<"{\"success\":true,\"atomic_refusals\":"<<refusals<<",\"mirrored_cases\":"<<mirrored<<"}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}

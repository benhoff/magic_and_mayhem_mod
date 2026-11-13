#include "../renderer/blit.hpp"
#include "../renderer/surface_copy.hpp"
#include "../renderer/scenes/scene_renderer.hpp"
#include "scene-resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static Image base(PixelFormat f,int w=31,int h=23){
    Image a{w,h,{}};const auto bits=f.bits==32?UINT32_MAX:((1u<<f.bits)-1);
    for(int i=0;i<w*h;++i)a.pixels.push_back(i%7<3?bits:((unsigned(i)*2654435761u)&bits));
    return a;
}
static unsigned keyed(GlBlitter& renderer){
    unsigned cases=0;
    for(auto format:{PixelFormat{8,{}},PixelFormat{16,{0xf800,0x7e0,0x1f}},PixelFormat{16,{0x7c00,0x3e0,0x1f}},PixelFormat{32,{0xff0000,0xff00,0xff}}})
        for(auto delta:{std::array<int,2>{0,0},{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,1},{1,-1},{-1,-1},{9,3},{-9,3}})
            for(bool batched:{false,true})for(bool clipped:{false,true}){
                const auto input=base(format);auto expected=input;
                const auto surface=renderer.create(input,format);
                const auto keyMask=format.bits==8?255u:format.masks[0]|format.masks[1]|format.masks[2];
                const auto key=keyMask;
                ClipperState clip;
                if(clipped){clip={true,std::vector<Rect>{{3,3,13,12},{13,4,27,17},{5,17,25,21}}};renderer.setClipper(surface,clip);}
                SurfaceCopyRequest request;request.source={10,7,20,15};request.destination={10+delta[0],7+delta[1],20+delta[0],15+delta[1]};
                const auto plan=planSurfaceCopy(input.width,input.height,input.width,input.height,clip,request);
                for(const auto& p:plan.pieces)for(int y=p.source.top;y<p.source.bottom;++y)for(int x=p.source.left;x<p.source.right;++x){
                    const auto word=expected.pixels[y*input.width+x];
                    if((word&keyMask)!=key)expected.pixels[(p.y+y-p.source.top)*input.width+p.x+x-p.source.left]=word;
                }
                const auto before=renderer.stats();
                auto copy=[&]{require(renderer.surfaceCopy(surface,surface,request,key).hresult==plan.hresult,"Keyed result differs");};
                if(batched)renderer.batch(copy);else copy();
                const auto after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks,"Keyed ordinary readback");
                require(renderer.read(surface).pixels==expected.pixels,"Keyed chain differs from in-place scalar");
                renderer.destroy(surface);++cases;
            }
    // Long single-axis chains, holes, unused bits and the existing atomic cap.
    for(auto shape:{std::array<int,2>{2048,2},{2,2048},{65,65}}){
        const PixelFormat format{16,{0x7c00,0x3e0,0x1f}};auto input=base(format,shape[0],shape[1]),expected=input;
        const auto surface=renderer.create(input,format);SurfaceCopyRequest request;
        const bool horizontal=shape[0]>shape[1];request.source={0,0,shape[0]-(horizontal?1:0),shape[1]-(horizontal?0:1)};
        request.destination={horizontal?1:0,horizontal?0:1,shape[0],shape[1]};
        if(shape[0]==65){bool refused=false;try{renderer.surfaceCopy(surface,surface,request,0x7fff);}catch(const std::exception&){refused=true;}
            require(refused,"Keyed capacity guard missing");}
        else for(int y=0;y<request.source.bottom;++y)for(int x=0;x<request.source.right;++x){auto word=expected.pixels[y*input.width+x];if((word&0x7fff)!=0x7fff)expected.pixels[(y+request.destination.top)*input.width+x+request.destination.left]=word;}
        if(shape[0]!=65)renderer.surfaceCopy(surface,surface,request,0x7fff);
        require(renderer.read(surface).pixels==expected.pixels,"Long chain/cap pixels differ");renderer.destroy(surface);++cases;
    }
    return cases;
}
static unsigned blends(GlBlitter& renderer){
    unsigned cases=0;
    for(bool rgb555:{false,true}){
        const PixelFormat format=rgb555?PixelFormat{16,{0x7c00,0x3e0,0x1f}}:PixelFormat{16,{0xf800,0x7e0,0x1f}};
        auto expected=base(format,257,256);const auto dst=renderer.create(expected,format);
        auto srcImage=base(format,17,13);Image maskImage{17,13,{}};for(int i=0;i<17*13;++i)maskImage.pixels.push_back(i%5?255:0);
        const auto src=renderer.create(srcImage,format),mask=renderer.create(maskImage,{8,{}});
        const unsigned m=rgb555?0x3def:0x7bef;
        renderer.batch([&]{for(unsigned i=0;i<512;++i){
            const auto mode=i%3==0?CompositeMode::half:(i%3==1?CompositeMode::quarterSource:CompositeMode::quarterDestination);
            const int x=int((i*31)%240),y=int((i*37)%243);SpriteComposite c;c.mode=mode;renderer.composite(src,dst,{0,0,17,13},x,y,mask,c);
            for(int row=0;row<13;++row)for(int col=0;col<17;++col){const auto at=row*17+col;if(!maskImage.pixels[at])continue;
                auto s=srcImage.pixels[at],&d=expected.pixels[(y+row)*257+x+col];
                if(mode==CompositeMode::half)d=((s>>1)&m)+((d>>1)&m);
                else{if(mode==CompositeMode::quarterDestination)std::swap(s,d);const auto half=(d>>1)&m;d=half+((half>>1)&m)+((s>>2)&(m>>1)&m);}
            }++cases;
        }});
        require(renderer.read(dst).pixels==expected.pixels,"Blend differs from scalar");renderer.destroy(src);renderer.destroy(mask);renderer.destroy(dst);
    }
    return cases;
}
static unsigned rows(GlBlitter& renderer){
    mnm::test::SceneResources resources;unsigned cases=0;
    for(unsigned height:{200u,225u}){
        mnm::assets::Sprite sprite;sprite.storage=mnm::assets::SpriteStorage::indexed8;sprite.palettes.resize(1);
        mnm::assets::SpriteFrame f;f.width=300;f.height=height;f.paletteIndex=0;
        f.opaqueMask.assign(300*height,1);f.pixels=std::vector<std::uint8_t>(300*height,1);sprite.frames.push_back(f);
        const auto id=resources.add("dense"+std::to_string(height),sprite);
        auto input=base({16,{0xf800,0x7e0,0x1f}},300,230),expected=input;
        SceneRenderer scene(renderer,resources.manager,input);
        SceneDraw draw{id,0,0,0};draw.rowEffect=SpriteRowEffect{RowEffectKind::sampleDestination,std::vector<std::int32_t>(height,32),false};
        std::vector<SceneDraw> draws{draw,draw};
        for(const auto& d:draws){(void)d;const auto old=expected;for(unsigned y=0;y<height;++y)for(unsigned x=0;x<300;++x)expected.pixels[y*300+x]=old.pixels[y*300+x+32];}
        const auto before=renderer.stats();scene.draw(draws);const auto after=renderer.stats();
        require(before.nativeReadbacks==after.nativeReadbacks,"Row ordinary readback");
        if(height==225)require(after.nativeSnapshotCopies-before.nativeSnapshotCopies==2,"Oversize row did not preserve clone fallback");
        require(scene.read().pixels==expected.pixels,"Row wrap/fallback history differs");
        scene.releaseCachedImages();scene.draw(draws);require(scene.read().pixels==expected.pixels,"Row cache release differs");++cases;
    }
    return cases;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);unsigned cases=0;
    for(bool baseline:{false,true}){
        qputenv("MNM_NATIVE_FEEDBACK_BASELINE",baseline?"1":"0");GlBlitter renderer;
        cases+=keyed(renderer);cases+=blends(renderer);cases+=rows(renderer);require(renderer.stats().surfaces==0,"Effect surface leak");
    }
    std::cout<<cases<<" native optimization scalar comparisons pass (feedback and forced snapshot)\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

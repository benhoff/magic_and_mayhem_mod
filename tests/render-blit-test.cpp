#include "blit.hpp"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <array>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

using namespace mnm::render;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static Image reference(const Blit& c){
    auto out=c.destination;
    for(int sy=c.sourceRect.top;sy<c.sourceRect.bottom;++sy)
        for(int sx=c.sourceRect.left;sx<c.sourceRect.right;++sx){
            const auto value=c.source.pixels[sy*c.source.width+sx];
            if(c.sourceKey && value==*c.sourceKey)continue;
            const auto dx=c.destinationX+sx-c.sourceRect.left,dy=c.destinationY+sy-c.sourceRect.top;
            out.pixels[dy*out.width+dx]=value;
        }
    return out;
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try {
        // Simulate an existing Qt viewport context and verify that it survives
        // constructing, drawing with and destroying the independent blitter.
        QOpenGLContext caller;QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
        caller.setFormat(format);require(caller.create(),"Cannot create caller context");
        QOffscreenSurface surface;surface.setFormat(caller.format());surface.create();
        require(caller.makeCurrent(&surface),"Cannot make caller context current");
        caller.functions()->glViewport(7,8,90,100);
        unsigned count=0;
        {
            GlBlitter renderer;
            require(QOpenGLContext::currentContext()==&caller,"Constructor changed caller context");
            const auto driver=renderer.driver();std::cout<<driver.vendor<<" / "<<driver.renderer<<" / "<<driver.version<<'\n';
            std::mt19937 rng(0x4d4e4d);
            for(unsigned bits:{8u,16u,24u,32u})for(unsigned mode=0;mode<4;++mode)for(int sample=0;sample<12;++sample){
                const auto mask=bits==32?UINT32_MAX:((std::uint32_t{1}<<bits)-1);
                Blit c;c.bits=bits;c.source={7,5,std::vector<std::uint32_t>(35)};
                c.destination={11,9,std::vector<std::uint32_t>(99)};
                for(auto& p:c.source.pixels)p=rng()&mask;
                for(auto& p:c.destination.pixels)p=rng()&mask;
                if(mode)c.sourceKey=mode==1?0:mode==2?mask:(rng()&mask);
                if(c.sourceKey)for(std::size_t i=0;i<c.source.pixels.size();++i)
                    if(sample==0 || i%3==0)c.source.pixels[i]=*c.sourceKey;
                const int left=int(rng()%7),top=int(rng()%5);
                c.sourceRect={left,top,left+1+int(rng()%unsigned(7-left)),top+1+int(rng()%unsigned(5-top))};
                const int width=c.sourceRect.right-left,height=c.sourceRect.bottom-top;
                c.destinationX=int(rng()%unsigned(12-width));c.destinationY=int(rng()%unsigned(10-height));
                require(renderer.draw(c).pixels==reference(c).pixels,"Integer draw differs from CPU reference");
                require(QOpenGLContext::currentContext()==&caller,"Draw changed caller context");++count;
            }
            Blit c;c.bits=32;c.source={256,256,std::vector<std::uint32_t>(256*256,0x80000001)};
            c.destination={2048,2048,std::vector<std::uint32_t>(2048*2048,0xffffffff)};
            c.sourceRect={0,0,256,256};c.destinationX=c.destinationY=1792;
            require(renderer.draw(c).pixels==reference(c).pixels,"Maximum-size corner copy differs");++count;
            c.source={1,1,{0xffffffff}};c.destination={1,1,{0x12345678}};c.sourceRect={0,0,1,1};
            c.destinationX=c.destinationY=0;c.sourceKey=0xffffffff;
            require(renderer.draw(c).pixels==c.destination.pixels,"All-transparent draw changed destination");++count;
            c.sourceKey.reset();require(renderer.draw(c).pixels==c.source.pixels,"Opaque unsigned pixel was lost");++count;
            const auto reject=[&renderer](const Blit& bad){
                bool rejected=false;try{renderer.draw(bad);}catch(const std::runtime_error&){rejected=true;}
                require(rejected,"Invalid command accepted");
            };
            auto bad=c;bad.destinationX=std::numeric_limits<int>::max();reject(bad);
            bad=c;bad.sourceRect.left=-1;reject(bad);
            bad=c;bad.source.pixels.clear();reject(bad);
            bad=c;bad.bits=8;reject(bad);
            bad=c;bad.bits=15;reject(bad);
            // Failure must not poison the reusable context or subsequent calls.
            require(renderer.draw(c).pixels==c.source.pixels,"Draw after rejection failed");++count;
        }
        require(QOpenGLContext::currentContext()==&caller,"Destructor changed caller context");
        std::array<GLint,4> viewport{};caller.functions()->glGetIntegerv(GL_VIEWPORT,viewport.data());
        require(viewport==std::array<GLint,4>{7,8,90,100},"Caller viewport state changed");
        std::cout<<count<<" OpenGL draws match; command guards and caller context preserved\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}

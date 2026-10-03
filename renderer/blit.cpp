#include "blit.hpp"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QThread>
#include <algorithm>
#include <atomic>
#include <unordered_map>
#include <stdexcept>

namespace mnm::render {
void validate(const Blit& c){
    if(c.bits!=8 && c.bits!=16 && c.bits!=24 && c.bits!=32)
        throw std::runtime_error("Unsupported native pixel size");
    const auto maximum=c.bits==32?UINT32_MAX:((std::uint32_t{1}<<c.bits)-1);
    const auto image=[maximum](const Image& i,int limit){
        if(i.width<1 || i.height<1 || i.width>limit || i.height>limit ||
           i.pixels.size()!=std::size_t(i.width)*std::size_t(i.height))
            throw std::runtime_error("Invalid surface dimensions or pixel count");
        if(std::any_of(i.pixels.begin(),i.pixels.end(),[maximum](auto p){return p>maximum;}))
            throw std::runtime_error("Native pixel exceeds its format");
    };
    image(c.source,256);image(c.destination,2048);
    const auto r=c.sourceRect;
    if(r.left<0 || r.top<0 || r.right<=r.left || r.bottom<=r.top ||
       r.right>c.source.width || r.bottom>c.source.height)
        throw std::runtime_error("Source rectangle is out of bounds");
    if(c.destinationX<0 || c.destinationY<0 || c.destinationX>c.destination.width-(r.right-r.left) ||
       c.destinationY>c.destination.height-(r.bottom-r.top))
        throw std::runtime_error("Destination rectangle is out of bounds");
    if(c.sourceKey && *c.sourceKey>maximum)throw std::runtime_error("Source key exceeds its format");
}
namespace {
// Preserve the caller's context; the blitter never changes viewport GL state.
struct Current {
    QOpenGLContext& context;
    QOpenGLContext* previous=QOpenGLContext::currentContext();
    QSurface* surface=previous?previous->surface():nullptr;
    Current(QOpenGLContext& context,QSurface* target):context(context){
        if(!context.makeCurrent(target))throw std::runtime_error("Cannot make the OpenGL context current");
    }
    ~Current(){if(previous && surface)previous->makeCurrent(surface);else context.doneCurrent();}
};
constexpr auto vertexSource=R"(#version 330 core
void main(){
    vec2 p[3]=vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3));
    gl_Position=vec4(p[gl_VertexID],0,1);
})";
constexpr auto fragmentSource=R"(#version 330 core
uniform usampler2D sourcePixels;
uniform ivec2 sourceOrigin;
uniform ivec2 destinationOrigin;
uniform bool hasKey;
uniform uint sourceKey;
layout(location=0) out uint nativePixel;
void main(){
    ivec2 at=ivec2(gl_FragCoord.xy)-destinationOrigin+sourceOrigin;
    uint pixel=texelFetch(sourcePixels,at,0).r;
    if(hasKey && pixel==sourceKey)discard;
    nativePixel=pixel;
})";
}
namespace {
constexpr auto presentationSource=R"(#version 330 core
uniform usampler2D nativePixels;
uniform usampler2D palette;
uniform bool indexed;
uniform uvec3 masks;
uniform uvec3 lowBits;
uniform uvec3 maxima;
layout(location=0) out uvec4 color;
void main(){
    uint pixel=texelFetch(nativePixels,ivec2(gl_FragCoord.xy),0).r;
    if(indexed)color=texelFetch(palette,ivec2(int(pixel),0),0);
    else color=uvec4(((uvec3(pixel)&masks)/lowBits)*255u/maxima,255u);
})";
void validateImage(const Image& i,unsigned bits){
    if(bits!=8 && bits!=16 && bits!=24 && bits!=32)throw std::runtime_error("Unsupported native pixel size");
    if(i.width<1 || i.height<1 || i.width>2048 || i.height>2048 ||
       i.pixels.size()!=std::size_t(i.width)*std::size_t(i.height))throw std::runtime_error("Invalid surface dimensions or pixel count");
    const auto maximum=bits==32?UINT32_MAX:((std::uint32_t{1}<<bits)-1);
    for(auto pixel:i.pixels)if(pixel>maximum)throw std::runtime_error("Native pixel exceeds its format");
}
void validateFormat(PixelFormat f){
    if(f.bits!=8 && f.bits!=16 && f.bits!=24 && f.bits!=32)throw std::runtime_error("Unsupported native pixel size");
    if(f.bits==8){if(f.masks!=std::array<std::uint32_t,3>{})throw std::runtime_error("Indexed surfaces have no RGB masks");return;}
    const auto maximum=f.bits==32?UINT32_MAX:((std::uint32_t{1}<<f.bits)-1);
    for(auto mask:f.masks){
        const auto low=mask&(~mask+1),normalized=low?mask/low:0;
        if(!mask || mask>maximum || normalized>255 || (normalized&(normalized+1)))throw std::runtime_error("Unsupported RGB mask");
    }
    if((f.masks[0]&f.masks[1]) || (f.masks[0]&f.masks[2]) || (f.masks[1]&f.masks[2]))throw std::runtime_error("Overlapping RGB masks");
}
SurfaceId nextId(){
    static std::atomic<SurfaceId> serial{1};auto id=serial.load();
    for(;;){if(id==UINT64_MAX)throw std::runtime_error("Surface IDs exhausted");if(serial.compare_exchange_weak(id,id+1))return id;}
}
}
struct GlBlitter::Impl {
    struct Surface {int width=0,height=0;PixelFormat format;GLuint native=0,palette=0,rgba=0;};
    QOffscreenSurface surface;
    QOpenGLContext context;
    QOpenGLFunctions_3_3_Core gl;
    std::unique_ptr<QOpenGLShaderProgram> program,presentation;
    GLuint vao=0,fbo=0;
    GLint maxTexture=0;
    Driver info;
    RenderStats counters;
    std::unordered_map<SurfaceId,Surface> surfaces;
    Impl(){
        if(!qobject_cast<QGuiApplication*>(QCoreApplication::instance()) ||
           QThread::currentThread()!=QCoreApplication::instance()->thread())
            throw std::runtime_error("GlBlitter requires the Qt GUI thread");
        QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
        context.setFormat(format);
        if(!context.create() || context.isOpenGLES())throw std::runtime_error("Desktop OpenGL 3.3 is unavailable");
        surface.setFormat(context.format());surface.create();
        if(!surface.isValid())throw std::runtime_error("Cannot create an offscreen OpenGL surface");
        Current current(context,&surface);
        if(!gl.initializeOpenGLFunctions())throw std::runtime_error("OpenGL 3.3 functions are unavailable");
        const auto string=[this](GLenum which){const auto* value=gl.glGetString(which);return value?std::string(reinterpret_cast<const char*>(value)):std::string{};};
        info={string(GL_VENDOR),string(GL_RENDERER),string(GL_VERSION)};
        const auto shader=[](const char* fragment){
            auto result=std::make_unique<QOpenGLShaderProgram>();
            if(!result->addShaderFromSourceCode(QOpenGLShader::Vertex,vertexSource) ||
               !result->addShaderFromSourceCode(QOpenGLShader::Fragment,fragment) || !result->link())
                throw std::runtime_error(result->log().toStdString());
            return result;
        };
        program=shader(fragmentSource);presentation=shader(presentationSource);
        gl.glGenVertexArrays(1,&vao);gl.glGenFramebuffers(1,&fbo);gl.glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexture);
        check();
    }
    void check(){if(const auto error=gl.glGetError();error!=GL_NO_ERROR)throw std::runtime_error("OpenGL error: "+std::to_string(error));}
    void thread() const{if(QThread::currentThread()!=context.thread())throw std::runtime_error("OpenGL renderer used on a different thread");}
    Surface& get(SurfaceId id){auto at=surfaces.find(id);if(at==surfaces.end())throw std::runtime_error("Unknown or destroyed surface");return at->second;}
    void release(Surface& s){GLuint textures[3]={s.native,s.palette,s.rgba};gl.glDeleteTextures(3,textures);}
    GLuint texture(GLint format,GLenum layout,GLenum type,int width,int height,const void* pixels){
        GLuint id=0;gl.glGenTextures(1,&id);gl.glBindTexture(GL_TEXTURE_2D,id);
        gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        gl.glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,layout,type,pixels);return id;
    }
    void attach(GLuint texture,int width,int height){
        gl.glBindFramebuffer(GL_FRAMEBUFFER,fbo);gl.glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
        gl.glDrawBuffer(GL_COLOR_ATTACHMENT0);gl.glReadBuffer(GL_COLOR_ATTACHMENT0);
        if(gl.glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Integer framebuffer is incomplete");
        gl.glViewport(0,0,width,height);
        gl.glDisable(GL_BLEND);gl.glDisable(GL_DITHER);gl.glDisable(GL_FRAMEBUFFER_SRGB);gl.glDisable(GL_DEPTH_TEST);
        gl.glDisable(GL_STENCIL_TEST);gl.glDisable(GL_CULL_FACE);gl.glDisable(GL_SCISSOR_TEST);
    }
    ~Impl(){
        if(context.isValid() && surface.isValid()){
            try {Current current(context,&surface);for(auto& pair:surfaces)release(pair.second);
                program.reset();presentation.reset();gl.glDeleteVertexArrays(1,&vao);gl.glDeleteFramebuffers(1,&fbo);}
            catch(const std::exception&){}
        }
    }
};
GlBlitter::GlBlitter():impl_(std::make_unique<Impl>()){}
GlBlitter::~GlBlitter()=default;
Driver GlBlitter::driver() const{return impl_->info;}
RenderStats GlBlitter::stats() const{impl_->thread();auto result=impl_->counters;result.surfaces=impl_->surfaces.size();return result;}
SurfaceId GlBlitter::create(const Image& image,PixelFormat format){
    validateFormat(format);validateImage(image,format.bits);auto& p=*impl_;p.thread();
    if(image.width>p.maxTexture || image.height>p.maxTexture)throw std::runtime_error("Surface exceeds the OpenGL texture limit");
    if(p.surfaces.size()>=64 || image.pixels.size()>16*1024*1024-p.counters.pixels)throw std::runtime_error("Renderer surface budget exceeded");
    const auto id=nextId();Current current(p.context,&p.surface);Impl::Surface s;s.width=image.width;s.height=image.height;s.format=format;
    auto& g=p.gl;g.glActiveTexture(GL_TEXTURE0);g.glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    try {
        s.native=p.texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,s.width,s.height,image.pixels.data());
        if(format.bits==8){std::array<std::uint8_t,1024> black{};for(unsigned i=0;i<256;++i)black[i*4+3]=255;
            s.palette=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,256,1,black.data());}
        p.check();p.surfaces.emplace(id,s);
    }catch(...){p.release(s);throw;}
    ++p.counters.uploads;p.counters.pixels+=image.pixels.size();return id;
}
void GlBlitter::destroy(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);Current current(p.context,&p.surface);
    p.counters.pixels-=std::size_t(s.width)*s.height;p.release(s);p.surfaces.erase(id);p.check();
}
void GlBlitter::update(SurfaceId id,int x,int y,const Image& patch){
    auto& p=*impl_;p.thread();auto& s=p.get(id);validateImage(patch,s.format.bits);
    if(x<0 || y<0 || x>s.width-patch.width || y>s.height-patch.height)throw std::runtime_error("Update rectangle is out of bounds");
    Current current(p.context,&p.surface);auto& g=p.gl;g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,s.native);
    g.glPixelStorei(GL_UNPACK_ALIGNMENT,4);g.glTexSubImage2D(GL_TEXTURE_2D,0,x,y,patch.width,patch.height,GL_RED_INTEGER,GL_UNSIGNED_INT,patch.pixels.data());
    p.check();++p.counters.uploads;
}
void GlBlitter::copy(SurfaceId source,SurfaceId destination,Rect r,int x,int y,std::optional<std::uint32_t> key){
    auto& p=*impl_;p.thread();auto& src=p.get(source);auto& dst=p.get(destination);
    if(source==destination)throw std::runtime_error("Self-copy is unsupported");
    if(src.format.bits!=dst.format.bits || src.format.masks!=dst.format.masks)throw std::runtime_error("Copy requires identical native formats");
    if(r.left<0 || r.top<0 || r.right<=r.left || r.bottom<=r.top || r.right>src.width || r.bottom>src.height ||
       x<0 || y<0 || x>dst.width-(r.right-r.left) || y>dst.height-(r.bottom-r.top))throw std::runtime_error("Copy rectangle is out of bounds");
    if(key && src.format.bits<32 && *key>((std::uint32_t{1}<<src.format.bits)-1))throw std::runtime_error("Source key exceeds its format");
    Current current(p.context,&p.surface);auto& g=p.gl;p.attach(dst.native,dst.width,dst.height);
    g.glEnable(GL_SCISSOR_TEST);g.glScissor(x,y,r.right-r.left,r.bottom-r.top);
    g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,src.native);
    if(!p.program->bind())throw std::runtime_error("Cannot bind copy shader");
    g.glUniform1i(p.program->uniformLocation("sourcePixels"),0);
    g.glUniform2i(p.program->uniformLocation("sourceOrigin"),r.left,r.top);g.glUniform2i(p.program->uniformLocation("destinationOrigin"),x,y);
    g.glUniform1i(p.program->uniformLocation("hasKey"),key.has_value());g.glUniform1ui(p.program->uniformLocation("sourceKey"),key.value_or(0));
    g.glBindVertexArray(p.vao);g.glDrawArrays(GL_TRIANGLES,0,3);g.glBindVertexArray(0);p.program->release();g.glDisable(GL_SCISSOR_TEST);
    p.check();++p.counters.copies;
}
void GlBlitter::swapContents(SurfaceId first,SurfaceId second){
    auto& p=*impl_;p.thread();auto& a=p.get(first);auto& b=p.get(second);
    if(first==second || a.width!=b.width || a.height!=b.height ||
       a.format.bits!=b.format.bits || a.format.masks!=b.format.masks)
        throw std::runtime_error("Aliased or incompatible surface swap");
    std::swap(a.native,b.native);
}
void GlBlitter::setPalette(SurfaceId id,unsigned first,const std::vector<Rgb>& colors){
    auto& p=*impl_;p.thread();auto& s=p.get(id);
    if(s.format.bits!=8 || colors.empty() || first>=256 || colors.size()>256-first)throw std::runtime_error("Invalid indexed palette update");
    std::vector<std::uint8_t> bytes;bytes.reserve(colors.size()*4);
    for(auto c:colors){bytes.push_back(c.red);bytes.push_back(c.green);bytes.push_back(c.blue);bytes.push_back(255);}
    Current current(p.context,&p.surface);auto& g=p.gl;g.glActiveTexture(GL_TEXTURE1);g.glBindTexture(GL_TEXTURE_2D,s.palette);
    g.glPixelStorei(GL_UNPACK_ALIGNMENT,4);g.glTexSubImage2D(GL_TEXTURE_2D,0,int(first),0,int(colors.size()),1,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,bytes.data());
    p.check();++p.counters.paletteUpdates;
}
Image GlBlitter::read(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);Current current(p.context,&p.surface);p.attach(s.native,s.width,s.height);
    Image out{s.width,s.height,std::vector<std::uint32_t>(std::size_t(s.width)*s.height)};
    p.gl.glPixelStorei(GL_PACK_ALIGNMENT,4);p.gl.glReadPixels(0,0,s.width,s.height,GL_RED_INTEGER,GL_UNSIGNED_INT,out.pixels.data());
    p.check();++p.counters.nativeReadbacks;return out;
}
QImage GlBlitter::present(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);Current current(p.context,&p.surface);auto& g=p.gl;
    if(!s.rgba){g.glActiveTexture(GL_TEXTURE0);s.rgba=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,s.width,s.height,nullptr);}
    p.attach(s.rgba,s.width,s.height);g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,s.native);
    g.glActiveTexture(GL_TEXTURE1);g.glBindTexture(GL_TEXTURE_2D,s.palette);
    if(!p.presentation->bind())throw std::runtime_error("Cannot bind presentation shader");
    g.glUniform1i(p.presentation->uniformLocation("nativePixels"),0);g.glUniform1i(p.presentation->uniformLocation("palette"),1);
    g.glUniform1i(p.presentation->uniformLocation("indexed"),s.format.bits==8);
    std::array<GLuint,3> low{1,1,1},maximum{1,1,1};
    if(s.format.bits!=8)for(unsigned i=0;i<3;++i){low[i]=s.format.masks[i]&(~s.format.masks[i]+1);maximum[i]=s.format.masks[i]/low[i];}
    g.glUniform3uiv(p.presentation->uniformLocation("masks"),1,s.format.masks.data());
    g.glUniform3uiv(p.presentation->uniformLocation("lowBits"),1,low.data());g.glUniform3uiv(p.presentation->uniformLocation("maxima"),1,maximum.data());
    g.glBindVertexArray(p.vao);g.glDrawArrays(GL_TRIANGLES,0,3);g.glBindVertexArray(0);p.presentation->release();
    QImage out(s.width,s.height,QImage::Format_RGBA8888);if(out.isNull())throw std::runtime_error("Cannot allocate presentation image");
    g.glPixelStorei(GL_PACK_ALIGNMENT,4);g.glReadPixels(0,0,s.width,s.height,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,out.bits());
    p.check();++p.counters.presentations;return out;
}
Image GlBlitter::draw(const Blit& c){
    validate(c);
    const PixelFormat f{c.bits,c.bits==8?std::array<std::uint32_t,3>{}:c.bits==16?
        std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}};
    SurfaceId src=0,dst=0;
    try {src=create(c.source,f);dst=create(c.destination,f);copy(src,dst,c.sourceRect,c.destinationX,c.destinationY,c.sourceKey);
        auto output=read(dst);destroy(dst);dst=0;destroy(src);src=0;return output;
    }catch(...){if(dst)destroy(dst);if(src)destroy(src);throw;}
}
}

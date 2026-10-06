#include "blit.hpp"
#include "surface_copy.hpp"
#include "known_pixels.hpp"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QThread>
#include <QPointer>
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
uniform usampler2D sourceMask;
uniform bool hasMask;
uniform ivec2 sourceOrigin;
uniform ivec2 destinationOrigin;
uniform bool hasKey;
uniform uint sourceKey;
layout(location=0) out uint nativePixel;
void main(){
    ivec2 at=ivec2(gl_FragCoord.xy)-destinationOrigin+sourceOrigin;
    if(hasMask && texelFetch(sourceMask,at,0).r==0u)discard;
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
struct GpuFrame::Data {
    std::shared_ptr<GlBlitter::Impl> owner;
    GLuint texture=0;
    QSize dimensions;
    QPointer<QOpenGLContext> consumer;
    GLsync produced=nullptr,consumed=nullptr;
    ~Data();
};
struct GlBlitter::Impl {
    struct Surface {int width=0,height=0;PixelFormat format;GLuint native=0,palette=0,rgba=0;std::weak_ptr<GpuFrame::Data> gpu;ClipperState clipper;KnownPixels validity;};
    QOffscreenSurface surface;
    QOpenGLContext context;
    QOpenGLFunctions_3_3_Core gl;
    std::unique_ptr<QOpenGLShaderProgram> program,presentation;
    GLuint vao=0,fbo=0;
    GLint maxTexture=0;
    Driver info;
    RenderStats counters;
    std::unordered_map<SurfaceId,Surface> surfaces;
    explicit Impl(QOpenGLContext* shareContext){
        if(!qobject_cast<QGuiApplication*>(QCoreApplication::instance()) ||
           QThread::currentThread()!=QCoreApplication::instance()->thread())
            throw std::runtime_error("GlBlitter requires the Qt GUI thread");
        QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
        context.setFormat(format);
        if(shareContext){
            if(!shareContext->isValid() || shareContext->thread()!=QThread::currentThread())
                throw std::runtime_error("Invalid GUI-thread sharing context");
            context.setShareContext(shareContext);
        }
        if(!context.create() || context.isOpenGLES())throw std::runtime_error("Desktop OpenGL 3.3 is unavailable");
        if(shareContext && !QOpenGLContext::areSharing(&context,shareContext))
            throw std::runtime_error("OpenGL context sharing failed");
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
    // Both samplers refer to storage distinct from the attached destination.
    void copyTexture(GLuint sourceTexture,GLuint maskTexture,Surface& dst,Rect r,int x,int y,
                     std::optional<std::uint32_t> key){
        attach(dst.native,dst.width,dst.height);
        gl.glEnable(GL_SCISSOR_TEST);gl.glScissor(x,y,r.right-r.left,r.bottom-r.top);
        gl.glActiveTexture(GL_TEXTURE0);gl.glBindTexture(GL_TEXTURE_2D,sourceTexture);
        gl.glActiveTexture(GL_TEXTURE1);gl.glBindTexture(GL_TEXTURE_2D,maskTexture?maskTexture:sourceTexture);
        if(!program->bind())throw std::runtime_error("Cannot bind copy shader");
        gl.glUniform1i(program->uniformLocation("sourcePixels"),0);
        gl.glUniform1i(program->uniformLocation("sourceMask"),1);
        gl.glUniform1i(program->uniformLocation("hasMask"),maskTexture!=0);
        gl.glUniform2i(program->uniformLocation("sourceOrigin"),r.left,r.top);gl.glUniform2i(program->uniformLocation("destinationOrigin"),x,y);
        gl.glUniform1i(program->uniformLocation("hasKey"),key.has_value());gl.glUniform1ui(program->uniformLocation("sourceKey"),key.value_or(0));
        gl.glBindVertexArray(vao);gl.glDrawArrays(GL_TRIANGLES,0,3);gl.glBindVertexArray(0);program->release();gl.glDisable(GL_SCISSOR_TEST);
        check();++counters.copies;
    }
    void copyShared(Surface& dst,const SurfaceCopyPiece& piece){
        const auto r=piece.source;const auto width=r.right-r.left,height=r.bottom-r.top;
        GLuint frozen=0;
        try {
            gl.glActiveTexture(GL_TEXTURE0);
            frozen=texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,width,height,nullptr);
            attach(dst.native,dst.width,dst.height);
            gl.glBindTexture(GL_TEXTURE_2D,frozen);
            gl.glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,r.left,r.top,width,height);check();
            copyTexture(frozen,0,dst,{0,0,width,height},piece.x,piece.y,std::nullopt);
            gl.glDeleteTextures(1,&frozen);frozen=0;check();
        }catch(...){gl.glDeleteTextures(1,&frozen);throw;}
    }
    ~Impl(){
        if(context.isValid() && surface.isValid()){
            try {Current current(context,&surface);for(auto& pair:surfaces)release(pair.second);
                program.reset();presentation.reset();gl.glDeleteVertexArrays(1,&vao);gl.glDeleteFramebuffers(1,&fbo);}
            catch(const std::exception&){}
        }
    }
};
GpuFrame::Data::~Data(){
    owner->thread();Current current(owner->context,&owner->surface);
    if(produced)owner->gl.glDeleteSync(produced);
    if(consumed)owner->gl.glDeleteSync(consumed);
    owner->gl.glDeleteTextures(1,&texture);
}
QSize GpuFrame::size() const{return data_?data_->dimensions:QSize{};}
unsigned GpuFrame::textureForCurrentContext() const{
    if(!data_)throw std::runtime_error("Empty GPU frame");
    auto& p=*data_->owner;p.thread();auto* current=QOpenGLContext::currentContext();
    if(!current || !QOpenGLContext::areSharing(current,&p.context))
        throw std::runtime_error("GPU frame requires a sharing current context");
    if(data_->consumer && data_->consumer!=current)
        throw std::runtime_error("GPU frame supports one consumer context");
    data_->consumer=current;
    if(data_->produced)p.gl.glWaitSync(data_->produced,0,GL_TIMEOUT_IGNORED);
    return data_->texture;
}
void GpuFrame::samplingComplete() const{
    textureForCurrentContext();auto& g=data_->owner->gl;
    if(data_->consumed)g.glDeleteSync(data_->consumed);
    data_->consumed=g.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
    if(!data_->consumed)throw std::runtime_error("Cannot fence GPU sampling");
    g.glFlush();
}
GlBlitter::GlBlitter(QOpenGLContext* shareContext):impl_(std::make_shared<Impl>(shareContext)){}
GlBlitter::~GlBlitter(){
    // Frames retain the context, but must not retain unrelated native surfaces.
    try {
        auto& p=*impl_;p.thread();Current current(p.context,&p.surface);
        for(auto& pair:p.surfaces)p.release(pair.second);
        p.surfaces.clear();p.counters.pixels=0;
    }catch(const std::exception&){}
}
Driver GlBlitter::driver() const{return impl_->info;}
RenderStats GlBlitter::stats() const{impl_->thread();auto result=impl_->counters;result.surfaces=impl_->surfaces.size();return result;}
SurfaceId GlBlitter::create(const Image& image,PixelFormat format){
    validateFormat(format);validateImage(image,format.bits);auto& p=*impl_;p.thread();
    if(image.width>p.maxTexture || image.height>p.maxTexture)throw std::runtime_error("Surface exceeds the OpenGL texture limit");
    if(p.surfaces.size()>=64 || image.pixels.size()>16*1024*1024-p.counters.pixels)throw std::runtime_error("Renderer surface budget exceeded");
    const auto id=nextId();Current current(p.context,&p.surface);Impl::Surface s;s.width=image.width;s.height=image.height;s.format=format;s.validity=KnownPixels(s.width,s.height);
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
    p.check();s.validity.define({x,y,x+patch.width,y+patch.height});++p.counters.uploads;
}
void GlBlitter::copy(SurfaceId source,SurfaceId destination,Rect r,int x,int y,std::optional<std::uint32_t> key,
                     std::optional<SurfaceId> mask){
    auto& p=*impl_;p.thread();auto& src=p.get(source);auto& dst=p.get(destination);
    if(source==destination)throw std::runtime_error("Self-copy is unsupported");
    if(src.format.bits!=dst.format.bits || src.format.masks!=dst.format.masks)throw std::runtime_error("Copy requires identical native formats");
    if(r.left<0 || r.top<0 || r.right<=r.left || r.bottom<=r.top || r.right>src.width || r.bottom>src.height ||
       x<0 || y<0 || x>dst.width-(r.right-r.left) || y>dst.height-(r.bottom-r.top))throw std::runtime_error("Copy rectangle is out of bounds");
    src.validity.require(r);
    if(key || mask)dst.validity.require({x,y,x+r.right-r.left,y+r.bottom-r.top});
    if(key && src.format.bits<32 && *key>((std::uint32_t{1}<<src.format.bits)-1))throw std::runtime_error("Source key exceeds its format");
    if(mask){
        auto& m=p.get(*mask);
        m.validity.require(r);
        if(*mask==destination || m.format.bits!=8 || m.width!=src.width || m.height!=src.height)
            throw std::runtime_error("Copy mask must be an indexed source-sized surface distinct from destination");
    }
    Current current(p.context,&p.surface);
    p.copyTexture(src.native,mask?p.get(*mask).native:0,dst,r,x,y,key);
    dst.validity.define({x,y,x+r.right-r.left,y+r.bottom-r.top});
}
void GlBlitter::invalidateContents(SurfaceId id){
    auto& p=*impl_;p.thread();p.get(id).validity.invalidate();
}
void GlBlitter::setClipper(SurfaceId id,const ClipperState& clipper){
    auto& p=*impl_;p.thread();auto& s=p.get(id);
    validateClipper(clipper,s.width,s.height);s.clipper=clipper;
}
SurfaceCopyResult GlBlitter::surfaceCopy(SurfaceId source,SurfaceId destination,const SurfaceCopyRequest& request){
    auto& p=*impl_;p.thread();const auto& s=p.get(source);auto& d=p.get(destination);
    const std::array<std::uint32_t,3> rgb565{0xf800,0x7e0,0x1f};
    if(source==destination && request.flags!=0 && request.flags!=(request.api==SurfaceCopyApi::BltFast?0x10u:0x01000000u))
        throw std::runtime_error("Self-copy requires opaque flags");
    if(s.format.bits!=16 || d.format.bits!=16 || s.format.masks!=rgb565 || d.format.masks!=rgb565)
        throw std::runtime_error("Surface2 copy requires RGB565 surfaces");
    const auto plan=planSurfaceCopy(s.width,s.height,d.width,d.height,d.clipper,request);
    if(source==destination){
        Current current(p.context,&p.surface);
        // Each ordered clip piece sees preceding writes, then freezes its own source.
        for(const auto& piece:plan.pieces){
            d.validity.require(piece.source);p.copyShared(d,piece);
            d.validity.define({piece.x,piece.y,piece.x+piece.source.right-piece.source.left,piece.y+piece.source.bottom-piece.source.top});
        }
    }else for(const auto& piece:plan.pieces)copy(source,destination,piece.source,piece.x,piece.y);
    return {plan.hresult,unsigned(plan.pieces.size())};
}
void GlBlitter::swapContents(SurfaceId first,SurfaceId second){
    auto& p=*impl_;p.thread();auto& a=p.get(first);auto& b=p.get(second);
    if(first==second || a.width!=b.width || a.height!=b.height ||
       a.format.bits!=b.format.bits || a.format.masks!=b.format.masks)
        throw std::runtime_error("Aliased or incompatible surface swap");
    std::swap(a.native,b.native);std::swap(a.validity,b.validity);
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
    auto& p=*impl_;p.thread();auto& s=p.get(id);s.validity.require({0,0,s.width,s.height});Current current(p.context,&p.surface);p.attach(s.native,s.width,s.height);
    Image out{s.width,s.height,std::vector<std::uint32_t>(std::size_t(s.width)*s.height)};
    p.gl.glPixelStorei(GL_PACK_ALIGNMENT,4);p.gl.glReadPixels(0,0,s.width,s.height,GL_RED_INTEGER,GL_UNSIGNED_INT,out.pixels.data());
    p.check();++p.counters.nativeReadbacks;return out;
}
namespace {
// Kept local to the owning renderer; both output paths use identical conversion.
template<class Owner,class Surface>
void resolvePresentation(Owner& p,Surface& s,GLuint target){
    auto& g=p.gl;
    p.attach(target,s.width,s.height);g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,s.native);
    g.glActiveTexture(GL_TEXTURE1);g.glBindTexture(GL_TEXTURE_2D,s.palette);
    if(!p.presentation->bind())throw std::runtime_error("Cannot bind presentation shader");
    g.glUniform1i(p.presentation->uniformLocation("nativePixels"),0);g.glUniform1i(p.presentation->uniformLocation("palette"),1);
    g.glUniform1i(p.presentation->uniformLocation("indexed"),s.format.bits==8);
    std::array<GLuint,3> low{1,1,1},maximum{1,1,1};
    if(s.format.bits!=8)for(unsigned i=0;i<3;++i){low[i]=s.format.masks[i]&(~s.format.masks[i]+1);maximum[i]=s.format.masks[i]/low[i];}
    g.glUniform3uiv(p.presentation->uniformLocation("masks"),1,s.format.masks.data());
    g.glUniform3uiv(p.presentation->uniformLocation("lowBits"),1,low.data());g.glUniform3uiv(p.presentation->uniformLocation("maxima"),1,maximum.data());
    g.glBindVertexArray(p.vao);g.glDrawArrays(GL_TRIANGLES,0,3);g.glBindVertexArray(0);p.presentation->release();
    p.check();
}
}
QImage GlBlitter::present(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);s.validity.require({0,0,s.width,s.height});Current current(p.context,&p.surface);auto& g=p.gl;
    if(!s.rgba){g.glActiveTexture(GL_TEXTURE0);s.rgba=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,s.width,s.height,nullptr);}
    resolvePresentation(p,s,s.rgba);
    QImage out(s.width,s.height,QImage::Format_RGBA8888);if(out.isNull())throw std::runtime_error("Cannot allocate presentation image");
    g.glPixelStorei(GL_PACK_ALIGNMENT,4);g.glReadPixels(0,0,s.width,s.height,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,out.bits());
    p.check();++p.counters.presentations;++p.counters.rgbaReadbacks;return out;
}
GpuFrame GlBlitter::presentGpu(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);s.validity.require({0,0,s.width,s.height});Current current(p.context,&p.surface);auto& g=p.gl;
    auto frame=s.gpu.lock();
    if(!frame){
        frame=std::make_shared<GpuFrame::Data>();frame->owner=impl_;frame->dimensions={s.width,s.height};
        g.glActiveTexture(GL_TEXTURE0);
        frame->texture=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,s.width,s.height,nullptr);
        s.gpu=frame;
    }
    if(frame->consumed){g.glWaitSync(frame->consumed,0,GL_TIMEOUT_IGNORED);g.glDeleteSync(frame->consumed);frame->consumed=nullptr;}
    resolvePresentation(p,s,frame->texture);
    if(frame->produced)g.glDeleteSync(frame->produced);
    frame->produced=g.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
    if(!frame->produced)throw std::runtime_error("Cannot fence GPU presentation");
    g.glFlush();p.check();++p.counters.presentations;++p.counters.gpuPresentations;
    return GpuFrame(std::move(frame));
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

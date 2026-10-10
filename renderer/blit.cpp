#include "dib.hpp"
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
    bool switched=false;
    Current(QOpenGLContext& context,QSurface* target):context(context){
        if(previous==&context && surface==target)return;
        if(!context.makeCurrent(target))throw std::runtime_error("Cannot make the OpenGL context current");
        switched=true;
    }
    ~Current(){if(switched){if(previous && surface)previous->makeCurrent(surface);else context.doneCurrent();}}
};
constexpr auto vertexSource=R"(#version 330 core
void main(){
    vec2 p[3]=vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3));
    gl_Position=vec4(p[gl_VertexID],0,1);
})";
constexpr auto fragmentSource=R"(#version 330 core
uniform usampler2D sourcePixels;
uniform bool indexedSource;
uniform uint spritePalette[256];
uniform usampler2D sourceMask;
uniform bool hasMask;
uniform ivec2 sourceOrigin;
uniform ivec2 destinationOrigin;
uniform bool hasKey;
uniform uint sourceKey;
uniform uint keyMask;
layout(location=0) out uint nativePixel;
void main(){
    ivec2 at=ivec2(gl_FragCoord.xy)-destinationOrigin+sourceOrigin;
    if(hasMask && texelFetch(sourceMask,at,0).r==0u)discard;
    uint pixel=texelFetch(sourcePixels,at,0).r;
    if(indexedSource)pixel=spritePalette[pixel];
    if(hasKey && (pixel&keyMask)==sourceKey)discard;
    nativePixel=pixel;
})";
}
namespace {
constexpr auto compositeSource=R"(#version 330 core
uniform usampler2D sourcePixels;
uniform bool indexedSource;
uniform uint spritePalette[256];
uniform usampler2D sourceMask;
uniform usampler2D oldPixels;
uniform ivec2 sourceOrigin;
uniform ivec2 destinationOrigin;
uniform int mode;
uniform uvec3 additiveChannels;
uniform int rowOffsets[16];
uniform int rowPeriod;
layout(location=0) out uint nativePixel;
void main(){
    ivec2 destination=ivec2(gl_FragCoord.xy);
    if(mode==6){
        uint d=texelFetch(oldPixels,destination,0).r;
        uvec3 channels=uvec3(d>>11u,(d>>5u)&63u,d&31u);
        channels=min((channels+additiveChannels)&uvec3(65535u),uvec3(31u,63u,31u));
        nativePixel=(channels.x<<11u)|(channels.y<<5u)|channels.z;return;
    }
    ivec2 at=destination-destinationOrigin+sourceOrigin;
    if(texelFetch(sourceMask,at,0).r==0u)discard;
    uint s=texelFetch(sourcePixels,at,0).r;
    if(indexedSource)s=spritePalette[s];
    uint d=texelFetch(oldPixels,destination,0).r;
    uint m=0x7befu;
    if(mode==1)nativePixel=((s>>1u)&m)+((d>>1u)&m);
    else if(mode==2){uint halfway=(d>>1u)&m;nativePixel=halfway+((halfway>>1u)&m)+((s>>2u)&(m>>1u)&m);}
    else if(mode==4){uint halfway=(s>>1u)&m;nativePixel=halfway+((halfway>>1u)&m)+((d>>2u)&(m>>1u)&m);}
    else nativePixel=texelFetch(oldPixels,destination+ivec2(rowOffsets[destination.y%rowPeriod],0),0).r;
})";
constexpr auto presentationSource=R"(#version 330 core
uniform usampler2D nativePixels;
uniform usampler2D palette;
uniform bool indexed;
uniform bool rgb565;
uniform uvec3 masks;
uniform uvec3 lowBits;
uniform uvec3 maxima;
layout(location=0) out uvec4 color;
void main(){
    uint pixel=texelFetch(nativePixels,ivec2(gl_FragCoord.xy),0).r;
    if(indexed)color=texelFetch(palette,ivec2(int(pixel),0),0);
    else if(rgb565){
        // Constant shifts avoid per-pixel uniform integer division on software GPUs.
        uvec3 channels=uvec3((pixel>>11u)&31u,(pixel>>5u)&63u,pixel&31u);
        color=uvec4((channels<<uvec3(3,2,3))|(channels>>uvec3(2,4,2)),255u);
    } else {
        uvec3 channels=(uvec3(pixel)&masks)/lowBits;
        color=uvec4(channels*255u/maxima,255u);
    }
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
    struct Surface {int width=0,height=0;PixelFormat format;GLuint native=0,palette=0,rgba=0;std::weak_ptr<GpuFrame::Data> gpu;ClipperState clipper;KnownPixels validity;std::array<Rgb,256> paletteColors{};std::array<bool,256> paletteKnown{};};
    QOffscreenSurface surface;
    QOpenGLContext context;
    QOpenGLFunctions_3_3_Core gl;
    std::unique_ptr<QOpenGLShaderProgram> program,presentation,compositing;
    GLuint vao=0,fbo=0;
    GLuint attached=0,boundProgram=0;
    QSize viewport;
    bool scissor=false;
    unsigned batchDepth=0;
    struct CopyUniforms {GLint hasMask,sourceOrigin,destinationOrigin,hasKey,sourceKey,keyMask,indexed,palette;} copyUniforms{};
    struct PresentationUniforms {GLint indexed,rgb565,masks,lowBits,maxima;} presentationUniforms{};
    struct CompositeUniforms {GLint sourceOrigin,destinationOrigin,mode,rowOffsets,rowPeriod,indexed,palette,additive;} compositeUniforms{};
    std::optional<std::array<std::uint16_t,256>> copyPalette,compositePalette;
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
        program=shader(fragmentSource);presentation=shader(presentationSource);compositing=shader(compositeSource);
        const auto location=[](QOpenGLShaderProgram& shader,const char* name){
            const auto value=shader.uniformLocation(name);
            if(value<0)throw std::runtime_error(std::string("Missing shader uniform: ")+name);
            return value;
        };
        copyUniforms={location(*program,"hasMask"),location(*program,"sourceOrigin"),location(*program,"destinationOrigin"),
                      location(*program,"hasKey"),location(*program,"sourceKey"),location(*program,"keyMask"),location(*program,"indexedSource"),location(*program,"spritePalette[0]")};
        presentationUniforms={location(*presentation,"indexed"),location(*presentation,"rgb565"),location(*presentation,"masks"),
                              location(*presentation,"lowBits"),location(*presentation,"maxima")};
        useProgram(program->programId());
        gl.glUniform1i(location(*program,"sourcePixels"),0);gl.glUniform1i(location(*program,"sourceMask"),1);
        useProgram(presentation->programId());
        gl.glUniform1i(location(*presentation,"nativePixels"),0);gl.glUniform1i(location(*presentation,"palette"),1);
        compositeUniforms={location(*compositing,"sourceOrigin"),location(*compositing,"destinationOrigin"),location(*compositing,"mode"),location(*compositing,"rowOffsets"),location(*compositing,"rowPeriod"),location(*compositing,"indexedSource"),location(*compositing,"spritePalette[0]"),location(*compositing,"additiveChannels")};
        useProgram(compositing->programId());gl.glUniform1i(location(*compositing,"sourcePixels"),0);gl.glUniform1i(location(*compositing,"sourceMask"),1);gl.glUniform1i(location(*compositing,"oldPixels"),2);
        gl.glGenVertexArrays(1,&vao);gl.glGenFramebuffers(1,&fbo);gl.glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexture);
        // This context is private to the renderer; caller/viewport state lives
        // in a different context and does not invalidate these bindings.
        gl.glBindVertexArray(vao);gl.glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        gl.glDrawBuffer(GL_COLOR_ATTACHMENT0);gl.glReadBuffer(GL_COLOR_ATTACHMENT0);
        gl.glDisable(GL_BLEND);gl.glDisable(GL_DITHER);gl.glDisable(GL_FRAMEBUFFER_SRGB);gl.glDisable(GL_DEPTH_TEST);
        gl.glDisable(GL_STENCIL_TEST);gl.glDisable(GL_CULL_FACE);gl.glDisable(GL_SCISSOR_TEST);
        gl.glPixelStorei(GL_PACK_ALIGNMENT,4);gl.glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        check();
    }
    void check(bool immediate=false){
#ifdef NDEBUG
        if(batchDepth && !immediate)return;
#else
        (void)immediate;
#endif
        if(const auto error=gl.glGetError();error!=GL_NO_ERROR)throw std::runtime_error("OpenGL error: "+std::to_string(error));
    }
    void useProgram(GLuint id){if(boundProgram!=id){gl.glUseProgram(id);boundProgram=id;}}
    void setScissor(bool enabled){if(scissor!=enabled){if(enabled)gl.glEnable(GL_SCISSOR_TEST);else gl.glDisable(GL_SCISSOR_TEST);scissor=enabled;}}
    void deleteTextures(GLsizei count,const GLuint* textures){
        for(GLsizei i=0;i<count;++i)if(textures[i]==attached)attached=0;
        gl.glDeleteTextures(count,textures);
    }
    void thread() const{if(QThread::currentThread()!=context.thread())throw std::runtime_error("OpenGL renderer used on a different thread");}
    Surface& get(SurfaceId id){auto at=surfaces.find(id);if(at==surfaces.end())throw std::runtime_error("Unknown or destroyed surface");return at->second;}
    void release(Surface& s){GLuint textures[3]={s.native,s.palette,s.rgba};deleteTextures(3,textures);}
    GLuint texture(GLint format,GLenum layout,GLenum type,int width,int height,const void* pixels){
        GLuint id=0;
        try {
            gl.glGenTextures(1,&id);gl.glBindTexture(GL_TEXTURE_2D,id);
            gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            gl.glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,layout,type,pixels);check(true);return id;
        }catch(...){deleteTextures(1,&id);throw;}
    }
    void attach(GLuint texture,int width,int height){
        if(attached!=texture){
            attached=0; // A failed attachment must not leave a valid old cache.
            gl.glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
            if(gl.glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Integer framebuffer is incomplete");
            attached=texture;
        }
        if(viewport!=QSize(width,height)){gl.glViewport(0,0,width,height);viewport={width,height};}
    }
    // Both samplers refer to storage distinct from the attached destination.
    void selectPalette(GLint indexed,GLint location,std::optional<std::array<std::uint16_t,256>>& previous,const std::array<std::uint16_t,256>* palette){
        gl.glUniform1i(indexed,palette!=nullptr);
        if(palette&&(!previous||*previous!=*palette)){
            std::array<GLuint,256> words;std::copy(palette->begin(),palette->end(),words.begin());
            gl.glUniform1uiv(location,256,words.data());previous=*palette;++counters.paletteUpdates;
        }
    }
    void copyTexture(GLuint sourceTexture,GLuint maskTexture,Surface& dst,Rect r,int x,int y,
                     std::optional<std::uint32_t> key,std::uint32_t keyMask=UINT32_MAX,const std::array<std::uint16_t,256>* palette=nullptr){
        if(!maskTexture && !key && !palette){
            // Identical integer storage: opaque copies need no fragment shader.
            // Sources are distinct here; overlap callers supply a frozen texture.
            attach(sourceTexture,dst.width,dst.height);
            gl.glActiveTexture(GL_TEXTURE0);gl.glBindTexture(GL_TEXTURE_2D,dst.native);
            gl.glCopyTexSubImage2D(GL_TEXTURE_2D,0,x,y,r.left,r.top,r.right-r.left,r.bottom-r.top);
            check();++counters.copies;return;
        }
        attach(dst.native,dst.width,dst.height);
        setScissor(true);gl.glScissor(x,y,r.right-r.left,r.bottom-r.top);
        gl.glActiveTexture(GL_TEXTURE0);gl.glBindTexture(GL_TEXTURE_2D,sourceTexture);
        gl.glActiveTexture(GL_TEXTURE1);gl.glBindTexture(GL_TEXTURE_2D,maskTexture?maskTexture:sourceTexture);
        useProgram(program->programId());
        selectPalette(copyUniforms.indexed,copyUniforms.palette,copyPalette,palette);
        gl.glUniform1i(copyUniforms.hasMask,maskTexture!=0);
        gl.glUniform2i(copyUniforms.sourceOrigin,r.left,r.top);gl.glUniform2i(copyUniforms.destinationOrigin,x,y);
        gl.glUniform1i(copyUniforms.hasKey,key.has_value());gl.glUniform1ui(copyUniforms.sourceKey,key.value_or(0));
        gl.glUniform1ui(copyUniforms.keyMask,keyMask);
        gl.glDrawArrays(GL_TRIANGLES,0,3);
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
            deleteTextures(1,&frozen);frozen=0;check();
        }catch(...){deleteTextures(1,&frozen);throw;}
    }
    // Measured keyed overlap reads its own preceding writes in row-major order.
    // A separate one-pixel texture avoids sampling the attached framebuffer.
    void copySharedKeyed(Surface& dst,const SurfaceCopyPiece& piece,std::uint32_t key,std::uint32_t keyMask){
        GLuint pixel=0;
        try {
            gl.glActiveTexture(GL_TEXTURE0);
            pixel=texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,1,1,nullptr);
            for(int y=piece.source.top;y<piece.source.bottom;++y)
                for(int x=piece.source.left;x<piece.source.right;++x){
                    attach(dst.native,dst.width,dst.height);
                    gl.glBindTexture(GL_TEXTURE_2D,pixel);
                    gl.glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,x,y,1,1);check();
                    copyTexture(pixel,0,dst,{0,0,1,1},piece.x+x-piece.source.left,
                                piece.y+y-piece.source.top,key,keyMask);
                }
            deleteTextures(1,&pixel);pixel=0;check();
        }catch(...){deleteTextures(1,&pixel);throw;}
    }
    ~Impl(){
        if(context.isValid() && surface.isValid()){
            try {Current current(context,&surface);for(auto& pair:surfaces)release(pair.second);
                program.reset();presentation.reset();compositing.reset();gl.glDeleteVertexArrays(1,&vao);gl.glDeleteFramebuffers(1,&fbo);}
            catch(const std::exception&){}
        }
    }
};
GpuFrame::Data::~Data(){
    owner->thread();Current current(owner->context,&owner->surface);
    if(produced)owner->gl.glDeleteSync(produced);
    if(consumed)owner->gl.glDeleteSync(consumed);
    owner->deleteTextures(1,&texture);
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
void GlBlitter::batch(const std::function<void()>& operations){
    auto& p=*impl_;p.thread();
    if(!operations)throw std::runtime_error("Missing renderer batch operations");
    Current current(p.context,&p.surface);++p.batchDepth;
    try {operations();}catch(...){--p.batchDepth;throw;}
    --p.batchDepth;p.check();
}
RenderStats GlBlitter::stats() const{impl_->thread();auto result=impl_->counters;result.surfaces=impl_->surfaces.size();return result;}
SurfaceId GlBlitter::create(const Image& image,PixelFormat format){
    validateFormat(format);validateImage(image,format.bits);auto& p=*impl_;p.thread();
    if(image.width>p.maxTexture || image.height>p.maxTexture)throw std::runtime_error("Surface exceeds the OpenGL texture limit");
    if(p.surfaces.size()>=64 || image.pixels.size()>16*1024*1024-p.counters.pixels)throw std::runtime_error("Renderer surface budget exceeded");
    const auto id=nextId();Current current(p.context,&p.surface);Impl::Surface s;s.width=image.width;s.height=image.height;s.format=format;s.validity=KnownPixels(s.width,s.height);
    auto& g=p.gl;g.glActiveTexture(GL_TEXTURE0);
    try {
        s.native=p.texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,s.width,s.height,image.pixels.data());
        if(format.bits==8){std::array<std::uint8_t,1024> black{};for(unsigned i=0;i<256;++i)black[i*4+3]=255;
            s.palette=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,256,1,black.data());}
        p.check(true);p.surfaces.emplace(id,std::move(s));
    }catch(...){p.release(s);throw;}
    ++p.counters.uploads;p.counters.pixels+=image.pixels.size();return id;
}
SurfaceId GlBlitter::allocate(int width,int height,PixelFormat format){
    validateFormat(format);auto& p=*impl_;p.thread();
    if(width<1||height<1||width>2048||height>2048||width>p.maxTexture||height>p.maxTexture)
        throw std::runtime_error("Invalid allocated surface dimensions");
    const auto count=std::size_t(width)*height;
    if(p.surfaces.size()>=64||count>16*1024*1024-p.counters.pixels)throw std::runtime_error("Renderer surface budget exceeded");
    const auto id=nextId();Current current(p.context,&p.surface);Impl::Surface s;
    s.width=width;s.height=height;s.format=format;s.validity=KnownPixels(width,height);s.validity.invalidate();
    auto& g=p.gl;g.glActiveTexture(GL_TEXTURE0);
    try{
        s.native=p.texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,width,height,nullptr);
        if(format.bits==8){std::array<std::uint8_t,1024> black{};for(unsigned i=0;i<256;++i)black[i*4+3]=255;
            s.palette=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,256,1,black.data());}
        p.check(true);p.surfaces.emplace(id,std::move(s));
    }catch(...){p.release(s);throw;}
    p.counters.pixels+=count;return id;
}
void GlBlitter::updateRows(SurfaceId id,const Image& plane,int first,int rows){
    auto& p=*impl_;p.thread();const auto& s=p.get(id);
    if(plane.width!=s.width||plane.height!=s.height)throw std::runtime_error("Invalid upload plane extent");
    updateRegionRows(id,0,0,plane,first,rows);
}
void GlBlitter::updateRegionRows(SurfaceId id,int x,int y,const Image& plane,int first,int rows){
    auto& p=*impl_;p.thread();auto& s=p.get(id);
    if(plane.width<1||plane.height<1||plane.pixels.size()!=std::size_t(plane.width)*plane.height||first<0||rows<1||first>plane.height-rows||
       x<0||y<0||x>s.width-plane.width||y>s.height-plane.height)
        throw std::runtime_error("Invalid upload rows");
    const auto begin=plane.pixels.begin()+std::size_t(first)*plane.width,end=begin+std::size_t(rows)*plane.width;
    const auto maximum=s.format.bits==32?UINT32_MAX:((std::uint32_t{1}<<s.format.bits)-1);
    if(std::any_of(begin,end,[maximum](auto value){return value>maximum;}))throw std::runtime_error("Native upload pixel exceeds format");
    Current current(p.context,&p.surface);auto& g=p.gl;g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,s.native);
    g.glTexSubImage2D(GL_TEXTURE_2D,0,x,y+first,plane.width,rows,GL_RED_INTEGER,GL_UNSIGNED_INT,&*begin);
    p.check();s.validity.define({x,y+first,x+plane.width,y+first+rows});++p.counters.uploads;
}
void GlBlitter::destroy(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);Current current(p.context,&p.surface);
    p.counters.pixels-=std::size_t(s.width)*s.height;p.release(s);p.surfaces.erase(id);p.check();
}
void GlBlitter::update(SurfaceId id,int x,int y,const Image& patch){
    auto& p=*impl_;p.thread();auto& s=p.get(id);validateImage(patch,s.format.bits);
    if(x<0 || y<0 || x>s.width-patch.width || y>s.height-patch.height)throw std::runtime_error("Update rectangle is out of bounds");
    Current current(p.context,&p.surface);auto& g=p.gl;g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,s.native);
    g.glTexSubImage2D(GL_TEXTURE_2D,0,x,y,patch.width,patch.height,GL_RED_INTEGER,GL_UNSIGNED_INT,patch.pixels.data());
    p.check();s.validity.define({x,y,x+patch.width,y+patch.height});++p.counters.uploads;
}
void GlBlitter::reloadDib(SurfaceId id,const DibInput& dib,const std::optional<std::vector<Rect>>& dcRegions){
    auto& p=*impl_;p.thread();auto& s=p.get(id);
    const bool indexed=s.format.bits==8;
    const bool rgb565=s.format.bits==16 && s.format.masks==std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f};
    if(!indexed && !rgb565 && ((s.format.bits!=24 && s.format.bits!=32) || s.format.masks!=std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}))
        throw std::runtime_error("DIB reload requires indexed8, RGB565 or canonical RGB24/32 target");
    if(indexed && !std::all_of(s.paletteKnown.begin(),s.paletteKnown.end(),[](bool known){return known;}))
        throw std::runtime_error("Indexed DIB reload requires all 256 explicit palette entries");
    if(dcRegions){
        if(dcRegions->size()>maxClipRegions)throw std::runtime_error("DC region budget exceeded");
        for(auto r:*dcRegions)if(r.left<0 || r.top<0 || r.left>=r.right || r.top>=r.bottom || r.right>s.width || r.bottom>s.height)
            throw std::runtime_error("DC region outside destination");
    }
    auto image=decodeDibRgb(dib);
    const int w=std::min(image.width,s.width),h=std::min(image.height,s.height);
    Image converted{w,h,std::vector<std::uint32_t>(std::size_t(w)*h)};
    bool identicalPalette=indexed && dib.header[14]==8;
    if(identicalPalette)for(unsigned i=0;i<256;++i){const auto c=s.paletteColors[i];
        if(dib.palette[i*4]!=c.blue || dib.palette[i*4+1]!=c.green || dib.palette[i*4+2]!=c.red){identicalPalette=false;break;}
    }
    std::unordered_map<std::uint32_t,std::uint32_t> indices;
    for(int y=0;y<h;++y)for(int x=0;x<w;++x){
        auto color=image.pixels[std::size_t(y)*image.width+x];
        if(rgb565)color=((color>>19)&31u)<<11|((color>>10)&63u)<<5|((color>>3)&31u);
        else if(identicalPalette){
            const auto stride=(std::size_t(image.width)+3)&~std::size_t(3);
            color=dib.pixels[std::size_t(image.height-1-y)*stride+x];
        }else if(indexed){
            // Observed true-color input uses a 5-bit-bin center lookup; indexed
            // source palettes retain full channel precision before translation.
            if(dib.header[14]==24)color=(color&0xf8f8f8u)+0x040404u;
            const auto found=indices.find(color);
            if(found!=indices.end())color=found->second;
            else{
                const int red=int((color>>16)&255),green=int((color>>8)&255),blue=int(color&255);
                unsigned best=0,distance=UINT32_MAX;
                for(unsigned i=0;i<256;++i){const auto c=s.paletteColors[i];
                    const int r=red-c.red,g=green-c.green,b=blue-c.blue;
                    const auto d=unsigned(r*r+g*g+b*b);
                    if(d<distance){distance=d;best=i;}
                }
                indices.emplace(color,best);color=best;
            }
        }
        converted.pixels[std::size_t(y)*w+x]=color;
    }
    // DirectDraw's attached Blt clipper is separate from the application GDI
    // region. Overlapping GDI rectangles form a union of identical writes.
    const std::vector<Rect> whole{{0,0,w,h}};
    for(auto r:dcRegions?*dcRegions:whole){
        r.right=std::min(r.right,w);r.bottom=std::min(r.bottom,h);
        if(r.left>=r.right || r.top>=r.bottom)continue;
        Image patch{r.right-r.left,r.bottom-r.top,std::vector<std::uint32_t>(std::size_t(r.right-r.left)*(r.bottom-r.top))};
        for(int y=r.top;y<r.bottom;++y)std::copy_n(converted.pixels.begin()+std::size_t(y)*w+r.left,patch.width,patch.pixels.begin()+std::size_t(y-r.top)*patch.width);
        update(id,r.left,r.top,patch);
    }
}
void GlBlitter::copy(SurfaceId source,SurfaceId destination,Rect r,int x,int y,std::optional<std::uint32_t> key,
                     std::optional<SurfaceId> mask,std::uint32_t keyMask){
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
    p.copyTexture(src.native,mask?p.get(*mask).native:0,dst,r,x,y,key,keyMask);
    dst.validity.define({x,y,x+r.right-r.left,y+r.bottom-r.top});
}
void GlBlitter::composite(SurfaceId source,SurfaceId destination,Rect r,int x,int y,SurfaceId mask,const SpriteComposite& c,const std::array<std::uint16_t,256>* palette){
    if(c.mode==CompositeMode::copy&&!palette){copy(source,destination,r,x,y,std::nullopt,mask);return;}
    auto& p=*impl_;p.thread();auto& src=p.get(source);auto& dst=p.get(destination);auto& coverage=p.get(mask);
    const PixelFormat rgb565{16,{0xf800,0x7e0,0x1f}};
    if(source==destination||mask==destination||src.format.bits!=rgb565.bits||src.format.masks!=rgb565.masks||dst.format.bits!=rgb565.bits||dst.format.masks!=rgb565.masks||coverage.format.bits!=8||coverage.width!=src.width||coverage.height!=src.height)
        throw std::invalid_argument("Sprite composition requires distinct RGB565 source/destination and matching coverage");
    if(r.left<0||r.top<0||r.right<=r.left||r.bottom<=r.top||r.right>src.width||r.bottom>src.height||x<0||y<0||x>dst.width-(r.right-r.left)||y>dst.height-(r.bottom-r.top))throw std::invalid_argument("Composition rectangle outside storage");
    if(c.mode!=CompositeMode::copy&&c.mode!=CompositeMode::half&&c.mode!=CompositeMode::quarterSource&&c.mode!=CompositeMode::quarterDestination&&c.mode!=CompositeMode::displace)throw std::invalid_argument("Unsupported sprite composite mode");
    int reach=0;
    if(c.mode==CompositeMode::displace){
        if(!c.rowPeriod||c.rowPeriod>16)throw std::invalid_argument("Displacement row period outside bounds");
        for(unsigned i=0;i<c.rowPeriod;++i){if(c.rowOffsets[i]<0||c.rowOffsets[i]>16)throw std::invalid_argument("Backward or excessive displacement unsupported");reach=std::max(reach,c.rowOffsets[i]);}
        if(x+r.right-r.left+reach>dst.width)throw std::invalid_argument("Displacement sample outside destination");
    }
    src.validity.require(r);coverage.validity.require(r);dst.validity.require({0,0,dst.width,dst.height});
    Current current(p.context,&p.surface);
    if(c.mode==CompositeMode::copy){p.copyTexture(src.native,coverage.native,dst,r,x,y,std::nullopt,UINT32_MAX,palette);return;}
    auto& g=p.gl;GLuint frozen=0;
    try{
        g.glActiveTexture(GL_TEXTURE2);frozen=p.texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,dst.width,dst.height,nullptr);
        p.attach(dst.native,dst.width,dst.height);g.glBindTexture(GL_TEXTURE_2D,frozen);g.glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,dst.width,dst.height);p.check();
        p.setScissor(true);g.glScissor(x,y,r.right-r.left,r.bottom-r.top);
        g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,src.native);g.glActiveTexture(GL_TEXTURE1);g.glBindTexture(GL_TEXTURE_2D,coverage.native);g.glActiveTexture(GL_TEXTURE2);g.glBindTexture(GL_TEXTURE_2D,frozen);
        p.useProgram(p.compositing->programId());p.selectPalette(p.compositeUniforms.indexed,p.compositeUniforms.palette,p.compositePalette,palette);g.glUniform2i(p.compositeUniforms.sourceOrigin,r.left,r.top);g.glUniform2i(p.compositeUniforms.destinationOrigin,x,y);g.glUniform1i(p.compositeUniforms.mode,int(c.mode));g.glUniform1iv(p.compositeUniforms.rowOffsets,16,c.rowOffsets.data());g.glUniform1i(p.compositeUniforms.rowPeriod,int(c.rowPeriod));
        g.glDrawArrays(GL_TRIANGLES,0,3);p.check();++p.counters.copies;p.deleteTextures(1,&frozen);frozen=0;
    }catch(...){p.deleteTextures(1,&frozen);throw;}
}
void GlBlitter::colourRect(SurfaceId destination,const Image& pixels,Rect source,int x,int y,const SpriteComposite& mode){
    if(pixels.width>64||pixels.height>64)throw std::invalid_argument("Colour rectangle exceeds primitive budget");
    SurfaceId colour=0,mask=0;
    try{colour=create(pixels,{16,{0xf800,0x7e0,0x1f}});mask=create({pixels.width,pixels.height,std::vector<std::uint32_t>(pixels.pixels.size(),1)},{8,{}});
        composite(colour,destination,source,x,y,mask,mode);destroy(mask);destroy(colour);
    }catch(...){if(mask)destroy(mask);if(colour)destroy(colour);throw;}
}
void GlBlitter::additiveRect(SurfaceId destination,Rect r,const std::array<std::uint16_t,3>& channels){
    auto& p=*impl_;p.thread();auto& dst=p.get(destination);
    if(dst.format.bits!=16||dst.format.masks!=std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}||
       r.left<0||r.top<0||r.right<=r.left||r.bottom<=r.top||r.right>dst.width||r.bottom>dst.height)
        throw std::invalid_argument("Additive rectangle requires in-bounds RGB565 storage");
    dst.validity.require({0,0,dst.width,dst.height});
    Current current(p.context,&p.surface);auto& g=p.gl;GLuint frozen=0;
    try{
        g.glActiveTexture(GL_TEXTURE2);frozen=p.texture(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT,dst.width,dst.height,nullptr);
        p.attach(dst.native,dst.width,dst.height);g.glBindTexture(GL_TEXTURE_2D,frozen);g.glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,dst.width,dst.height);
        p.setScissor(true);g.glScissor(r.left,r.top,r.right-r.left,r.bottom-r.top);
        // All integer samplers have distinct storage from the framebuffer. Only oldPixels is sampled in mode6.
        for(unsigned unit=0;unit<3;++unit){g.glActiveTexture(GL_TEXTURE0+unit);g.glBindTexture(GL_TEXTURE_2D,frozen);}
        p.useProgram(p.compositing->programId());g.glUniform1i(p.compositeUniforms.mode,6);
        g.glUniform3ui(p.compositeUniforms.additive,channels[0],channels[1],channels[2]);g.glDrawArrays(GL_TRIANGLES,0,3);
        p.check();++p.counters.copies;p.deleteTextures(1,&frozen);frozen=0;
    }catch(...){p.deleteTextures(1,&frozen);throw;}
}
void GlBlitter::invalidateContents(SurfaceId id){
    auto& p=*impl_;p.thread();p.get(id).validity.invalidate();
}
void GlBlitter::setClipper(SurfaceId id,const ClipperState& clipper){
    auto& p=*impl_;p.thread();auto& s=p.get(id);
    validateClipper(clipper,s.width,s.height);s.clipper=clipper;
}
SurfaceCopyResult GlBlitter::surfaceCopy(SurfaceId source,SurfaceId destination,const SurfaceCopyRequest& request,
                                       std::optional<std::uint32_t> nativeKey){
    auto& p=*impl_;p.thread();const auto& s=p.get(source);auto& d=p.get(destination);
    if(source==destination && request.flags!=0 && request.flags!=(request.api==SurfaceCopyApi::BltFast?0x10u:0x01000000u))
        throw std::runtime_error("Self-copy requires opaque flags");
    if(s.format.bits!=d.format.bits || s.format.masks!=d.format.masks)
        throw std::runtime_error("Surface2 copy requires identical native formats");
    if(nativeKey && s.format.bits<32 && *nativeKey>((std::uint32_t{1}<<s.format.bits)-1))
        throw std::runtime_error("Source key exceeds native storage width");
    const auto keyMask=s.format.bits==8?255u:s.format.masks[0]|s.format.masks[1]|s.format.masks[2];
    const auto plan=planSurfaceCopy(s.width,s.height,d.width,d.height,d.clipper,request);
    if(source==destination){
        if(nativeKey){
            std::size_t pixels=0;
            for(const auto& piece:plan.pieces)
                pixels+=std::size_t(piece.source.right-piece.source.left)*(piece.source.bottom-piece.source.top);
            if(pixels>4096)throw std::runtime_error("Ordered keyed overlap pixel budget exceeded");
        }
        Current current(p.context,&p.surface);
        // Each ordered clip piece sees preceding writes, then freezes its own source.
        for(const auto& piece:plan.pieces){
            d.validity.require(piece.source);
            if(nativeKey){
                d.validity.require({piece.x,piece.y,piece.x+piece.source.right-piece.source.left,piece.y+piece.source.bottom-piece.source.top});
                p.copySharedKeyed(d,piece,*nativeKey,keyMask);
            }else p.copyShared(d,piece);
            d.validity.define({piece.x,piece.y,piece.x+piece.source.right-piece.source.left,piece.y+piece.source.bottom-piece.source.top});
        }
    }else for(const auto& piece:plan.pieces)copy(source,destination,piece.source,piece.x,piece.y,nativeKey,std::nullopt,keyMask);
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
    g.glTexSubImage2D(GL_TEXTURE_2D,0,int(first),0,int(colors.size()),1,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,bytes.data());
    p.check();for(unsigned i=0;i<colors.size();++i){s.paletteColors[first+i]=colors[i];s.paletteKnown[first+i]=true;}++p.counters.paletteUpdates;
}
Image GlBlitter::read(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);s.validity.require({0,0,s.width,s.height});Current current(p.context,&p.surface);p.attach(s.native,s.width,s.height);
    Image out{s.width,s.height,std::vector<std::uint32_t>(std::size_t(s.width)*s.height)};
    p.gl.glReadPixels(0,0,s.width,s.height,GL_RED_INTEGER,GL_UNSIGNED_INT,out.pixels.data());
    p.check(true);++p.counters.nativeReadbacks;return out;
}
namespace {
// Kept local to the owning renderer; both output paths use identical conversion.
template<class Owner,class Surface>
void resolvePresentation(Owner& p,Surface& s,GLuint target){
    auto& g=p.gl;
    p.attach(target,s.width,s.height);p.setScissor(false);g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,s.native);
    g.glActiveTexture(GL_TEXTURE1);g.glBindTexture(GL_TEXTURE_2D,s.palette);
    p.useProgram(p.presentation->programId());
    g.glUniform1i(p.presentationUniforms.indexed,s.format.bits==8);
    g.glUniform1i(p.presentationUniforms.rgb565,s.format.bits==16 && s.format.masks==std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f});
    std::array<GLuint,3> low{1,1,1},maximum{1,1,1};
    if(s.format.bits!=8)for(unsigned i=0;i<3;++i){low[i]=s.format.masks[i]&(~s.format.masks[i]+1);maximum[i]=s.format.masks[i]/low[i];}
    g.glUniform3uiv(p.presentationUniforms.masks,1,s.format.masks.data());
    g.glUniform3uiv(p.presentationUniforms.lowBits,1,low.data());g.glUniform3uiv(p.presentationUniforms.maxima,1,maximum.data());
    g.glDrawArrays(GL_TRIANGLES,0,3);
    p.check();
}
}
QImage GlBlitter::present(SurfaceId id){
    auto& p=*impl_;p.thread();auto& s=p.get(id);s.validity.require({0,0,s.width,s.height});Current current(p.context,&p.surface);auto& g=p.gl;
    if(!s.rgba){g.glActiveTexture(GL_TEXTURE0);s.rgba=p.texture(GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,s.width,s.height,nullptr);}
    resolvePresentation(p,s,s.rgba);
    QImage out(s.width,s.height,QImage::Format_RGBA8888);if(out.isNull())throw std::runtime_error("Cannot allocate presentation image");
    g.glReadPixels(0,0,s.width,s.height,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,out.bits());
    p.check(true);++p.counters.presentations;++p.counters.rgbaReadbacks;return out;
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

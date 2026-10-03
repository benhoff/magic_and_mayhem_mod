#include "blit.hpp"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QThread>
#include <algorithm>
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
struct GlBlitter::Impl {
    QOffscreenSurface surface;
    QOpenGLContext context;
    QOpenGLFunctions_3_3_Core gl;
    std::unique_ptr<QOpenGLShaderProgram> program;
    GLuint vao=0;
    Driver info;
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
        const auto string=[this](GLenum which){
            const auto* value=gl.glGetString(which);
            return value?std::string(reinterpret_cast<const char*>(value)):std::string{};
        };
        info={string(GL_VENDOR),string(GL_RENDERER),string(GL_VERSION)};
        program=std::make_unique<QOpenGLShaderProgram>();
        if(!program->addShaderFromSourceCode(QOpenGLShader::Vertex,vertexSource) ||
           !program->addShaderFromSourceCode(QOpenGLShader::Fragment,fragmentSource) || !program->link())
            throw std::runtime_error(program->log().toStdString());
        gl.glGenVertexArrays(1,&vao);
        if(gl.glGetError()!=GL_NO_ERROR)throw std::runtime_error("Cannot initialize OpenGL drawing resources");
    }
    ~Impl(){
        // Context destruction also reclaims its resources if it has been lost.
        if(context.isValid() && surface.isValid()){
            try {Current current(context,&surface);program.reset();if(vao)gl.glDeleteVertexArrays(1,&vao);}
            catch(const std::exception&){}
        }
    }
};
GlBlitter::GlBlitter():impl_(std::make_unique<Impl>()){}
GlBlitter::~GlBlitter()=default;
Driver GlBlitter::driver() const{return impl_->info;}
Image GlBlitter::draw(const Blit& c){
    validate(c);
    auto& p=*impl_;
    if(QThread::currentThread()!=p.context.thread())throw std::runtime_error("OpenGL blitter used on a different thread");
    Current current(p.context,&p.surface);auto& g=p.gl;
    GLint maximum=0;g.glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum);
    if(std::max({c.source.width,c.source.height,c.destination.width,c.destination.height})>maximum)
        throw std::runtime_error("Capture exceeds the OpenGL texture limit");
    Image output{c.destination.width,c.destination.height,std::vector<std::uint32_t>(c.destination.pixels.size())};
    struct Resources {
        QOpenGLFunctions_3_3_Core& gl;
        GLuint textures[2]={0,0},fbo=0;
        ~Resources(){gl.glBindFramebuffer(GL_FRAMEBUFFER,0);gl.glDeleteFramebuffers(1,&fbo);gl.glDeleteTextures(2,textures);}
    } resources{g};
    g.glGenTextures(2,resources.textures);g.glActiveTexture(GL_TEXTURE0);
    g.glPixelStorei(GL_UNPACK_ALIGNMENT,4);g.glPixelStorei(GL_PACK_ALIGNMENT,4);
    const Image* images[2]={&c.source,&c.destination};
    for(int i=0;i<2;++i){
        g.glBindTexture(GL_TEXTURE_2D,resources.textures[i]);
        g.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        g.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        g.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        g.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        g.glTexImage2D(GL_TEXTURE_2D,0,GL_R32UI,images[i]->width,images[i]->height,0,
                       GL_RED_INTEGER,GL_UNSIGNED_INT,images[i]->pixels.data());
    }
    g.glGenFramebuffers(1,&resources.fbo);g.glBindFramebuffer(GL_FRAMEBUFFER,resources.fbo);
    g.glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,resources.textures[1],0);
    g.glDrawBuffer(GL_COLOR_ATTACHMENT0);g.glReadBuffer(GL_COLOR_ATTACHMENT0);
    if(g.glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Native integer framebuffer is incomplete");
    g.glViewport(0,0,output.width,output.height);
    g.glDisable(GL_BLEND);g.glDisable(GL_DITHER);g.glDisable(GL_FRAMEBUFFER_SRGB);
    g.glDisable(GL_DEPTH_TEST);g.glDisable(GL_STENCIL_TEST);g.glDisable(GL_CULL_FACE);
    g.glEnable(GL_SCISSOR_TEST);
    g.glScissor(c.destinationX,c.destinationY,c.sourceRect.right-c.sourceRect.left,c.sourceRect.bottom-c.sourceRect.top);
    g.glBindTexture(GL_TEXTURE_2D,resources.textures[0]);
    if(!p.program->bind())throw std::runtime_error("Cannot bind the blit shader");
    g.glUniform1i(p.program->uniformLocation("sourcePixels"),0);
    g.glUniform2i(p.program->uniformLocation("sourceOrigin"),c.sourceRect.left,c.sourceRect.top);
    g.glUniform2i(p.program->uniformLocation("destinationOrigin"),c.destinationX,c.destinationY);
    g.glUniform1i(p.program->uniformLocation("hasKey"),c.sourceKey.has_value());
    g.glUniform1ui(p.program->uniformLocation("sourceKey"),c.sourceKey.value_or(0));
    g.glBindVertexArray(p.vao);g.glDrawArrays(GL_TRIANGLES,0,3);
    g.glDisable(GL_SCISSOR_TEST);g.glBindVertexArray(0);p.program->release();
    // Integer readback has the same row order used for upload and coordinates.
    g.glReadPixels(0,0,output.width,output.height,GL_RED_INTEGER,GL_UNSIGNED_INT,output.pixels.data());
    if(const auto error=g.glGetError();error!=GL_NO_ERROR)
        throw std::runtime_error("OpenGL blit/readback error: "+std::to_string(error));
    return output;
}
}

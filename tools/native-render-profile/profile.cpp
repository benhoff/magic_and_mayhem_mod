#include "commands.hpp"
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <cstdio>
#include <stdexcept>
#include <vector>
static void require(bool ok,const char* error){if(!ok)throw std::runtime_error(error);}
static const char* vertex=R"(#version 330 core
void main(){vec2 p[3]=vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3));gl_Position=vec4(p[gl_VertexID],0,1);})";
static const char* fragment=R"(#version 330 core
uniform usampler2D frame;out vec4 color;
void main(){color=vec4(texelFetch(frame,ivec2(gl_FragCoord.xy),0))/255.0;})";
int main(int argc,char**argv){
 if(argc!=5){std::fprintf(stderr,"Usage: profile PREFIX {serial|throughput} REPETITIONS REPORT\n");return 2;}
 QGuiApplication app(argc,argv);
 try{
  require(qgetenv("DISPLAY").isEmpty()&&qgetenv("WAYLAND_DISPLAY").isEmpty(),"Profile must have no display");
  const bool serial=QByteArray(argv[2])=="serial";require(serial||QByteArray(argv[2])=="throughput","Invalid mode");
  int repetitions=QByteArray(argv[3]).toInt();require(repetitions>=1&&repetitions<=100,"Invalid repetitions");
  QFile file(argv[1]);require(file.open(QIODevice::ReadOnly),"Cannot read prefix");auto bytes=file.readAll();
  QElapsedTimer clock;clock.start();auto started=clock.nsecsElapsed();
  mnm::render::CommandDecoder decoder(mnm::render::CommandStreamMode::Streaming);std::vector<mnm::render::SurfaceCommand> commands;for(qsizetype at=0;at<bytes.size();at+=65536){auto part=decoder.append(bytes.mid(at,65536));commands.insert(commands.end(),std::make_move_iterator(part.begin()),std::make_move_iterator(part.end()));}
  const auto decodeMs=(clock.nsecsElapsed()-started)/1e6;
  require(!commands.empty()&&commands.back().operation==6,"Prefix must end on complete PRESENT");
  std::vector<std::pair<std::size_t,std::size_t>> groups;std::size_t first=0;
  for(std::size_t i=0;i<commands.size();++i)if(commands[i].operation==6){groups.push_back({first,i+1-first});first=i+1;}
  QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);format.setRenderableType(QSurfaceFormat::OpenGL);
  QOpenGLContext context;context.setFormat(format);require(context.create(),"Root context create");
  QOffscreenSurface surface;surface.setFormat(context.format());surface.create();require(surface.isValid()&&context.makeCurrent(&surface),"Root surface/current");
  QOpenGLFunctions_3_3_Core g;require(g.initializeOpenGLFunctions(),"Root GL functions");
  const auto string=[&](GLenum key){return QString::fromLatin1(reinterpret_cast<const char*>(g.glGetString(key)));};
  QString renderer=string(GL_RENDERER),vendor=string(GL_VENDOR),version=string(GL_VERSION);
  QByteArray wanted=qgetenv("MNM_PROFILE_EGL_VENDOR");if(wanted.isEmpty())wanted="NVIDIA";
  require(vendor.contains(QString::fromLatin1(wanted),Qt::CaseInsensitive),"Unexpected GL vendor; refusing fallback");
  QOpenGLShaderProgram shader;require(shader.addShaderFromSourceCode(QOpenGLShader::Vertex,vertex)&&shader.addShaderFromSourceCode(QOpenGLShader::Fragment,fragment)&&shader.link(),"Sampling shader");
  GLuint texture=0,fbo=0,vao=0;g.glGenTextures(1,&texture);g.glBindTexture(GL_TEXTURE_2D,texture);
  g.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,800,600,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);g.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);g.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  g.glGenFramebuffers(1,&fbo);g.glBindFramebuffer(GL_FRAMEBUFFER,fbo);g.glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
  require(g.glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"Sampling FBO");g.glGenVertexArrays(1,&vao);
  QJsonArray samples,runs;QString finalHash;
  for(int repeat=-2;repeat<repetitions;++repeat){
   mnm::render::GpuFrame lease;GLsync sampled=nullptr;
   mnm::render::GlBlitter blitter(&context);
   auto* backend=reinterpret_cast<QOpenGLContext*>(qApp->property("mnmProfileLastContext").value<quintptr>());require(backend&&backend!=&context,"Backend context probe");
   QOffscreenSurface probe;probe.setFormat(backend->format());probe.create();require(probe.isValid(),"Backend probe surface");
   mnm::render::CommandConsumer consumer(blitter,[&](auto frame){
    lease=std::move(frame);require(QOpenGLContext::currentContext()==&context,"Unexpected callback context");
    auto size=lease.size();require(size.width()<=800&&size.height()<=600,"Frame exceeds profile output");
    g.glBindFramebuffer(GL_FRAMEBUFFER,fbo);g.glViewport(0,0,size.width(),size.height());g.glDisable(GL_BLEND);g.glDisable(GL_DITHER);g.glDisable(GL_FRAMEBUFFER_SRGB);
    g.glActiveTexture(GL_TEXTURE0);g.glBindTexture(GL_TEXTURE_2D,lease.textureForCurrentContext());shader.bind();shader.setUniformValue("frame",0);g.glBindVertexArray(vao);g.glDrawArrays(GL_TRIANGLES,0,3);g.glBindVertexArray(0);shader.release();
    lease.samplingComplete();if(serial){sampled=g.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);g.glFlush();}
   },{mnm::render::CommandDiagnostics::Skip,false,mnm::render::CommandStreamMode::Streaming});
   g.glBindFramebuffer(GL_FRAMEBUFFER,fbo);g.glViewport(0,0,800,600);g.glClearColor(0,0,0,1);g.glClear(GL_COLOR_BUFFER_BIT);g.glFinish();auto runStart=clock.nsecsElapsed();
   for(std::size_t index=0;index<groups.size();++index){
    GLuint query[2]={0,0};auto cpuStart=clock.nsecsElapsed();
    if(serial){require(backend->makeCurrent(&probe),"Timer backend current");g.glGenQueries(2,query);g.glQueryCounter(query[0],GL_TIMESTAMP);g.glFlush();require(context.makeCurrent(&surface),"Timer root restore");}
    const auto [at,count]=groups[index];consumer.submit(commands.data()+at,count);
    if(serial){
     require(backend->makeCurrent(&probe),"End timer backend");g.glWaitSync(sampled,0,GL_TIMEOUT_IGNORED);g.glQueryCounter(query[1],GL_TIMESTAMP);auto done=g.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);g.glFlush();
     require(context.makeCurrent(&surface),"End timer root restore");g.glWaitSync(done,0,GL_TIMEOUT_IGNORED);
     auto submitted=clock.nsecsElapsed();g.glFinish();auto completed=clock.nsecsElapsed();
     GLuint64 a=0,b=0;require(backend->makeCurrent(&probe),"Query results current");g.glGetQueryObjectui64v(query[0],GL_QUERY_RESULT,&a);g.glGetQueryObjectui64v(query[1],GL_QUERY_RESULT,&b);g.glDeleteQueries(2,query);g.glDeleteSync(done);g.glDeleteSync(sampled);sampled=nullptr;
     require(context.makeCurrent(&surface),"Query result restore");require(b>=a,"GPU timestamps regressed");
     if(repeat>=0)samples.append(QJsonObject{{"repeat",repeat},{"frame",int(index)},{"commands",int(count)},{"cpu_submit_ms",(submitted-cpuStart)/1e6},{"completion_ms",(completed-cpuStart)/1e6},{"completion_wait_ms",(completed-submitted)/1e6},{"gpu_timeline_ms",(b-a)/1e6}});
    }
    require(g.glGetError()==GL_NO_ERROR,"OpenGL error");
   }
   auto submitEnd=clock.nsecsElapsed();g.glFinish();auto completeEnd=clock.nsecsElapsed();
   if(repeat>=0)runs.append(QJsonObject{{"repeat",repeat},{"cpu_submit_ms",(submitEnd-runStart)/1e6},{"completion_ms",(completeEnd-runStart)/1e6},{"frames",int(groups.size())},{"commands",int(commands.size())},{"uploads",qint64(consumer.result().stats.uploads)},{"copies",qint64(consumer.result().stats.copies)},{"native_readbacks",qint64(consumer.result().stats.nativeReadbacks)},{"rgba_readbacks",qint64(consumer.result().stats.rgbaReadbacks)}});
   QByteArray rgba(800*600*4,0);g.glBindFramebuffer(GL_FRAMEBUFFER,fbo);g.glPixelStorei(GL_PACK_ALIGNMENT,1);g.glReadPixels(0,0,800,600,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());require(g.glGetError()==GL_NO_ERROR,"Diagnostic readback");
   QString hash=QString::fromLatin1(QCryptographicHash::hash(rgba,QCryptographicHash::Sha256).toHex());if(finalHash.isEmpty())finalHash=hash;else require(finalHash==hash,"Repeated replay pixels changed");
   if(qEnvironmentVariableIsSet("MNM_PROFILE_RGBA_OUT")&&repeat==repetitions-1){QFile out(qgetenv("MNM_PROFILE_RGBA_OUT"));require(out.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&out.write(rgba)==rgba.size(),"RGBA export");}
   consumer.abort();require(!blitter.stats().surfaces,"Prefix cleanup leak");lease={};g.glFinish();
  }
  g.glDeleteFramebuffers(1,&fbo);g.glDeleteTextures(1,&texture);g.glDeleteVertexArrays(1,&vao);
  QJsonObject report{{"success",true},{"renderer",renderer},{"vendor",vendor},{"version",version},{"qt_version",QT_VERSION_STR},{"mode",serial?"serial":"throughput"},{"warmup_replays",2},{"decode_ms",decodeMs},{"input_sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())},{"final_rgba_sha256",finalHash},{"samples",samples},{"runs",runs},{"desktop_used",false},{"diagnostic_readbacks_per_replay_outside_timing",1},{"scope","Captured complete-frame prefix through unchanged native decoder/consumer/GlBlitter; headless shared contexts and native GPU leases sampled to RGBA FBO. No game pacing, Qt widgets, compositor, monitor or input latency."}};
  QFile out(argv[4]);require(out.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Report exists/unwritable");out.write(QJsonDocument(report).toJson());std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());
  return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"profile failed: %s\n",e.what());return 1;}
}

#include <qpa/qplatformintegrationplugin.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformopenglcontext.h>
#include <qpa/qplatformoffscreensurface.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qwindowsysteminterface.h>
#include <QtCore/private/qeventdispatcher_unix_p.h>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QVariant>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <cstring>
namespace {
EGLConfig configFor(EGLDisplay display){
 EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
 EGLConfig config=nullptr;EGLint count=0;if(!eglChooseConfig(display,attrs,&config,1,&count)||!count)qFatal("No EGL OpenGL pbuffer config");return config;
}
class Surface:public QPlatformOffscreenSurface {
 EGLDisplay display_;EGLSurface surface_;QSurfaceFormat format_;
public:
 Surface(EGLDisplay d,QOffscreenSurface* s):QPlatformOffscreenSurface(s),display_(d),format_(s->requestedFormat()){
  EGLint attrs[]={EGL_WIDTH,1,EGL_HEIGHT,1,EGL_NONE};surface_=eglCreatePbufferSurface(d,configFor(d),attrs);
 }
 ~Surface()override{if(surface_!=EGL_NO_SURFACE)eglDestroySurface(display_,surface_);}
 bool isValid()const override{return surface_!=EGL_NO_SURFACE;}
 QSurfaceFormat format()const override{return format_;}
 EGLSurface native()const{return surface_;}
};
class Context:public QPlatformOpenGLContext {
 EGLDisplay display_;EGLContext context_=EGL_NO_CONTEXT;QSurfaceFormat format_;bool sharing_=false;
public:
 Context(EGLDisplay d,QOpenGLContext* q):display_(d),format_(q->format()){
  if(!eglBindAPI(EGL_OPENGL_API))qFatal("Cannot bind desktop OpenGL");
  EGLContext share=EGL_NO_CONTEXT;if(q->shareContext()){
   auto* c=dynamic_cast<Context*>(q->shareContext()->handle());if(!c)qFatal("Foreign sharing context");share=c->context_;sharing_=true;
  }
  EGLint attrs[]={EGL_CONTEXT_MAJOR_VERSION_KHR,3,EGL_CONTEXT_MINOR_VERSION_KHR,3,EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,EGL_NONE};
  context_=eglCreateContext(d,configFor(d),share,attrs);format_.setRenderableType(QSurfaceFormat::OpenGL);format_.setVersion(3,3);format_.setProfile(QSurfaceFormat::CoreProfile);
 }
 ~Context()override{if(context_!=EGL_NO_CONTEXT)eglDestroyContext(display_,context_);}
 bool isValid()const override{return context_!=EGL_NO_CONTEXT;}
 bool isSharing()const override{return sharing_;}
 QSurfaceFormat format()const override{return format_;}
 bool makeCurrent(QPlatformSurface* s)override{auto* p=dynamic_cast<Surface*>(s);return p&&eglMakeCurrent(display_,p->native(),p->native(),context_);}
 void doneCurrent()override{eglMakeCurrent(display_,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);}
 void swapBuffers(QPlatformSurface*)override{qFatal("Headless baseline has no swapchain");}
 QFunctionPointer getProcAddress(const char* name)override{return reinterpret_cast<QFunctionPointer>(eglGetProcAddress(name));}
};
class Screen:public QPlatformScreen {
public:QRect geometry()const override{return {0,0,800,600};}int depth()const override{return 32;}QImage::Format format()const override{return QImage::Format_RGBA8888;}
};
class Integration:public QPlatformIntegration {
 EGLDisplay display_=EGL_NO_DISPLAY;Screen* screen_=nullptr;
public:
 Integration(){
  auto query=reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
  auto get=reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
  if(!query||!get)qFatal("EGL device extensions unavailable");
  EGLDeviceEXT devices[16];EGLint count=0;if(!query(16,devices,&count))qFatal("EGL device enumeration failed");
  QByteArray wanted=qgetenv("MNM_PROFILE_EGL_VENDOR");if(wanted.isEmpty())wanted="NVIDIA";
  for(int i=0;i<count;++i){auto d=get(EGL_PLATFORM_DEVICE_EXT,devices[i],nullptr);EGLint a,b;if(!eglInitialize(d,&a,&b))continue;
   const char* vendor=eglQueryString(d,EGL_VENDOR);if(vendor&&QByteArray(vendor).contains(wanted)){display_=d;break;}eglTerminate(d);
  }
  if(display_==EGL_NO_DISPLAY)qFatal("Requested EGL vendor unavailable; refusing fallback");
 }
 ~Integration()override{if(screen_)QWindowSystemInterface::handleScreenRemoved(screen_);eglTerminate(display_);}
 void initialize()override{screen_=new Screen;QWindowSystemInterface::handleScreenAdded(screen_);}
 bool hasCapability(Capability c)const override{if(c==OpenGL||c==OffscreenSurface)return true;return QPlatformIntegration::hasCapability(c);}
 QPlatformWindow* createPlatformWindow(QWindow*)const override{qFatal("Window requested in headless baseline");return nullptr;}
 QPlatformBackingStore* createPlatformBackingStore(QWindow*)const override{qFatal("Backing store requested in headless baseline");return nullptr;}
 QAbstractEventDispatcher* createEventDispatcher()const override{return new QEventDispatcherUNIX;}
 QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* q)const override{qApp->setProperty("mnmProfileLastContext",QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(q)));return new Context(display_,q);}
 QPlatformOffscreenSurface* createPlatformOffscreenSurface(QOffscreenSurface* s)const override{return new Surface(display_,s);}
 QOpenGLContext::OpenGLModuleType openGLModuleType()override{return QOpenGLContext::LibGL;}
};
class Plugin:public QPlatformIntegrationPlugin {
 Q_OBJECT
 Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "headless.json")
public:QPlatformIntegration* create(const QString& key,const QStringList&)override{return key=="mnm-headless-egl"?new Integration:nullptr;}
};
}
#include "egl_platform.moc"

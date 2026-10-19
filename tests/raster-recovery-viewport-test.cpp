#include "gl_viewport.hpp"
#include <QApplication>
#include <QWindow>
#include <QPainter>
#include <QScreen>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
class EventTrace:public QObject {
    bool eventFilter(QObject* object,QEvent* event) override {
        if(qEnvironmentVariableIntValue("MNM_TRACE_PLATFORM_TEST")==1 && (event->type()==QEvent::Hide || event->type()==QEvent::Show || event->type()==QEvent::PlatformSurface || event->type()==QEvent::Close)){
            std::cerr<<object->metaObject()->className()<<" "<<object->objectName().toStdString()<<" event "<<int(event->type())<<" spontaneous "<<event->spontaneous();
            if(auto* widget=qobject_cast<QWidget*>(object))std::cerr<<" visible "<<widget->isVisible()<<" hidden "<<widget->isHidden();
            if(qobject_cast<QWindow*>(object))for(auto* widget:QApplication::topLevelWidgets())if(widget->objectName()=="second")std::cerr<<" root-visible "<<widget->isVisible()<<" root-hidden "<<widget->isHidden()<<" explicit "<<widget->testAttribute(Qt::WA_WState_ExplicitShowHide);
            std::cerr<<"\n";
        }
        return false;
    }
};
}
int main(int argc,char** argv){
    qputenv("MNM_NATIVE_DEVICE_RECOVERY","1");QApplication app(argc,argv);EventTrace trace;app.installEventFilter(&trace);
    try{
        QWidget first,second;first.setObjectName("first");second.setObjectName("second");first.resize(320,240);second.resize(320,240);
        GlViewport viewport(&first);viewport.setObjectName("viewport");bool presentationDesired=true;viewport.shouldRestorePlatformSurface=[&]{return presentationDesired;};viewport.resize(320,240);first.show();viewport.show();app.processEvents();
        require(viewport.ready() && viewport.rasterPresentation() && !viewport.context(),"Recovery viewport entered GUI OpenGL");
        mnm::render::GlBlitter renderer;auto surface=renderer.create({2,2,{0xf800,0x7e0,31,65535}},{16,{0xf800,0x7e0,31}});
        viewport.setGpuFrame(renderer.presentGpu(surface).snapshot());viewport.repaint();app.processEvents();
        auto check=[&]{const auto image=viewport.grabFramebuffer();require(image.pixelColor(60,40)==QColor(Qt::red)&&image.pixelColor(260,200)==QColor(Qt::white),"Raster frame lost across platform recreation");};
        auto onscreen=[&](QWidget& parent,QColor expected,const char* stage){
            QEventLoop loop;QTimer::singleShot(50,&loop,&QEventLoop::quit);loop.exec();
            const auto image=parent.screen()->grabWindow(parent.winId()).toImage();
            if(image.isNull() || image.pixelColor(qRound(60*image.devicePixelRatio()),qRound(40*image.devicePixelRatio()))!=expected){
                std::cerr<<stage<<" onscreen "<<image.width()<<"x"<<image.height()<<" pixel "<<image.pixelColor(60,40).name().toStdString()<<" expected "<<expected.name().toStdString()<<" visible "<<parent.isVisible()<<" window "<<parent.windowHandle()->isVisible()<<"\n";
                throw std::runtime_error("Recreated native window did not display retained pixels");
            }
        };
        check();onscreen(first,Qt::red,"initial");const auto frameSize=viewport.frameSize();
        // Reparenting and explicit native-window destruction recreate actual Qt
        // platform surfaces. The frame and renderer are kept throughout.
        viewport.setParent(&second);second.show();viewport.show();app.processEvents();check();
        auto* window=second.windowHandle();require(window,"No native platform window");window->destroy();app.processEvents();check();onscreen(second,Qt::red,"recreated");
        renderer.injectGraphicsFault(2);renderer.update(surface,0,0,{1,1,{31}});
        viewport.setGpuFrame(renderer.presentGpu(surface).snapshot());viewport.repaint();app.processEvents();
        require(viewport.grabFramebuffer().pixelColor(60,40)==QColor(Qt::blue) && viewport.frameSize()==frameSize && !viewport.context(),"Recovery could not continue on recreated raster surface");
        onscreen(second,Qt::blue,"continued");
        // A deliberate hide must not resurrect a closed/hidden application window.
        presentationDesired=false;second.hide();window->destroy();app.processEvents();require(!second.isVisible()&&!window->isVisible(),"Recovery resurrected hidden window");
        presentationDesired=true;second.show();viewport.show();app.processEvents();
        second.close();window->destroy();app.processEvents();require(!second.isVisible()&&!window->isVisible(),"Recovery resurrected a closed window");
        second.show();viewport.show();app.processEvents();window->destroy();presentationDesired=false;second.close();app.processEvents();
        require(!second.isVisible()&&!window->isVisible(),"Application cancellation did not stop queued recovery");
        QPoint point;require(viewport.imagePoint(viewport.imageRect().center(),point),"Input coordinates lost after recreation");
        std::cout<<QJsonDocument(QJsonObject{{"success",true},{"gui_gl_context",false},{"reparent",true},{"platform_surface_recreated",true},{"continued_after_hung_worker",true},{"frame_and_input_retained",true},{"onscreen_pixels_checked",true},{"hidden_window_stays_hidden",true},{"closed_window_stays_closed",true},{"queued_recovery_cancelled",true}}).toJson().constData();
        viewport.setGpuFrame({});return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

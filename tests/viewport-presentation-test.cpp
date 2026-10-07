#include "viewport_presentation.hpp"
#include "blit.hpp"
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QKeyEvent>
#include <QMainWindow>
#include <QLabel>
#include <QEventLoop>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSurfaceFormat>
#include <QToolBar>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
unsigned comparisons=0;
unsigned onscreenComparisons=0;
void settle(){QEventLoop loop;QTimer::singleShot(100,&loop,&QEventLoop::quit);loop.exec();}
void screenCompare(QMainWindow& window,GlViewport& viewport,QColor expected){
    settle();const auto screenshot=window.screen()->grabWindow(window.winId()).toImage();
    require(!screenshot.isNull(),"Cannot capture composited window");
    const auto origin=viewport.mapTo(&window,QPoint());const auto rect=viewport.imageRect();
    const double ratio=screenshot.devicePixelRatio();
    for(double fraction:{0.25,0.5,0.75}){
        const QPoint pixel(qRound((origin.x()+rect.x()+rect.width()*fraction)*ratio),qRound((origin.y()+rect.y()+rect.height()*fraction)*ratio));
        require(screenshot.rect().contains(pixel),"Screen sample outside window");
        const auto color=screenshot.pixelColor(pixel);
        if(color!=expected){
            screenshot.save("working/tests/viewport-presentation/black-composited.png");
            const auto framebuffer=viewport.grabFramebuffer();framebuffer.save("working/tests/viewport-presentation/black-framebuffer.png");
            std::fprintf(stderr,"Composited pixel %d,%d: %08x expected %08x, screenshot %dx%d DPR %.2f, window %dx%d at %d,%d, viewport %dx%d FBO %dx%d, error %s\n",pixel.x(),pixel.y(),color.rgba(),expected.rgba(),screenshot.width(),screenshot.height(),ratio,window.width(),window.height(),window.x(),window.y(),viewport.width(),viewport.height(),framebuffer.width(),framebuffer.height(),qPrintable(viewport.error()));
            throw std::runtime_error("Composited window is black or stale");}
    }
    ++onscreenComparisons;
}
void startupAndStreaming(){
    require(QApplication::primaryScreen()->geometry().width()>=1000 && QApplication::primaryScreen()->geometry().height()>=800,"Onscreen fullscreen checks require a larger virtual display");
    QMainWindow window;auto* container=new QWidget(&window);auto* layout=new QVBoxLayout(container);layout->setContentsMargins(0,0,0,0);container->setMinimumSize(800,600);
    auto* placeholder=new QLabel("Waiting for native frame",container);layout->addWidget(placeholder);
    auto* viewport=new GlViewport(container);layout->addWidget(viewport);viewport->hide();window.setCentralWidget(container);
    auto* toolbar=window.addToolBar("Game");toolbar->setMovable(false);
    auto* log=new QDockWidget("Launch log",&window);log->setWidget(new QPlainTextEdit(log));window.addDockWidget(Qt::BottomDockWidgetArea,log);
    ViewportPresentation controls(window,*viewport,{GlViewport::Scaling::Smooth,true});controls.addControls(*toolbar);controls.hideInFullscreen(*toolbar);controls.hideInFullscreen(*log);
    window.resize(900,750);controls.show();settle();require(!toolbar->isHidden(),"Startup fullscreen hid launch controls");
    viewport->show();settle();require(viewport->ready(),"Empty native viewport did not initialize");
    require(!toolbar->isHidden() && !log->isHidden(),"Blank native viewport hid launch controls");
    mnm::render::GlBlitter renderer(viewport->context());const auto surface=renderer.create({4,3,std::vector<std::uint32_t>(12,0x1f437f)},{24,{0xff0000,0xff00,0xff}});
    viewport->setGpuFrame(renderer.presentGpu(surface));placeholder->hide();
    screenCompare(window,*viewport,QColor(31,67,127));require(toolbar->isHidden() && log->isHidden(),"Ready frame did not hide fullscreen chrome");
    QColor previous(31,67,127);
    for(unsigned i=0;i<8;++i){
        controls.setFullscreen(i%2==0);screenCompare(window,*viewport,previous);
        const QColor color(17+i*13,41+i*9,91+i*7);
        renderer.update(surface,0,0,{4,3,std::vector<std::uint32_t>(12,(color.red()<<16)|(color.green()<<8)|color.blue())});
        viewport->setGpuFrame(renderer.presentGpu(surface));screenCompare(window,*viewport,color);
        previous=color;
    }
    controls.setFullscreen(true);viewport->setGpuFrame({});settle();require(!toolbar->isHidden() && !log->isHidden(),"Empty frame left fullscreen UI black");
}
void compare(GlViewport& viewport,const QImage& source){
    QApplication::processEvents();viewport.repaint();const auto output=viewport.grabFramebuffer();
    require(viewport.ready() && viewport.error().isEmpty(),"OpenGL presentation failed");
    double scale=std::min(double(output.width())/source.width(),double(output.height())/source.height());
    if(viewport.scaling()==GlViewport::Scaling::Integer && scale>=1)scale=std::floor(scale);
    const int w=qRound(source.width()*scale),h=qRound(source.height()*scale);
    const int left=(output.width()-w)/2,top=(output.height()-h+1)/2;
    const auto rect=viewport.imageRect();const auto ratio=viewport.devicePixelRatioF();
    require(qRound(rect.x()*ratio)==left && qRound(rect.y()*ratio)==top && qRound(rect.width()*ratio)==w && qRound(rect.height()*ratio)==h,"Input and framebuffer rectangles diverged");
    for(int y=0;y<output.height();++y)for(int x=0;x<output.width();++x){
        double expected[4]={0,0,0,255};
        if(x>=left && x<left+w && y>=top && y<top+h){
            const double sx=(x-left+0.5)*source.width()/w,sy=(y-top+0.5)*source.height()/h;
            if(viewport.scaling()==GlViewport::Scaling::Smooth){
                const int x0=int(std::floor(sx-0.5)),y0=int(std::floor(sy-0.5));
                const double fx=sx-0.5-x0,fy=sy-0.5-y0;
                for(auto& channel:expected)channel=0;
                for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx){
                    const auto color=source.pixelColor(qBound(0,x0+dx,source.width()-1),qBound(0,y0+dy,source.height()-1));
                    const int channels[]={color.red(),color.green(),color.blue(),color.alpha()};
                    const double weight=(dx?fx:1-fx)*(dy?fy:1-fy);
                    for(int c=0;c<4;++c)expected[c]+=weight*channels[c];
                }
            }else{
                const auto color=source.pixelColor(std::min(int(sx),source.width()-1),std::min(int(sy),source.height()-1));
                expected[0]=color.red();expected[1]=color.green();expected[2]=color.blue();expected[3]=color.alpha();
            }
        }
        const auto color=output.pixelColor(x,y);const int channels[]={color.red(),color.green(),color.blue(),color.alpha()};
        const int tolerance=viewport.scaling()==GlViewport::Scaling::Smooth?1:0;
        for(int c=0;c<4;++c)if(std::abs(channels[c]-qRound(expected[c]))>tolerance){
            std::fprintf(stderr,"Pixel mismatch at %d,%d channel %d: %d vs %.3f\n",x,y,c,channels[c],expected[c]);
            throw std::runtime_error("Framebuffer disagrees with independent scaling oracle");
        }
    }
    QPoint point;
    for(const auto logical: {QPoint(0,0),QPoint(source.width()/2,source.height()/2),QPoint(source.width()-1,source.height()-1)}){
        const QPointF position(rect.x()+(logical.x()+0.5)*rect.width()/source.width(),rect.y()+(logical.y()+0.5)*rect.height()/source.height());
        require(viewport.imagePoint(position,point) && point==logical,"Logical input coordinates changed");
    }
    require(!viewport.imagePoint({rect.right()+0.1,rect.center().y()},point),"Right border admitted");
    require(viewport.imagePoint({rect.right()+10,rect.bottom()+10},point,true) && point==QPoint(source.width()-1,source.height()-1),"Drag clamp changed");
    ++comparisons;
}
struct KeyObserver:QObject {
    unsigned presses=0,releases=0,fullscreenKeys=0;
    bool eventFilter(QObject*,QEvent* event) override {
        if(event->type()==QEvent::KeyPress || event->type()==QEvent::KeyRelease){
            if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_F9)++fullscreenKeys;
            if(event->type()==QEvent::KeyPress)++presses;else ++releases;
            return true;
        }
        return false;
    }
};
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    try {
        startupAndStreaming();
        QMainWindow window;auto* viewport=new GlViewport(&window);window.setCentralWidget(viewport);
        auto* toolbar=window.addToolBar("Presentation");toolbar->setMovable(false);
        auto* log=new QDockWidget("Log",&window);log->setWidget(new QPlainTextEdit(log));window.addDockWidget(Qt::BottomDockWidgetArea,log);
        auto* hidden=new QDockWidget("Hidden",&window);window.addDockWidget(Qt::LeftDockWidgetArea,hidden);hidden->hide();
        KeyObserver observer;viewport->installEventFilter(&observer);
        ViewportPresentation controls(window,*viewport);controls.addControls(*toolbar);
        controls.hideInFullscreen(*toolbar);controls.hideInFullscreen(*log);controls.hideInFullscreen(*hidden);
        window.resize(337,480);controls.show();settle();
        QImage image(7,5,QImage::Format_RGBA8888);mnm::render::Image native{7,5,{}};
        for(int y=0;y<5;++y)for(int x=0;x<7;++x){
            const QColor color((x*43+y*37)%256,(x*71+y*19)%256,(x*13+y*103)%256);
            image.setPixelColor(x,y,color);native.pixels.push_back((color.red()<<16)|(color.green()<<8)|color.blue());
        }
        mnm::render::GlBlitter renderer(viewport->context());
        const auto surface=renderer.create(native,{24,{0xff0000,0xff00,0xff}});auto lease=renderer.presentGpu(surface);
        const auto before=renderer.stats();auto* choices=toolbar->findChild<QComboBox*>("viewportScaling");require(choices,"Scaling selector missing");
        for(int mode=0;mode<3;++mode){
            choices->setCurrentIndex(mode);require(int(viewport->scaling())==mode,"Scaling control failed");
            viewport->setFrame(image);compare(*viewport,image);const auto uploads=viewport->imageUploads();
            viewport->setScaling(mode==1?GlViewport::Scaling::Sharp:GlViewport::Scaling::Smooth);compare(*viewport,image);
            require(viewport->imageUploads()==uploads,"Changing display scaling uploaded a new CPU frame");
            viewport->setScaling(GlViewport::Scaling(mode));
            viewport->setGpuFrame(lease);compare(*viewport,image);require(viewport->imageUploads()==uploads,"GPU scaling uploaded pixels");
            window.resize(351+mode,485+mode);compare(*viewport,image);
        }
        renderer.destroy(surface);compare(*viewport,image);
        require(renderer.stats().nativeReadbacks==before.nativeReadbacks && renderer.stats().rgbaReadbacks==before.rgbaReadbacks,"Scaling read back renderer storage");
        const auto normalSize=window.size();const auto frameSize=viewport->frameSize();
        auto* fullscreen=toolbar->findChild<QPushButton*>("fullscreenButton");
        require(fullscreen && fullscreen->text()=="Fullscreen" && !fullscreen->isChecked(),"Fullscreen button missing");
        fullscreen->click();settle();require(window.isFullScreen() && fullscreen->isChecked() && fullscreen->text()=="Exit fullscreen","Fullscreen button did not enter fullscreen");
        viewport->hide();settle();require(fullscreen->isVisible(),"Exit button unavailable while viewport hidden");
        fullscreen->click();settle();require(!window.isFullScreen() && !fullscreen->isChecked() && fullscreen->text()=="Fullscreen","Fullscreen button did not exit fullscreen");
        viewport->show();settle();require(window.size()==normalSize,"Button toggle lost normal window size");
        auto key=[&](QEvent::Type type,int code,bool repeat=false){QKeyEvent event(type,code,Qt::NoModifier,QString(),repeat);QApplication::sendEvent(viewport,&event);};
        key(QEvent::KeyPress,Qt::Key_F9);key(QEvent::KeyRelease,Qt::Key_F9);settle();
        require(window.isFullScreen() && toolbar->isHidden() && log->isHidden() && hidden->isHidden(),"Fullscreen chrome failed");
        require(fullscreen->isChecked() && fullscreen->text()=="Exit fullscreen","F9 did not synchronize button state");
        require(viewport->frameSize()==frameSize,"Fullscreen changed logical dimensions");compare(*viewport,image);
        key(QEvent::KeyPress,Qt::Key_F9,true);require(window.isFullScreen(),"F9 repeat toggled fullscreen");
        key(QEvent::KeyPress,Qt::Key_Escape);key(QEvent::KeyRelease,Qt::Key_Escape);
        require(window.isFullScreen() && observer.presses==1 && observer.releases==1,"Escape was consumed by host");
        key(QEvent::KeyPress,Qt::Key_F9);key(QEvent::KeyRelease,Qt::Key_F9);settle();
        require(!window.isFullScreen() && !toolbar->isHidden() && !log->isHidden() && hidden->isHidden(),"Fullscreen restore changed chrome");
        if(window.size()!=normalSize || observer.fullscreenKeys!=0)std::fprintf(stderr,"Restored size %dx%d, previous %dx%d, F9 deliveries %u\n",window.width(),window.height(),normalSize.width(),normalSize.height(),observer.fullscreenKeys);
        require(window.size()==normalSize && observer.fullscreenKeys==0,"Window geometry or reserved key forwarding failed");compare(*viewport,image);
        controls.setFullscreen(true);viewport->hide();settle();
        require(!toolbar->isHidden() && !log->isHidden() && hidden->isHidden(),"Launch controls unavailable without a frame");
        viewport->show();settle();require(toolbar->isHidden() && log->isHidden(),"Game chrome not hidden after frame appears");
        controls.setFullscreen(false);
        window.showMaximized();settle();controls.setFullscreen(true);controls.setFullscreen(false);settle();
        require(window.isMaximized(),"Maximized state lost");window.showNormal();
        // A frame larger than the window fits intact even in integer mode.
        QImage large(800,600,QImage::Format_RGBA8888);large.fill(QColor(31,63,127));viewport->setFrame(large);
        window.resize(350,350);viewport->setScaling(GlViewport::Scaling::Integer);compare(*viewport,large);
        require(viewport->imageRect().width()<=viewport->width() && viewport->imageRect().height()<=viewport->height(),"Integer fallback cropped frame");
        std::printf("{\"success\":true,\"full_frame_comparisons\":%u,\"onscreen_comparisons\":%u,\"empty_frame_controls\":true,\"cpu_gpu_scaling\":true,\"logical_input_preserved\":true,\"fullscreen_restore\":true,\"f9_consumed\":true,\"escape_forwarded\":true,\"presentation_readbacks\":0,\"gpu_scaling_uploads\":0}\n",comparisons,onscreenComparisons);
    }catch(const std::exception& error){std::fprintf(stderr,"Presentation test failed: %s\n",error.what());return 1;}
}

#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <cstdio>

int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication application(argc,argv);GlViewport viewport;viewport.resize(320,240);viewport.show();
    // Same-sized frames exercise texture reuse; changed dimensions exercise allocation.
    for(int i=0;i<12;++i){
        QImage frame(i<8?80:64,i<8?60:48,QImage::Format_RGBA8888);
        const QColor expected((i*29)%256,(i*47)%256,(i*71)%256);frame.fill(expected);
        viewport.setFrame(frame);application.processEvents();
        const QImage result=viewport.grabFramebuffer();
        if(!viewport.ready() || result.isNull() || result.pixelColor(result.width()/2,result.height()/2)!=expected){
            std::fprintf(stderr,"Texture upload/readback failed for frame %d: %s\n",i,qPrintable(viewport.error()));return 1;
        }
    }
    std::puts("Repeated texture uploads and resize passed");return 0;
}

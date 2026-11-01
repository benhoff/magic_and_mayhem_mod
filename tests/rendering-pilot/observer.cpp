#include "gl_viewport.hpp"
#include "live_command_renderer.hpp"
#include <QApplication>
#include <memory>
int main(int argc,char** argv){
    QApplication app(argc,argv);GlViewport viewport;
    if(viewport.acceptanceState || viewport.acceptanceOwner)return 1;
    auto first=std::make_unique<LiveCommandRenderer>(viewportRenderTarget(viewport));
    if(!viewport.acceptanceState || viewport.acceptanceOwner!=first.get())return 2;
    auto second=std::make_unique<LiveCommandRenderer>(viewportRenderTarget(viewport));
    if(!viewport.acceptanceState || viewport.acceptanceOwner!=second.get())return 3;
    first.reset();
    if(!viewport.acceptanceState || viewport.acceptanceOwner!=second.get())return 4;
    second.reset();
    return viewport.acceptanceState || viewport.acceptanceOwner?5:0;
}

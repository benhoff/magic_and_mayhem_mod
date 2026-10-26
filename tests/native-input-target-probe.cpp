#include "window_host.hpp"
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <cstddef>
#include <X11/Xlib.h>
// Read-only query on the explicitly selected private display. No focus, lower,
// reparent, input, game memory or native draw command is changed by this probe.
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    if(argc!=3)return 2;
    bool widthOk=false,heightOk=false;
    const int width=QString::fromLocal8Bit(argv[1]).toInt(&widthOk);
    const int height=QString::fromLocal8Bit(argv[2]).toInt(&heightOk);
    if(!widthOk||!heightOk||width<1||height<1||width>2048||height>2048)return 2;
    WindowHost host;QJsonArray rows;
    for(auto desktop:host.desktops({}))rows.append(QJsonObject{{"desktop",qint64(desktop)},{"target",qint64(host.inputWindow(desktop,QSize(width,height)))}});
    const auto data=QJsonDocument(QJsonObject{{"available",host.available()},{"width",width},{"height",height},{"desktops",rows},{"attributes_bytes",qint64(sizeof(XWindowAttributes))},{"map_state_offset",qint64(offsetof(XWindowAttributes,map_state))},{"all_event_masks_offset",qint64(offsetof(XWindowAttributes,all_event_masks))}}).toJson(QJsonDocument::Compact);
    return std::fwrite(data.constData(),1,size_t(data.size()),stdout)==size_t(data.size())?0:1;
}

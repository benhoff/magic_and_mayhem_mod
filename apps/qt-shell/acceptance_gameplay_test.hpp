#pragma once
#include <chrono>
#include "gl_viewport.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>
#include <QtEndian>
#include <functional>
#include <memory>

// Opt-in actions through the ordinary viewport. These receipts describe input
// intent and presentation fences; original-state admission happens offline.
inline void acceptanceGameplay(QMainWindow& window,const QString& directory,
    std::shared_ptr<QJsonArray> rows,std::function<void()> completed,
    std::function<void(const QString&)> failed){
    struct Action {const char* name;int key;quint32 scan;int x,y;bool shift;int hold,settle;};
    static const Action actions[]={
        {"center-first",Qt::Key_W,25,-1,-1,false,180,400},
        {"center-second",Qt::Key_W,25,-1,-1,false,180,1600},
        {"hover-creature",0,0,400,265,false,0,1000},
        {"select-creature",0,0,400,265,true,150,1000},
        {"pointer-away-selected",0,0,450,350,false,0,1000},
        {"hover-creature-again",0,0,400,265,false,0,800},
        {"deselect-creature",0,0,400,265,true,150,1000},
        {"pointer-away-deselected",0,0,450,350,false,0,1000},
        {"hover-tooltip",0,0,573,575,false,0,1800},
        {"close-tooltip",0,0,450,350,false,0,1000},
        {"rotate-1",Qt::Key_Period,60,-1,-1,false,180,1800},
        {"rotate-2",Qt::Key_Period,60,-1,-1,false,180,1800},
        {"rotate-3",Qt::Key_Period,60,-1,-1,false,180,1800},
        {"rotate-4",Qt::Key_Period,60,-1,-1,false,180,1800},
        {"pan-up",Qt::Key_Up,111,-1,-1,false,600,1200},
        {"pause",Qt::Key_Space,65,-1,-1,false,180,1800},
        {"resume",Qt::Key_Space,65,-1,-1,false,180,1800}
    };
    auto ordinal=std::make_shared<unsigned>(0);
    auto run=std::make_shared<std::function<void()>>();
    *run=[&window,directory,rows,completed,failed,ordinal,run]{
        if(*ordinal==sizeof(actions)/sizeof(actions[0])){auto done=completed;*run={};done();return;}
        const auto action=actions[(*ordinal)++];
        auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");
        if(!viewport||!viewport->isVisible()||viewport->frameSize()!=QSize(800,600)){
            auto reject=failed;*run={};reject("Gameplay witness requires a visible actual 800x600 World");return;
        }
        viewport->setFocus(Qt::OtherFocusReason);
        auto record=[rows,directory,viewport,action](const char* phase,bool accepted){
            QJsonObject row{{"action",action.name},{"phase",phase},{"accepted",accepted},
                {"host_us",QString::number(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())},
                {"world",QString::number(viewport->swappedWorldGeneration())},
                {"paint",QString::number(viewport->swappedPaint())}};
            QFile observations(QDir(directory).filePath("input-admission.bin"));
            if(observations.open(QIODevice::ReadOnly)){
                const auto bytes=observations.read(16+4096*144+1);
                if(bytes.size()>=160 && bytes.size()<=16+4096*144 && (bytes.size()-16)%144==0 && bytes.left(8)=="MNMCAI01"){
                    const auto* last=bytes.constData()+bytes.size()-144;
                    row["original_tick"]=QString::number(qFromLittleEndian<quint32>(last+4));
                    row["original_row"]=int(qFromLittleEndian<quint32>(last));
                }
            }
            rows->append(row);
        };
        auto key=[viewport](QEvent::Type type,int value,quint32 scan){
            QKeyEvent event(type,value,Qt::NoModifier,scan,0,0);event.ignore();
            QApplication::sendEvent(viewport,&event);return event.isAccepted();
        };
        auto mouse=[viewport,action](QEvent::Type type){
            const auto rect=viewport->imageRect();
            const QPointF point(rect.left()+(action.x+0.5)*rect.width()/800,
                                rect.top()+(action.y+0.5)*rect.height()/600);
            QPoint mapped;const bool maps=viewport->imagePoint(point,mapped)&&mapped==QPoint(action.x,action.y);
            const bool click=action.hold>0;
            QMouseEvent event(type,point,QPointF(viewport->mapToGlobal(point.toPoint())),
                click?Qt::LeftButton:Qt::NoButton,
                type==QEvent::MouseButtonPress?Qt::LeftButton:Qt::NoButton,
                action.shift?Qt::ShiftModifier:Qt::NoModifier);
            event.ignore();QApplication::sendEvent(viewport,&event);return maps&&event.isAccepted();
        };
        auto press=[action,key,mouse,record,run,failed,&window]{
            const bool accepted=action.key?key(QEvent::KeyPress,action.key,action.scan):
                mouse(action.hold?QEvent::MouseButtonPress:QEvent::MouseMove);
            if(!accepted){*run={};failed("Gameplay input was refused by the viewport");return;}
            QTimer::singleShot(action.hold,&window,[action,key,mouse,record,run,failed,&window]{
                const bool buttonReleased=action.key?key(QEvent::KeyRelease,action.key,action.scan):
                    !action.hold||mouse(QEvent::MouseButtonRelease);
                const bool released=buttonReleased&&(!action.shift||key(QEvent::KeyRelease,Qt::Key_Shift,50));
                record("released",released);
                if(!released){*run={};failed("Gameplay release was refused by the viewport");return;}
                QTimer::singleShot(action.settle,&window,[record,run]{record("settled",true);(*run)();});
            });
        };
        // Publish an actual Shift press before the mouse event, allowing the
        // original mapped-key consumer to observe the modifier independently.
        const bool modifier=!action.shift||key(QEvent::KeyPress,Qt::Key_Shift,50);
        record("begin",modifier);
        if(!modifier){auto reject=failed;*run={};reject("Gameplay Shift press was refused");return;}
        QTimer::singleShot(action.shift?400:0,&window,press);
    };
    (*run)();
}

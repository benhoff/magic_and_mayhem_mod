#pragma once
// Passive native UI metadata only. It has no font/render/owner decision consumer.
#include <array>
#include <chrono>
#include <cstdio>
#include <QApplication>
#include <QAbstractButton>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QRawFont>
#include <QSet>
#include <QThread>
namespace font_trace {
struct State {bool enabled=qEnvironmentVariableIntValue("MNM_FONT_LIFETIME")==1;quint64 sequence=0,generations=0,owners=0;};
inline State& state(){static State value;return value;}
inline bool enabled(){return state().enabled;}
inline quint64 generation(){return enabled()?++state().generations:0;}
inline quint64 owner(){return enabled()?++state().owners:0;}
inline void record(const char* kind,QJsonObject fields){
 if(!enabled())return;
 auto& s=state();
 if(s.sequence>4096)return;
 fields.insert("schema",1);fields.insert("sequence",QString::number(++s.sequence));
 fields.insert("kind",s.sequence>4096?"overflow":kind);
 fields.insert("host_ns",QString::number(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()));
 fields.insert("pid",double(QCoreApplication::applicationPid()));
 fields.insert("gui_thread",qApp&&QThread::currentThread()==qApp->thread());
 const auto bytes=QJsonDocument(fields).toJson(QJsonDocument::Compact);
 std::fprintf(stderr,"MNM_QTFONT01 %s\n",bytes.constData());
}
inline QString caption(QWidget* widget){
 if(auto* button=qobject_cast<QAbstractButton*>(widget))return button->text();
 if(auto* label=qobject_cast<QLabel*>(widget))return label->text();
 return {};
}
inline void paint(quint64 generation,quint64 owner,QWidget* widget,
 const std::array<QString,4>& families,QSet<QString>& seen){
 if(!enabled()||!generation||!widget->isVisible())return;
 const auto text=caption(widget);if(text.isEmpty())return;
 const auto font=widget->font();int role=-1;
 for(int i=0;i<4;++i)if(font.family()==families[i])role=i;
 if(role<0)return;
 const auto point=widget->mapToGlobal(QPoint(0,0));
 const auto geometry=QString(":%1:%2:%3:%4").arg(point.x()).arg(point.y()).arg(widget->width()).arg(widget->height());
 const auto key=geometry+widget->objectName()+":"+QString::number(role)+":"+QString::number(font.pixelSize())+":"+text;
 if(seen.contains(key))return;
 if(seen.size()>=512){record("overflow",{});return;}seen.insert(key);
 const auto raw=QRawFont::fromFont(font);QJsonArray glyphs;
 for(auto index:raw.glyphIndexesForString(text.left(256)))glyphs.append(double(index));
 record("paint",{{"generation",QString::number(generation)},{"owner",QString::number(owner)},
 {"widget",widget->objectName()},{"class",widget->metaObject()->className()},
 {"text",text.left(256)},{"truncated",text.size()>256},{"role",role},
 {"family",font.family()},{"resolved_family",raw.familyName()},{"font_valid",raw.isValid()},
 {"font_size",font.pixelSize()},{"glyphs",glyphs},
 {"rect",QJsonArray{point.x(),point.y(),widget->width(),widget->height()}}});
}
}

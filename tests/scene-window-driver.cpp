#include "scene-window-driver.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QBuffer>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>
#include <iostream>
#include <set>
#include <stdexcept>
namespace mnm::scene::test {
namespace {
void check(bool b,const char* reason) {if(!b) throw std::runtime_error(reason);}
void write(const QString& path,const QByteArray& bytes) {
    QFile f(path);check(f.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Window artifact exists or cannot be opened");
    check(f.write(bytes)==bytes.size() && f.flush(),"Window artifact write failed");
}
QString label(const QJsonObject& action) {
    const auto name=action.value("name").toString();check(QRegularExpression("^[a-z][a-z0-9-]{0,31}$").match(name).hasMatch(),"Invalid window artifact name");return name;
}
void mouse(QWidget& widget,int x,int y,Qt::MouseButton button) {
    const QPointF local(x+0.5,y+0.5),global(widget.mapToGlobal(QPoint(x,y)));check(widget.isVisible() && widget.isEnabled(),"Window action widget unavailable");
    QMouseEvent press(QEvent::MouseButtonPress,local,global,button,button,Qt::NoModifier);QApplication::sendEvent(&widget,&press);
    QMouseEvent release(QEvent::MouseButtonRelease,local,global,button,Qt::NoButton,Qt::NoModifier);QApplication::sendEvent(&widget,&release);
}
class Driver final:public QObject {
    QWidget& window_;SceneCanvas& canvas_;const game::MovementSession& session_;std::function<Frame()> frame_;
    Playback& playback_;std::function<void()>& observer_;QString output_;QJsonArray actions_,captures_;int index_=0;
    unsigned remaining_=0;bool finished_=false;std::set<QString> names_;
    void fail(const std::string& reason) {if(finished_) return;finished_=true;observer_={};playback_.pause();std::cerr<<reason<<'\n';QApplication::closeAllWindows();QApplication::exit(1);}
    template<class F> void guard(F f) {try {f();} catch(const std::exception& e) {fail(e.what());}}
    QPushButton& button(const QString& name) {auto* b=window_.findChild<QPushButton*>(name);check(b,"Window button missing");return *b;}
    void click(const QString& name) {auto& b=button(name);mouse(b,b.width()/2,b.height()/2,Qt::LeftButton);}
    void schedule(int delay=0) {QTimer::singleShot(delay,this,[this] {next();});}
    void capture(const QString& name) {
        check(names_.insert(name).second,"Duplicate window capture name");const auto prefix=output_+'/'+name;
        check(game::writeSnapshot((prefix+".mnw").toStdString(),session_.world().state()).durable,"Window checkpoint capture failed");
        const auto frame=frame_();QByteArray pixels;for(const auto p:frame.pixels.pixels) {pixels.append(char(p&255));pixels.append(char(p>>8));}write(prefix+".565",pixels);
        QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);check(frame.image.save(&buffer,"PNG"),"Window PNG capture failed");write(prefix+".png",png);
        check(canvas_.grab().save(prefix+".canvas.png") && window_.grab().save(prefix+".window.png"),"Actual QWidget screenshot failed");
        QJsonArray queue,actors,pending;
        for(const auto& d:frame.queue) {
            QJsonObject draw{{"creature",d.creature},{"frame",qint64(d.frame)},{"x",d.x},{"y",d.y},{"key",d.key}};
            if(d.actor) {draw.insert("slot",int(d.actor->slot));draw.insert("generation",qint64(d.actor->generation));}
            if(d.standing) draw.insert("cell",QJsonArray{d.standing->x,d.standing->y,d.standing->z});
            queue.append(draw);
        }
        const auto& state=session_.world().state();
        for(unsigned i=0;i<state.slots.size();++i) if(state.slots[i].entity && state.slots[i].entity->motion) {
            const auto& e=*state.slots[i].entity;const auto fine=session_.finePosition(e);
            QJsonObject actor{{"slot",int(i)},{"generation",qint64(state.slots[i].generation)},{"fine",QJsonArray{fine.x,fine.y,fine.z}}};
            actors.append(actor);
        }
        for(const auto& c:state.pending) {
            QJsonObject item{{"operation",int(c.operation)},{"slot",int(c.subject.slot)},{"generation",qint64(c.subject.generation)}};
            if(c.destination) item.insert("destination",QJsonArray{c.destination->x,c.destination->y,c.destination->z});
            pending.append(item);
        }
        auto* choices=window_.findChild<QComboBox*>("creatureSelection");check(choices,"Creature choices missing");
        const QJsonObject canonical{{"tick",qint64(state.tick)},{"queue",queue},{"actors",actors}};
        write(prefix+".json",QJsonDocument(canonical).toJson());
        captures_.append(QJsonObject{{"name",name},{"tick",qint64(state.tick)},{"playing",playback_.playing()},{"selected_index",choices->currentIndex()},{"pending",pending}});
    }
    void acceptSave(const QString& path,unsigned tries=0) {
        guard([&] {
            check(tries<100,"Native Save dialog did not appear");auto* dialog=qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
            if(!dialog) {QTimer::singleShot(10,this,[this,path,tries] {acceptSave(path,tries+1);});return;}
            check(!playback_.playing(),"Save did not pause before modal interaction");dialog->selectFile(path);
            check(QMetaObject::invokeMethod(dialog,"accept",Qt::DirectConnection),"Cannot accept native Save dialog");
        });
    }
    void next() {
        if(finished_) return;
        guard([&] {
            if(index_==actions_.size()) {check(!playback_.playing(),"Window script ended while playing");write(output_+"/window-report.json",QJsonDocument(QJsonObject{{"all_match",true},{"captures",captures_}}).toJson());finished_=true;observer_={};QApplication::exit(0);return;}
            check(actions_[index_].isObject(),"Window action must be an object");const auto a=actions_[index_++].toObject();const auto op=a.value("op").toString();
            if(op=="capture") capture(label(a));
            else if(op=="button") click(a.value("button").toString());
            else if(op=="mouse") {
                const int x=a.value("x").toInt(-1),y=a.value("y").toInt(-1);check(x>=0 && x<512 && y>=0 && y<256,"Window click outside canvas");
                const auto b=a.value("button").toString();check(b=="left" || b=="right","Unknown canvas mouse button");mouse(canvas_,x,y,b=="left"?Qt::LeftButton:Qt::RightButton);
            } else if(op=="wait") {const int ms=a.value("ms").toInt(-1);check(ms>=0 && ms<=500,"Window wait outside bounds");schedule(ms);return;}
            else if(op=="ticks" || op=="run") {
                const int ticks=a.value("count").toInt(0);check(ticks>0 && ticks<=32,"Window tick wait outside bounds");remaining_=unsigned(ticks);
                if(!playback_.playing()) click("playSimulation");
                const auto before=a.value("before").toArray();check(before.size()<=4,"Too many atomic window inputs");
                for(const auto& item:before) {
                    const auto input=item.toObject();const auto kind=input.value("op").toString();
                    if(kind=="button") click(input.value("button").toString());
                    else if(kind=="capture") capture(label(input));
                    else throw std::runtime_error("Invalid atomic window input");
                }
                return;
            } else if(op=="save") {
                if(a.value("play").toBool() && !playback_.playing()) click("playSimulation");
                const auto path=output_+'/'+label(a)+".mnms";check(!QFile::exists(path),"Window Save would overwrite a file");
                QTimer::singleShot(0,this,[this,path] {acceptSave(path);});click("saveCheckpoint");
                check(QFile::exists(path) && !playback_.playing(),"Window Save did not complete while paused");
            } else throw std::runtime_error("Unknown window action");
            schedule();
        });
    }
public:
    Driver(const QString& script,const QString& output,QWidget& w,SceneCanvas& c,const game::MovementSession& s,std::function<Frame()> f,Playback& p,std::function<void()>& o):window_(w),canvas_(c),session_(s),frame_(std::move(f)),playback_(p),observer_(o),output_(output) {
        QFile file(script);check(file.open(QIODevice::ReadOnly) && file.size()<=65536,"Window script outside bounds");QJsonParseError error;const auto doc=QJsonDocument::fromJson(file.readAll(),&error);
        check(error.error==QJsonParseError::NoError && doc.isArray() && doc.array().size()>0 && doc.array().size()<=64,"Invalid bounded window script");actions_=doc.array();
        check(!QFile::exists(output_) && QDir().mkpath(output_),"Window output directory already exists or cannot be created");
        observer_=[this] {guard([&] {if(remaining_ && --remaining_==0) {click("pauseSimulation");schedule();}});};
        QTimer::singleShot(45000,this,[this] {fail("Native window script timeout");});schedule();
    }
    ~Driver() override {observer_={};}
};
}
std::shared_ptr<QObject> startWindowScript(const QString& script,const QString& output,QWidget& w,SceneCanvas& c,const game::MovementSession& s,std::function<Frame()> f,Playback& p,std::function<void()>& o) {
    return std::make_shared<Driver>(script,output,w,c,s,std::move(f),p,o);
}
}

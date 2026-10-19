#include "campaign-window-driver.hpp"
#include "campaign-graph-pilot.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <fstream>
#include <stdexcept>
namespace mnm::skirmish::test {
namespace {
void need(bool ok,const char *why) {if(!ok) throw std::runtime_error(why);}
auto bytes(const Controller &c) {need(c.campaign(),"missing campaign");return game::encodeCampaignSnapshot(c.campaign()->checkpoint(),c.campaign()->progress().identity);}
void mouse(QWidget *widget,QPoint at,Qt::MouseButton button=Qt::LeftButton) {
    for(auto *parent=widget->parentWidget();parent;parent=parent->parentWidget()) if(auto *area=qobject_cast<QScrollArea*>(parent)) area->ensureWidgetVisible(widget);
    QMouseEvent press(QEvent::MouseButtonPress,QPointF(at),QPointF(widget->mapToGlobal(at)),button,button,Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease,QPointF(at),QPointF(widget->mapToGlobal(at)),button,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(widget,&press);QApplication::sendEvent(widget,&release);
}
void click(Window &w,const QString &id) {auto *b=w.findChild<QPushButton*>(id);need(b && b->isEnabled(),qPrintable("missing/disabled button "+id));mouse(b,b->rect().center());}
void dialog(Window &w,const QString &id,const std::filesystem::path &path) {
    QTimer picker;bool chosen=false;
    QObject::connect(&picker,&QTimer::timeout,[&]{
        auto *d=qobject_cast<QFileDialog*>(QApplication::activeModalWidget());if(!d) return;
        if(!chosen) {d->setDirectory(QString::fromStdString(path.parent_path().string()));d->selectFile(QString::fromStdString(path.string()));chosen=true;}
        if(auto *field=d->findChild<QLineEdit*>("fileNameEdit")) {field->setText(QString::fromStdString(path.filename().string()));QMetaObject::invokeMethod(d,"accept",Qt::DirectConnection);}
    });picker.start(20);click(w,id);picker.stop();need(chosen,"file dialog absent");
}
struct Pilot {
    const game::CampaignSession &session;std::vector<game::SkirmishOrder> orders;
    const auto *battle() const {return session.battle();}
    const auto &progress() const {return session.progress();}
    const auto *missionPlan() const {return session.missionPlan();}
    void enqueue(game::SkirmishOrder order) {orders.push_back(order);}
};
void order(Window &w,Controller &c,const game::SkirmishOrder &o) {
    auto *canvas=dynamic_cast<ArenaCanvas*>(w.findChild<QWidget*>("arenaCanvas"));need(canvas,"canvas absent");
    const auto cell=[&](game::Handle h) {const auto *e=c.session().world().find(h);need(e,"order actor absent");return game::Point{e->x,e->y,e->z};};
    // Each planned native order goes through real unit picking and spell buttons.
    need(c.mode()==CursorMode::select,"order mode was not reset");
    mouse(canvas,canvas->presentedCenter(cell(o.subject)));
    need(c.view().selected==std::optional<game::Handle>(o.subject),"production unit picking selected wrong actor");
    auto expected=c.session().state();expected.pending.push_back(o);
    if(o.kind==game::SkirmishOrderKind::move) mouse(canvas,canvas->presentedCenter(*o.destination),Qt::RightButton);
    else if(o.kind==game::SkirmishOrderKind::attack) mouse(canvas,canvas->presentedCenter(cell(*o.target)),Qt::RightButton);
    else {
        click(w,o.kind==game::SkirmishOrderKind::summon?"summonButton":o.kind==game::SkirmishOrderKind::bolt?"boltButton":o.kind==game::SkirmishOrderKind::cure?"cureButton":"sphereButton");
        mouse(canvas,canvas->presentedCenter(o.kind==game::SkirmishOrderKind::summon?*o.destination:cell(*o.target)));
    }
    need(game::encodeSkirmish(c.session().state())==game::encodeSkirmish(expected),"widget order differs from headless order");
}
void capture(Window &w,const std::filesystem::path &path) {QApplication::processEvents();need(w.grab().save(QString::fromStdString(path.string())),"capture failed");}
}
void campaignWindow(const QString &script,const QString &output,Window &window,Controller &c) {
    QTimer::singleShot(300000,[]{QApplication::exit(2);});
    QTimer::singleShot(0,[&window,&c,script,output]{
        const std::filesystem::path out(output.toStdString());
        try {
            need(!std::filesystem::exists(out),"new output required");std::filesystem::create_directories(out);
            QFile f(script);need(f.open(QIODevice::ReadOnly) && f.size()<65536,"invalid script input");QJsonParseError error;
            const auto doc=QJsonDocument::fromJson(f.readAll(),&error);need(error.error==QJsonParseError::NoError && doc.isArray() && doc.array().size()<=256,"invalid action list");
            unsigned ticks=0,orders=0;
            for(const auto value:doc.array()) {
                const auto a=value.toObject();const auto kind=a["kind"].toString();
                if(kind=="button") click(window,a["name"].toString());
                else if(kind=="disabled") {
                    auto *b=window.findChild<QPushButton*>(a["name"].toString());need(b && !b->isEnabled(),"expected disabled action");
                    const auto before=bytes(c);mouse(b,b->rect().center());need(before==bytes(c),"disabled action changed campaign");
                } else if(kind=="route") {
                    auto *list=window.findChild<QListWidget*>("campaignMissionList");need(list && list->isVisible(),"mission picker absent");
                    QListWidgetItem *item=nullptr;for(int i=0;i<list->count();++i) if(list->item(i)->data(Qt::UserRole+1)==a["region"].toString()) item=list->item(i);
                    need(item,"mission absent");const bool enabled=item->flags()&Qt::ItemIsEnabled;
                    need(enabled!=a["reject"].toBool(),"wrong route availability");const auto before=bytes(c);
                    list->scrollToItem(item);mouse(list->viewport(),list->visualItemRect(item).center());
                    if(a["reject"].toBool()) need(bytes(c)==before,"disabled route changed campaign");
                    else need(QString::fromStdString(c.campaign()->progress().region)==a["region"].toString(),"route selection failed");
                } else if(kind=="pilot") {
                    const auto count=a["ticks"].toInt();need(count>=0 && count<=1024,"pilot bound");
                    for(int i=0;i<count && c.hasBattle();++i) {
                        Pilot p{*c.campaign(),{}};mnm::tests::campaignPilot(p);
                        for(const auto &o:p.orders) {order(window,c,o);++orders;}
                        click(window,"stepButton");++ticks;
                        if(i%32==0) QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                    }
                    if(a["win"].toBool()) need(!c.hasBattle() && c.campaign()->progress().lastResult->outcome==game::CampaignOutcome::victory,"production journey failed");
                } else if(kind=="steps") {
                    const auto n=a["count"].toInt();need(n>=0 && n<=256,"step bound");for(int i=0;i<n && c.hasBattle();++i) {click(window,"stepButton");++ticks;}
                } else if(kind=="spell") {
                    const auto m=c.session().state();const auto slot=unsigned(a["target"].toInt());game::Handle target{};
                    for(const auto &u:m.units) if(u.actor.slot==slot) target=u.actor;
                    need(bool(target.generation),"spell target absent");
                    order(window,c,{a["spell"]=="cure"?game::SkirmishOrderKind::cure:game::SkirmishOrderKind::sphere,m.wizards[0].actor,target,{}});++orders;
                } else if(kind=="save") {
                    const auto path=out/a["name"].toString().toStdString();dialog(window,"saveCampaignButton",path);
                    need(game::encodeCampaignSnapshot(game::readCampaignSnapshot(path,c.campaign()->progress().identity),c.campaign()->progress().identity)==bytes(c),"dialog save differs");
                } else if(kind=="load") {
                    const auto before=bytes(c);const auto path=std::filesystem::path(a["path"].toString().toStdString());dialog(window,"loadCampaignButton",path);
                    need(!c.playing(),"load did not pause");
                    if(a["reject"].toBool()) {need(bytes(c)==before,"refused load changed campaign");need(!window.findChild<QLabel*>("feedbackStatus")->text().isEmpty(),"refusal feedback absent");}
                    else need(bytes(c)==game::encodeCampaignSnapshot(game::readCampaignSnapshot(path,c.campaign()->progress().identity),c.campaign()->progress().identity),"loaded bytes differ");
                } else if(kind=="pause") {
                    c.pause();const auto before=bytes(c);QEventLoop loop;QTimer::singleShot(100,&loop,&QEventLoop::quit);loop.exec();need(before==bytes(c),"paused state advanced");
                } else if(kind=="expect") {
                    const auto &p=c.campaign()->progress();const auto v=c.view();
                    if(a.contains("battle")) need(c.hasBattle()==a["battle"].toBool(),"battle ownership mismatch");
                    if(a.contains("completed")) need(p.completed==a["completed"].toBool(),"completion mismatch");
                    if(a.contains("region")) need(QString::fromStdString(p.region)==a["region"].toString(),"region mismatch");
                    if(a.contains("transition")) need(p.transition==unsigned(a["transition"].toInt()),"transition mismatch");
                    if(a.contains("terrain")) need(bool(v.mission)==a["terrain"].toBool(),"wrong terrain presentation");
                    if(a.contains("text")) {auto *label=window.findChild<QLabel*>("missionStatus");need(label && label->isVisible() && label->text().contains(a["text"].toString()),"objective/reward text absent");}
                    if(a.contains("feedback_not")) need(!window.findChild<QLabel*>("feedbackStatus")->text().contains(a["feedback_not"].toString()),"misleading objective feedback");
                    if(a.contains("route_text")) {bool found=false;for(const auto &choice:v.choices) found|=choice.text.contains(a["route_text"].toString());need(found,"route status text absent");}
                    for(const auto &pair:{std::pair<const char*,const char*>{"cure","cureButton"},{"sphere","sphereButton"},{"continue_enabled","continueCampaignButton"}})
                        if(a.contains(pair.first)) {auto *b=window.findChild<QPushButton*>(pair.second);need(b && b->isEnabled()==a[pair.first].toBool(),"control availability mismatch");}
                    if(a.contains("label")) need(window.findChild<QPushButton*>("continueCampaignButton")->text()==a["label"].toString(),"Continue/Retry label mismatch");
                    if(a.contains("choices")) need(p.graph && p.graph->choices.size()==unsigned(a["choices"].toInt()),"route commitment mismatch");
                } else if(kind=="capture") capture(window,out/a["name"].toString().toStdString());
                else throw std::runtime_error("unknown Crossroads action");
            }
            c.pause();need(c.campaign()->save(out/"final.mnmc").durable,"final save failed");capture(window,out/"window.png");
            QFile report(QString::fromStdString((out/"report.json").string()));need(report.open(QIODevice::WriteOnly),"report failed");
            report.write(QJsonDocument(QJsonObject{{"success",true},{"ticks",int(ticks)},{"orders",int(orders)}}).toJson());QApplication::exit(0);
        } catch(const std::exception &e) {std::ofstream(out/"failure.txt")<<e.what();QApplication::exit(1);}
    });
}
}

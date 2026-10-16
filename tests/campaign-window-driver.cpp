#include "campaign-window-driver.hpp"
#include "persistence/campaign.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include <fstream>
#include <algorithm>
#include <stdexcept>
namespace mnm::skirmish::test {
namespace {
void require(bool value, const char *reason) {
    if (!value)
        throw std::runtime_error(reason);
}
void mouse(QWidget *widget, QPoint point, Qt::MouseButton button = Qt::LeftButton) {
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(point), QPointF(widget->mapToGlobal(point)),
                      button, button, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point),
                        QPointF(widget->mapToGlobal(point)), button, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(widget, &press);
    QApplication::sendEvent(widget, &release);
}
QPushButton *button(Window &window, const QString &name) {
    auto *result = window.findChild<QPushButton *>(name);
    require(result && result->isEnabled(), "Missing/disabled campaign window action");
    mouse(result, result->rect().center());
    return result;
}
void dialog(Window &window, const QString &name, const std::filesystem::path &path) {
    QTimer picker;
    bool chosen = false;
    QObject::connect(&picker, &QTimer::timeout, [&] {
        auto *d = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
        if (!d)
            return;
        if (!chosen) {
            d->setDirectory(QString::fromStdString(path.parent_path().string()));
            d->selectFile(QString::fromStdString(path.string()));
            chosen = true;
        }
        auto *field = d->findChild<QLineEdit *>("fileNameEdit");
        if (field) {
            field->setText(QString::fromStdString(path.filename().string()));
            QMetaObject::invokeMethod(d, "accept", Qt::DirectConnection);
        }
    });
    picker.start(30);
    button(window, name);
    picker.stop();
    require(chosen, "Campaign action did not open a file dialog");
}
std::vector<std::uint8_t> bytes(const Controller &c) {
    require(c.campaign(), "Expected campaign ownership");
    return game::encodeCampaignSnapshot(c.campaign()->checkpoint(),
                                        c.campaign()->progress().identity);
}
} // namespace
void campaignWindow(const QString &script, const QString &output, Window &window, Controller &c) {
    QTimer::singleShot(60000, [] { QApplication::exit(2); });
    QTimer::singleShot(0, [&window, &c, script, output] {
        const std::filesystem::path out(output.toStdString());
        try {
            require(!std::filesystem::exists(out), "New campaign window output required");
            std::filesystem::create_directories(out);
            QFile file(script);
            require(file.open(QIODevice::ReadOnly) && file.size() < 65536,
                    "Invalid campaign window script");
            QJsonParseError error;
            const auto document = QJsonDocument::fromJson(file.readAll(), &error);
            require(error.error == QJsonParseError::NoError && document.isArray(),
                    "Invalid script JSON");
            const auto actions = document.array();
            require(actions.size() <= 256, "Too many campaign actions");
            for (const auto &value : actions) {
                const auto action = value.toObject();
                const auto kind = action["kind"].toString();
                if (kind == "button")
                    button(window, action["name"].toString());
                else if (kind == "cell") {
                    game::Point point{action["x"].toInt(), action["y"].toInt(), action.contains("z")?action["z"].toInt():1};
                    if (action.contains("wizard")) {
                        const auto actor = c.session()
                                               .state()
                                               .wizards.at(unsigned(action["wizard"].toInt()))
                                               .actor;
                        const auto *e = c.session().world().find(actor);
                        require(e, "Missing wizard");
                        point = {e->x, e->y, e->z};
                    }
                    if (action.contains("actor")) {
                        const auto slot=unsigned(action["actor"].toInt());
                        require(slot<c.session().world().state().slots.size() && c.session().world().state().slots[slot].entity,"Missing target actor");
                        const auto &body=*c.session().world().state().slots[slot].entity;
                        point={body.x,body.y,body.z};
                    }
                    auto *canvas = dynamic_cast<ArenaCanvas *>(window.findChild<QWidget *>("arenaCanvas"));
                    require(canvas, "Missing arena canvas");
                    mouse(canvas, canvas->presentedCenter(point),action["right"].toBool()?Qt::RightButton:Qt::LeftButton);
                } else if (kind == "finish") {
                    for(unsigned tick=0;tick<160 && c.hasBattle();++tick) {
                        if(tick%3==0 && c.view().mana[0]>=game::SkirmishRules::boltCost && !c.view().busy) {
                            auto *canvas=dynamic_cast<ArenaCanvas *>(window.findChild<QWidget *>("arenaCanvas"));
                            require(canvas,"Missing canvas");
                            const auto meta=c.session().state();
                            auto cell=[&](unsigned owner) {const auto *e=c.session().world().find(meta.wizards[owner].actor);return game::Point{e->x,e->y,e->z};};
                            mouse(canvas,canvas->presentedCenter(cell(0)));button(window,"boltButton");
                            mouse(canvas,canvas->presentedCenter(cell(1)));
                        }
                        button(window,"stepButton");
                    }
                    require(!c.hasBattle(),"Scripted battle did not finish");
                } else if (kind == "steps") {
                    const auto count = action["count"].toInt();
                    require(count >= 0 && count <= 256 && !c.playing(), "Invalid step count/state");
                    for (int i = 0; i < count && c.hasBattle(); ++i)
                        button(window, "stepButton");
                } else if (kind == "play") {
                    const auto before = c.session().world().state().tick;
                    QEventLoop loop;
                    QTimer poll;
                    QObject::connect(&poll, &QTimer::timeout, [&] {
                        if (!c.hasBattle() || c.session().world().state().tick > before)
                            loop.quit();
                    });
                    poll.start(5);
                    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
                    button(window, "playButton");
                    loop.exec();
                    c.pause();
                    require(!c.hasBattle() || c.session().world().state().tick > before,
                            "Play did not advance");
                } else if (kind == "pause") {
                    c.pause();
                    const auto before = bytes(c);
                    QEventLoop loop;
                    QTimer::singleShot(250, &loop, &QEventLoop::quit);
                    loop.exec();
                    require(before == bytes(c), "Paused campaign advanced");
                } else if (kind == "save") {
                    const auto path = out / action["name"].toString().toStdString();
                    dialog(window, "saveCampaignButton", path);
                    require(bytes(c) == game::encodeCampaignSnapshot(
                                            game::readCampaignSnapshot(
                                                path, c.campaign()->progress().identity),
                                            c.campaign()->progress().identity),
                            "Window save differs");
                } else if (kind == "load") {
                    const auto path =
                        std::filesystem::path(action["path"].toString().toStdString());
                    const auto before = bytes(c);
                    dialog(window, "loadCampaignButton", path);
                    require(bytes(c) == (action["reject"].toBool()
                                             ? before
                                             : game::encodeCampaignSnapshot(
                                                   game::readCampaignSnapshot(
                                                       path, c.campaign()->progress().identity),
                                                   c.campaign()->progress().identity)),
                            "Campaign load changed/refused wrong checkpoint");
                    require(!c.playing(), "Campaign load did not pause");
                    if (action["reject"].toBool()) {
                        const auto *status = window.findChild<QLabel *>("feedbackStatus");
                        require(status && !status->text().isEmpty(),
                                "Missing load refusal feedback");
                    }
                } else if (kind == "expect") {
                    const auto &p = c.campaign()->progress();
                    if (action.contains("cures")) require(c.view().cures.size()==unsigned(action["cures"].toInt()),"Wrong active Cure count");
                    if (action.contains("health")) {
                        const auto view=c.view();auto it=std::find_if(view.units.begin(),view.units.end(),[&](const auto &u){return u.actor.slot==unsigned(action["actor"].toInt());});
                        require(it!=view.units.end() && it->health==unsigned(action["health"].toInt()),"Wrong actor health");
                    }
                    if (action.contains("transition"))
                        require(p.transition == unsigned(action["transition"].toInt()),
                                "Wrong campaign transition");
                    if (action.contains("completed"))
                        require(p.completed == action["completed"].toBool(), "Wrong completion");
                    if (action.contains("battle"))
                        require(c.hasBattle() == action["battle"].toBool(),
                                "Wrong battle ownership");
                    if (action.contains("label"))
                        require(window.findChild<QPushButton *>("continueCampaignButton")->text() ==
                                    action["label"].toString(),
                                "Wrong Continue/Retry label");
                    if (action.contains("continue_enabled"))
                        require(window.findChild<QPushButton *>("continueCampaignButton")
                                        ->isEnabled() == action["continue_enabled"].toBool(),
                                "Wrong Continue availability");
                } else
                    throw std::runtime_error("Unknown campaign window action");
            }
            c.pause();
            const auto result = c.campaign()->save(out / "final.mnmc");
            require(result.durable, "Final publication failed");
            require(window.grab().save(QString::fromStdString((out / "window.png").string())),
                    "Window capture failed");
            QFile report(QString::fromStdString((out / "report.json").string()));
            require(report.open(QIODevice::WriteOnly), "Report publication failed");
            report.write(
                QJsonDocument(QJsonObject{{"success", true},
                                          {"transition", int(c.campaign()->progress().transition)}})
                    .toJson());
            QApplication::exit(0);
        } catch (const std::exception &e) {
            std::ofstream(out / "failure.txt") << e.what();
            QApplication::exit(1);
        }
    });
}
} // namespace mnm::skirmish::test

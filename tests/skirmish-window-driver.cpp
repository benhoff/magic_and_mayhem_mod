#include "skirmish-window-driver.hpp"
#include "window.hpp"
#include "trace.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
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
#include <stdexcept>
namespace mnm::skirmish::test {
class Driver {
    Window &window_;
    Controller &controller_;
    std::filesystem::path output_;
    QJsonArray actions_, saves_;
    unsigned end_ = 0, ordersCount_ = 0;
    int cursor_ = 0;
    bool timer_ = true;
    std::ofstream orders_, trace_;
    QTimer watchdog_;
    static void require(bool v, const char *reason) {
        if (!v)
            throw std::runtime_error(reason);
    }
    template <class F> void guarded(F f) {
        try {
            f();
        } catch (const std::exception &e) {
            std::ofstream(output_ / "failure.txt") << e.what();
            QApplication::exit(1);
        }
    }
    void mouse(QWidget *widget, QPoint point, Qt::MouseButton button) {
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(point),
                          QPointF(widget->mapToGlobal(point)), button, button, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point),
                            QPointF(widget->mapToGlobal(point)), button, Qt::NoButton,
                            Qt::NoModifier);
        QApplication::sendEvent(widget, &press);
        QApplication::sendEvent(widget, &release);
    }
    void button(const QString &name) {
        auto *b = window_.findChild<QPushButton *>(name);
        require(b && b->isEnabled(), "missing/disabled window action");
        mouse(b, b->rect().center(), Qt::LeftButton);
    }
    void dialog(const QString &buttonName, const std::filesystem::path &path) {
        QTimer picker;
        picker.setInterval(50);
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
            auto *filename = d->findChild<QLineEdit *>("fileNameEdit");
            require(filename, "file dialog filename field unavailable");
            filename->setText(QString::fromStdString(path.filename().string()));
            QMetaObject::invokeMethod(d, "accept", Qt::DirectConnection);
        });
        picker.start();
        button(buttonName);
        picker.stop();
        require(chosen, "save/load did not open file dialog");
    }
    void capture(const std::string &name) {
        auto result =
            game::writeSnapshot(output_ / (name + ".mnms"), controller_.session().world().state());
        require(result.durable, "test snapshot publication failed");
        auto *canvas = dynamic_cast<ArenaCanvas *>(window_.findChild<QWidget *>("arenaCanvas"));
        require(canvas && canvas->grab().save(
                              QString::fromStdString((output_ / (name + ".png")).string())),
                "canvas capture failed");
    }
    void finish() {
        controller_.pause();
        capture("final");
        QJsonObject result{{"success", true},
                           {"tick", int(controller_.session().world().state().tick)},
                           {"outcome", int(controller_.session().state().outcome)},
                           {"timer", timer_},
                           {"saves", saves_}};
        QFile report(QString::fromStdString((output_ / "report.json").string()));
        require(report.open(QIODevice::WriteOnly), "cannot write window report");
        report.write(QJsonDocument(result).toJson());
        report.close();
        orders_.flush();
        trace_.flush();
        QApplication::exit(0);
    }
    void process() {
        auto tick = controller_.session().world().state().tick;
        while (cursor_ < actions_.size() &&
               unsigned(actions_[cursor_].toObject()["tick"].toInt()) < tick)
            ++cursor_;
        while (cursor_ < actions_.size() &&
               unsigned(actions_[cursor_].toObject()["tick"].toInt()) == tick) {
            const auto a = actions_[cursor_++].toObject();
            const auto kind = a["kind"].toString();
            if (kind == "button")
                button(a["name"].toString());
            else if (kind == "cell") {
                game::Point p{a["x"].toInt(), a["y"].toInt(), 1};
                if (a.contains("actor")) {
                    auto *e = controller_.session().world().find({unsigned(a["actor"].toInt()), 1});
                    require(e, "script actor unavailable");
                    p = {e->x, e->y, e->z};
                }
                auto *canvas =
                    dynamic_cast<ArenaCanvas *>(window_.findChild<QWidget *>("arenaCanvas"));
                require(canvas, "canvas unavailable");
                mouse(canvas, ArenaCanvas::cellCenter(p),
                      a["right"].toBool() ? Qt::RightButton : Qt::LeftButton);
            } else if (kind == "save") {
                const auto path = output_ / a["name"].toString().toStdString();
                dialog("saveButton", path);
                require(game::encodeSnapshot(game::readSnapshot(path)) ==
                            game::encodeSnapshot(controller_.session().world().state()),
                        "window save changed state");
                saves_.append(QJsonObject{{"name", a["name"]},
                                          {"tick", int(tick)},
                                          {"next_action", cursor_},
                                          {"orders", int(ordersCount_)}});
            } else if (kind == "load") {
                const auto path = a["path"].toString().toStdString();
                auto before = game::encodeSnapshot(controller_.session().world().state());
                dialog("loadButton", path);
                if (a["reject"].toBool())
                    require(before == game::encodeSnapshot(controller_.session().world().state()),
                            "failed window load changed match");
                else
                    require(game::encodeSnapshot(game::readSnapshot(path)) ==
                                game::encodeSnapshot(controller_.session().world().state()),
                            "window load changed checkpoint");
            } else if (kind == "capture")
                capture(a["name"].toString().toStdString());
            else if (kind == "pause") {
                controller_.pause();
                const auto bytes = game::encodeSnapshot(controller_.session().world().state());
                QEventLoop wait;
                QTimer::singleShot(250, &wait, &QEventLoop::quit);
                wait.exec();
                require(bytes == game::encodeSnapshot(controller_.session().world().state()),
                        "paused window advanced");
            } else
                throw std::runtime_error("unknown window script action");
            tick = controller_.session().world().state().tick;
        }
        if (tick >= end_ || controller_.session().state().outcome != game::Outcome::running) {
            finish();
            return;
        }
        if (timer_) {
            if (!controller_.playing())
                button("playButton");
        } else
            QTimer::singleShot(0, [this] { guarded([&] { button("stepButton"); }); });
    }

  public:
    Driver(const QString &script, const QString &output, Window &w, Controller &c)
        : window_(w), controller_(c), output_(output.toStdString()) {
        require(!script.isEmpty() && !output.isEmpty() && !std::filesystem::exists(output_),
                "new script/output required");
        QFile input(script);
        require(input.open(QIODevice::ReadOnly) && input.size() < 65536, "invalid window script");
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(input.readAll(), &error);
        require(error.error == QJsonParseError::NoError && doc.isObject(),
                "invalid window script JSON");
        const auto config = doc.object();
        actions_ = config["actions"].toArray();
        end_ = unsigned(config["end"].toInt());
        timer_ = config["timer"].toBool(true);
        require(end_ <= 256 && actions_.size() <= 256, "window script bounds exceeded");
        int previous = -1;
        for (const auto &action : actions_) {
            int tick = action.toObject()["tick"].toInt(-1);
            require(tick >= previous && tick >= 0 && tick <= int(end_), "unordered window actions");
            previous = tick;
        }
        std::filesystem::create_directories(output_);
        orders_.open(output_ / "orders.txt");
        trace_.open(output_ / "trace.jsonl");
        c.onOrder = [this](const game::SkirmishOrder &o) {
            ++ordersCount_;
            const char *kinds[] = {"summon", "bolt", "move", "stop", "attack"};
            orders_ << controller_.session().world().state().tick << ' ' << kinds[unsigned(o.kind)]
                    << ' ' << o.subject.slot;
            if (o.target)
                orders_ << ' ' << o.target->slot;
            if (o.destination)
                orders_ << ' ' << o.destination->x << ' ' << o.destination->y;
            orders_ << '\n';
            orders_.flush();
        };
        c.afterTick = [this] {
            trace(trace_, controller_.session(), controller_.lastReport());
            const auto tick = controller_.session().world().state().tick;
            const bool hasAction = cursor_ < actions_.size() &&
                                   unsigned(actions_[cursor_].toObject()["tick"].toInt()) <= tick;
            if (!timer_ || hasAction || tick >= end_ ||
                controller_.session().state().outcome != game::Outcome::running) {
                controller_.pause();
                QTimer::singleShot(0, [this] { guarded([&] { process(); }); });
            }
        };
        watchdog_.setSingleShot(true);
        QObject::connect(&watchdog_, &QTimer::timeout, [] { QApplication::exit(2); });
        watchdog_.start(60000);
        QTimer::singleShot(0, [this] {
            guarded([&] {
                capture("initial");
                process();
            });
        });
    }
    ~Driver() {
        controller_.afterTick = {};
        controller_.onOrder = {};
    }
};
std::shared_ptr<Driver> start(const QString &s, const QString &o, Window &w, Controller &c) {
    return std::make_shared<Driver>(s, o, w, c);
}
} // namespace mnm::skirmish::test

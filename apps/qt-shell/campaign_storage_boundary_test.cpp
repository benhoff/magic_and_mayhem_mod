#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "../../protocols/include/mnm/menu_v13.h"
#include <QAbstractButton>
#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTimer>
#include <QtEndian>
#include <cstdio>
#include <memory>

namespace {
enum class Step {
    Main, Region, World, Create, Creating, Created, Overwrite,
    Declining, Declined, Accepting, Overwritten, Delete,
    Deleting, Deleted, Cancelled, Load, LoadCancelled, Quitting, Report, Exit
};
struct Run {
    Step step = Step::Main;
    QString name, error, before, after;
    qint64 beforeTime = 0, afterTime = 0;
    quint32 thread = 0;
    int input = 0, attempts = 0;
    bool helper = false, failed = false, noPreserved = false, deleteNoPreserved = false;
    bool rowsRefreshed = false, loadExcluded = false;
    bool overwriteNoObserved = false, overwriteYesObserved = false;
    QJsonArray states;
};
QString saveHash(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly) || !f.size()) return {};
    return QString::fromLatin1(QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha256).toHex());
}
}

// Exercises only a unique save created by this run. Original dialogs receive
// physical input; the native delete dialog receives normal Qt button clicks.
void installCampaignStorageBoundaryTest(QApplication& app, QMainWindow& window,
                                       LiveMenuSession& session, const QString& path) {
    auto r = std::make_shared<Run>();
    r->name = qEnvironmentVariable("MNM_CAMPAIGN_STORAGE_NAME");
    auto savePath = [&session, r] {
        return QDir(session.evidenceDirectory()).filePath("game/Save/" + r->name + ".sav");
    };
    auto write = [r, path](bool success) {
        QFile f(path);
        const QJsonObject result{{"success", success}, {"stage", int(r->step)}, {"error", r->error},
            {"states", r->states}, {"saved_name", r->name}, {"before_sha256", r->before},
            {"after_sha256", r->after}, {"before_mtime_ms", r->beforeTime},
            {"after_mtime_ms", r->afterTime}, {"overwrite_no_preserved", r->noPreserved},
            {"delete_no_preserved", r->deleteNoPreserved}, {"delete_rows_refreshed", r->rowsRefreshed},
            {"load_excludes_deleted", r->loadExcluded},
            {"scope", "Unique original save: overwrite No/Yes, native delete No/Yes, refreshed original rows, Save/Load Cancel, normal Quit on Wine-window embedding"}};
        return f.open(QIODevice::WriteOnly) && f.write(QJsonDocument(result).toJson()) > 0 && f.flush();
    };
    auto fail = [r, write, &app](const QString& why) {
        if (r->failed) return;
        r->failed = true; r->error = why;
        std::fprintf(stderr, "Storage boundaries failed: %s\n", qPrintable(why));
        write(false); app.exit(1);
    };
    auto click = [&window, fail](const QString& name) {
        auto* b = window.findChild<QPushButton*>(name);
        if (!b || !b->isVisible() || !b->isEnabled()) { fail("Unavailable control: " + name); return; }
        b->click();
    };
    auto input = [r, fail, &window, path](const QString& mode) {
        auto* v = window.findChild<QWidget*>("legacyGameContainer");
        if (!v || !v->isVisible()) { fail("Embedding viewport unavailable"); return; }
        window.activateWindow(); window.raise(); v->setFocus();
        const auto origin = v->mapToGlobal(QPoint((v->width()-800)/2, (v->height()-600)/2));
        auto* p = new QProcess(&window); r->helper = true;
        QObject::connect(p, qOverload<int,QProcess::ExitStatus>(&QProcess::finished), &window,
            [r, p, fail](int code, QProcess::ExitStatus status) {
                r->helper = false;
                if (code || status != QProcess::NormalExit) fail("Input failed: " + QString::fromLocal8Bit(p->readAllStandardError()));
                p->deleteLater();
            });
        p->start("python3", {QDir::current().filePath("tools/menu-storage-input.py"), "--mode", mode,
            "--rect", QString::number(origin.x()), QString::number(origin.y()), "800", "600",
            "--output", QFileInfo(path).dir().filePath(QString("boundary-input-%1.json").arg(r->input++))});
    };
    auto old = session.stateChanged;
    session.stateChanged = [r, old, fail, click, savePath, &window, path](const MenuBridge::State& s) {
        if (old) old(s);
        if (!s.ready || r->failed) return;
        if (r->thread && r->thread != s.thread) { fail("Original engine thread changed"); return; }
        r->thread = s.thread;
        QString action;
        auto select = [&] {
            auto* list = window.findChild<QListWidget*>("saveGameList");
            const int row = s.storage.names.indexOf(r->name);
            if (!list || row < 0) { fail("Original Save list lacks this run's save"); return false; }
            list->setCurrentRow(row); return true;
        };
        if (r->step == Step::Main && s.screen == 3) { r->step = Step::Region; action = "mainMenuAction0"; }
        else if (r->step == Step::Region && s.screen == 18) { r->step = Step::World; action = "regionEntryEnter"; }
        else if (r->step == Step::World && s.screen == 17) { r->step = Step::Create; action = "miniMenuButton2"; }
        else if (r->step == Step::Create && s.screen == 13) {
            if (QFileInfo::exists(savePath()) || s.storage.names.contains(r->name)) { fail("Test save already exists"); return; }
            auto* edit = window.findChild<QLineEdit*>("saveGameFileName");
            if (!edit) { fail("Save edit missing"); return; }
            edit->setText(r->name); r->step = Step::Creating; action = "saveGameSave";
        }
        else if (r->step == Step::Created && s.screen == 17) { r->step = Step::Overwrite; action = "miniMenuButton2"; }
        else if (r->step == Step::Overwrite && s.screen == 13) {
            if (!select()) return;
            r->before = saveHash(savePath()); r->beforeTime = QFileInfo(savePath()).lastModified().toMSecsSinceEpoch();
            if (r->before.isEmpty()) { fail("Original new save missing"); return; }
            r->step = Step::Declining; action = "saveGameSave";
        }
        else if (r->step == Step::Declining && r->overwriteNoObserved && s.screen == 13 && !s.storage.modal && !s.storage.pending) {
            r->noPreserved = saveHash(savePath()) == r->before && QFileInfo(savePath()).lastModified().toMSecsSinceEpoch() == r->beforeTime;
            if (!r->noPreserved) { fail("Overwrite No changed the save"); return; }
            // Record a ready return before asking the original writer again.
            r->step = Step::Declined;
            QTimer::singleShot(800, &window, [r, click] { r->step = Step::Accepting; click("saveGameSave"); });
        }
        else if (r->step == Step::Overwritten && s.screen == 17) { r->step = Step::Delete; action = "miniMenuButton2"; }
        else if (r->step == Step::Delete && s.screen == 13) {
            if (!select()) return;
            r->step = Step::Deleting;
            QTimer::singleShot(0, &window, [r, click, fail, savePath, &window] {
                click("saveGameDelete");
                auto* d = window.findChild<QMessageBox*>("saveGameDeleteConfirmation");
                if (!d || !d->isVisible()) { fail("Delete No dialog unavailable"); return; }
                d->button(QMessageBox::No)->click();
                r->deleteNoPreserved = saveHash(savePath()) == r->after;
                if (!r->deleteNoPreserved) { fail("Delete No changed save bytes"); return; }
                QTimer::singleShot(0, &window, [click, fail, &window] {
                    click("saveGameDelete");
                    for (auto* dialog : window.findChildren<QMessageBox*>("saveGameDeleteConfirmation")) {
                        if (dialog->isVisible()) { dialog->button(QMessageBox::Yes)->click(); return; }
                    }
                    fail("Delete Yes dialog unavailable");
                });
            });
        }
        else if (r->step == Step::Deleting && s.screen == 13 && !s.storage.names.contains(r->name)) {
            if (QFileInfo::exists(savePath())) { fail("Original row deleted without deleting file"); return; }
            auto* list = window.findChild<QListWidget*>("saveGameList");
            if (!list || list->count() != s.storage.names.size()) { fail("Native delete list stale"); return; }
            for (int i=0; i<list->count(); ++i) if (list->item(i)->text() != s.storage.names[i]) { fail("Native row differs from original"); return; }
            r->rowsRefreshed = true; r->step = Step::Deleted; action = "saveGameCancel";
        }
        else if (r->step == Step::Cancelled && s.screen == 17) { r->step = Step::Load; action = "miniMenuButton1"; }
        else if (r->step == Step::Load && s.screen == 12) {
            if (s.storage.names.contains(r->name) || QFileInfo::exists(savePath())) { fail("Deleted save reappeared in Load"); return; }
            r->loadExcluded = true; r->step = Step::LoadCancelled; action = "loadGameCancel";
        }
        else if (r->step == Step::Quitting && s.screen == 17) { r->step = Step::Report; action = "miniMenuButton4"; }
        else if (r->step == Step::Report && s.screen == 6) { r->step = Step::Exit; action = "battleResultContinue"; }
        else if (r->step == Step::Exit && s.screen == 3) { action = "mainMenuAction4"; }
        if (action.isEmpty()) return;
        r->attempts = 0;
        r->states.append(QJsonObject{{"screen", int(s.screen)}, {"ack", int(s.ack)},
            {"thread", int(s.thread)}, {"generation", int(s.generation)}, {"stage", int(r->step)}});
        QTimer::singleShot(0, &window, [r, click, action, &window, path] {
            window.grab().save(QFileInfo(path).dir().filePath(QString("storage-boundary-%1.png").arg(int(r->step))));
            click(action);
        });
    };
    auto viewport = session.originalViewportRequested;
    session.originalViewportRequested = [r, viewport, input, &window] {
        if (viewport) viewport();
        // Save acknowledgement precedes deferred modal creation; wait for the
        // modal snapshot rather than clicking an unconfirmed screen.
        if (r->step == Step::Report) QTimer::singleShot(800, &window, [input] { input("yes"); });
    };
    auto* timer = new QTimer(&window); timer->setInterval(1500);
    QObject::connect(timer, &QTimer::timeout, &window, [r, fail, input, savePath, &session, &window, path] {
        if (r->failed || r->helper) return;
        QFile events(QDir(session.evidenceDirectory()).filePath("events.bin"));
        bool world = false, saveReturned = false;
        if (events.open(QIODevice::ReadOnly)) {
            const auto bytes = events.readAll();
            for (int at=16; at+64<=bytes.size(); at+=64) {
                auto word = [&](int i) { return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(bytes.constData()+at+4*i)); };
                if ((word(1)==12 || word(1)==20) && word(3)==2 && word(6)==1) world = true;
                if (word(1)==4 && word(3)==13 && word(8)==1) saveReturned = true;
            }
        }
        QFile channel(QDir(session.evidenceDirectory()).filePath("channel.bin"));
        if (!channel.open(QIODevice::ReadOnly)) return;
        const auto wire = channel.readAll();
        if (wire.size() != MNM_MENU_V13_SIZE) return;
        auto word = [&](int offset) { return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(wire.constData()+offset)); };
        const auto sequence = word(128);
        channel.seek(128); const auto last = channel.read(4);
        if ((sequence & 1) || last.size()!=4 || sequence!=qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(last.constData()))) return;
        const auto screen = word(136), modal = word(MNM_MENU_V13_STORAGE+16);
        if ((r->step == Step::Declining || r->step == Step::Accepting) && screen == 13 && modal) {
            bool& observed = r->step == Step::Declining ? r->overwriteNoObserved : r->overwriteYesObserved;
            if (observed) return;
            observed = true;
            window.grab().save(QFileInfo(path).dir().filePath(QString("overwrite-%1.png").arg(int(r->step))));
            input(r->step == Step::Declining ? "no" : "yes");
        }
        if (r->step == Step::Creating && world && saveReturned && screen != 13 && !saveHash(savePath()).isEmpty()) r->step = Step::Created;
        if (r->step == Step::Accepting && r->overwriteYesObserved && world && saveReturned && !modal && screen != 13) {
            r->after = saveHash(savePath()); r->afterTime = QFileInfo(savePath()).lastModified().toMSecsSinceEpoch();
            if (r->after.isEmpty() || (r->after == r->before && r->afterTime <= r->beforeTime)) { fail("Overwrite Yes produced no observed replacement write"); return; }
            r->step = Step::Overwritten;
        }
        if (r->step == Step::Deleted && world && screen != 13) r->step = Step::Cancelled;
        if (r->step == Step::LoadCancelled && world && screen != 12) r->step = Step::Quitting;
        if (world && (r->step == Step::World || r->step == Step::Created || r->step == Step::Overwritten || r->step == Step::Cancelled || r->step == Step::Quitting)) {
            if (++r->attempts > 8) { fail("Mini ingress timed out"); return; }
            input("mini");
        }
    }); timer->start();
    auto failed = session.failed;
    session.failed = [failed, fail](const QString& s) { if (failed) failed(s); fail(s); };
    auto output = session.output;
    session.output = [output](const QString& s) { if (output) output(s); std::fprintf(stderr, "%s", qPrintable(s)); };
    auto finished = session.finished;
    session.finished = [r, finished, write, &app] {
        if (finished) finished();
        const bool success = !r->failed && r->step == Step::Exit && r->noPreserved && r->deleteNoPreserved && r->rowsRefreshed && r->loadExcluded;
        app.exit(success && write(success) ? 0 : 1);
    };
    session.winePrefix = QFileInfo(path).dir().filePath("wineprefix"); session.smokeSeconds = 0;
    QTimer::singleShot(240000, &window, [fail] { fail("Storage boundary deadline expired"); });
    QTimer::singleShot(0, &window, [click] { click("launchGame"); });
}

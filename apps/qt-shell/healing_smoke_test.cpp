#include "campaign_smoke_test.hpp"
#include "frame_stream.hpp"
#include "gl_viewport.hpp"
#include "live_menu_session.hpp"
#include "spellbox_widget.hpp"
#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QMainWindow>
#include <QProcess>
#include <QSaveFile>
#include <QSet>
#include <QSlider>
#include <QTimer>
#include <QtEndian>
#include <cstdio>
#include <future>
#include <memory>

void installNativeHealingSmokeTest(
    QApplication &app, QMainWindow &window, LiveMenuSession &session,
    const QString &path, std::function<CampaignPresentationProbe()> probe) {
  const bool crowded = qEnvironmentVariableIntValue("MNM_NATIVE_CROWDED") == 1;
  struct Run {
    int stage = 0;
    quint32 thread = 0, owner = 0;
    bool done = false, started = false;
    quint64 frames = 0, paints = 0;
    QElapsedTimer clock;
    QJsonArray steps, rates;
    std::future<void> capture;
  };
  auto run = std::make_shared<Run>();
  auto finish = [run, path, probe, crowded, &app](bool success, const QString &error) {
    if (run->done)
      return;
    run->done = true;
    const auto p = probe();
    QSaveFile f(path);
    QJsonObject r{
        {"success", success},
        {"error", error},
        {"healing_mode", !crowded},
        {"crowded_mode", crowded},
        {"steps", run->steps},
        {"gameplay_rates", run->rates},
        {"native_command_frames", qint64(p.frames)},
        {"native_painted_frames", qint64(p.paints)},
        {"native_command_fallback", p.fallback},
        {"native_recoveries", int(p.recoveries)},
        {"native_error", p.error},
        {"renderer", p.renderer},
        {"owner", int(run->owner)},
        {"scope",
         "Native Quick Battle setup/map/Cure loadout, optional explicit crowded "
         "setup, and original health "
         "recovery through native command presentation; original "
         "simulation/drawing retained, bounded private cleanup"}};
    const bool saved = f.open(QIODevice::WriteOnly) &&
                       f.write(QJsonDocument(r).toJson()) > 0 && f.commit();
    std::fprintf(stderr, "Native healing smoke %s: %s\n",
                 success && saved ? "complete" : "failed", qPrintable(error));
    if (!saved)
      app.exit(1);
  };
  auto click = [&window, finish](const QString &name) {
    auto *b = window.findChild<QAbstractButton *>(name);
    if (!b || !b->isVisible() || !b->isEnabled()) {
      finish(false, "Native healing control unavailable: " + name);
      return;
    }
    b->click();
  };
  auto snapshot = [run, &window, path](const QString &step) {
    const auto image = QFileInfo(path).dir().filePath(step + ".png");
    run->steps.append(
        QJsonObject{{"step", step},
                    {"screenshot", image},
                    {"screenshot_saved", window.grab().save(image)}});
  };
  auto output = session.output;
  session.output = [output](const QString &s) {
    if (output)
      output(s);
    std::fprintf(stderr, "%s", qPrintable(s));
  };
  auto failed = session.failed;
  session.failed = [failed, finish](const QString &e) {
    if (failed)
      failed(e);
    finish(false, e);
  };
  auto state = session.stateChanged;
  session.stateChanged = [run, state, click, snapshot, finish, crowded,
                          &window](const MenuBridge::State &s) {
    if (state)
      state(s);
    if (run->done || !s.ready)
      return;
    if (run->thread && s.thread != run->thread) {
      finish(false, "Healing menu engine thread changed");
      return;
    }
    run->thread = s.thread;
    const quint32 screens[] = {3, 22, 14, 25, 14, 7};
    if (run->stage == 6)
      return;
    if (run->stage >= 7) {
      finish(false, "Healing game left World unexpectedly");
      return;
    }
    if (s.screen != screens[run->stage] || s.ack != quint32(run->stage))
      return;
    const int stage = run->stage++;
    QTimer::singleShot(
        0, &window, [run, stage, click, snapshot, finish, crowded, &window, s] {
          const QString names[] = {"main", "quick",       "setup",
                                   "map",  "setup-ready", "spells"};
          snapshot(names[stage]);
          if (stage == 0)
            click("mainMenuAction2");
          if (stage == 1)
            click("quickBattleAction2");
          if (stage == 2) {
            auto *slider = window.findChild<QSlider *>("singlePlayerSlider3");
            if (!slider) {
              finish(false, "Native magic-items control unavailable");
              return;
            }
            slider->setValue(slider->maximum());
            if (crowded) {
              for (int index : {1, 2, 13}) {
                auto *rule = window.findChild<QSlider *>(
                    "singlePlayerSlider" + QString::number(index));
                if (!rule) {
                  finish(false, "Native crowded setup control unavailable");
                  return;
                }
                rule->setValue(rule->maximum());
              }
            }
            click("singlePlayerMap");
          }
          if (stage == 3) {
            auto *list = window.findChild<QListWidget *>("mapSelectionList");
            if (!list || list->count() < 2) {
              finish(false, "Native second map unavailable");
              return;
            }
            list->setCurrentRow(1);
            click("mapSelectionOk");
          }
          if (stage == 4) {
            if (s.battle.map != 2) {
              finish(false, "Original setup did not select map2");
              return;
            }
            if (crowded) {
              if (s.battle.rules[0] != 200 || s.battle.rules[1] != 800 ||
                  s.battle.rules[12] != 30) {
                finish(false, "Original crowded setup rules did not match");
                return;
              }
              run->steps.append(QJsonObject{{"step", "crowded-setup"},
                  {"mana", s.battle.rules[0]}, {"health", s.battle.rules[1]},
                  {"control_limit", s.battle.rules[12]}});
            }
            click("singlePlayerStart");
          }
          if (stage == 5) {
            auto *box =
                window.findChild<SpellboxWidget *>("liveSpellSelection");
            if (!box) {
              finish(false, "Native spell loadout unavailable");
              return;
            }
            const auto inventory = box->inventory();
            QString cure, zombie, law, chaos;
            for (const auto &i : inventory.items) {
              if (i.spells[0].id == "41")
                cure = i.id;
              if (i.spells[2].id == "14")
                zombie = i.id;
            }
            for (const auto &t : inventory.talismans) {
              if (law.isEmpty() &&
                  t.alignment == SpellboxWidget::Alignment::Law)
                law = t.id;
              if (chaos.isEmpty() &&
                  t.alignment == SpellboxWidget::Alignment::Chaos)
                chaos = t.id;
              if (!box->removeItem(t.id)) {
                finish(false, "Native loadout removal failed");
                return;
              }
            }
            if (cure.isEmpty() || zombie.isEmpty() || law.isEmpty() ||
                chaos.isEmpty() || !box->assignItem(cure, law) ||
                !box->assignItem(zombie, chaos)) {
              finish(
                  false,
                  "Original offered inventory cannot supply Cure and Zombie");
              return;
            }
            run->owner = s.spells.owner;
            QJsonArray assignments;
            for (const auto &a : box->draftRequest().assignments)
              assignments.append(QJsonObject{{"slot", a.talismanId},
                                             {"item", a.itemId},
                                             {"spell", a.spellId}});
            run->steps.append(QJsonObject{{"step", "healing-loadout"},
                                          {"owner", int(run->owner)},
                                          {"assignments", assignments}});
            click("spellboxOK");
          }
        });
  };
  auto battle = session.battleStarted;
  session.battleStarted = [run, battle, finish](quint32 dest) {
    if (battle)
      battle(dest);
    if (dest != 2 || run->stage != 6) {
      finish(false, "Unexpected healing handoff");
      return;
    }
    run->stage = 7;
  };
  auto *poll = new QTimer(&window);
  poll->setInterval(100);
  QObject::connect(
      poll, &QTimer::timeout, &window,
      [run, path, probe, finish, crowded, &window, &session] {
        if (run->done)
          return;
        const auto p = probe();
        if (p.fallback) {
          finish(false, "Healing presentation fell back");
          return;
        }
        const auto dir = QFileInfo(path).dir();
        const auto request = dir.filePath("capture-request.json");
        if (p.active && QFileInfo::exists(request) &&
            (!run->capture.valid() ||
             run->capture.wait_for(std::chrono::seconds(0)) ==
                 std::future_status::ready)) {
          QFile f(request);
          if (f.open(QIODevice::ReadOnly)) {
            const auto name = QJsonDocument::fromJson(f.readAll())
                                  .object()
                                  .value("name")
                                  .toString();
            f.close();
            if (name.isEmpty() || name.size() > 79 || name.contains('/') ||
                name.contains('\\') || name.contains("..")) {
              finish(false, "Invalid healing capture name");
              return;
            }
            FrameStream ref;
            QImage original;
            if (ref.open(QDir(session.evidenceDirectory())
                             .filePath("render-frame.bin")))
              original = ref.nextFrame();
            if (!original.isNull()) {
              auto *v = dynamic_cast<GlViewport *>(
                  window.findChild<QOpenGLWidget *>("nativeGameViewport"));
              QImage native;
              if (v && v->isVisible()) {
                const auto r = v->imageRect();
                const auto d = v->devicePixelRatioF();
                native = v->grabFramebuffer()
                             .copy(QRect(qRound(r.x() * d), qRound(r.y() * d),
                                         qRound(r.width() * d),
                                         qRound(r.height() * d)))
                             .scaled(v->frameSize(), Qt::IgnoreAspectRatio,
                                     Qt::FastTransformation);
              }
              run->capture = std::async(std::launch::async, [dir, name, native,
                                                             original, p] {
                const bool ns = !native.isNull() &&
                                native.save(dir.filePath(name + "-native.png")),
                           os = original.save(
                               dir.filePath(name + "-original.png"));
                QSaveFile f(dir.filePath(name + ".json"));
                if (f.open(QIODevice::WriteOnly)) {
                  f.write(QJsonDocument(
                              QJsonObject{{"native_saved", ns},
                                          {"original_saved", os},
                                          {"fallback", p.fallback},
                                          {"native_frames", qint64(p.frames)}})
                              .toJson());
                  f.commit();
                }
              });
              QFile::remove(request);
            }
          }
        }
        if (run->stage != 7 || !p.active || p.frames < 3)
          return;
        if (run->started) {
          const auto ms = run->clock.elapsed();
          if (ms >= 1000) {
            run->rates.append(QJsonObject{
                {"cycle", 0},
                {"seconds", ms / 1000.0},
                {"native_frames", qint64(p.frames - run->frames)},
                {"painted_frames", qint64(p.paints - run->paints)},
                {"native_fps", 1000.0 * (p.frames - run->frames) / ms},
                {"paint_fps", 1000.0 * (p.paints - run->paints) / ms}});
            run->frames = p.frames;
            run->paints = p.paints;
            run->clock.restart();
          }
          return;
        }
        // GP snapshots are copied after original World returns. Require at
        // least three distinct observation ticks before injecting input.
        QFile trace(
            QDir(session.evidenceDirectory()).filePath("gameplay-events.bin"));
        if (!trace.open(QIODevice::ReadOnly))
          return;
        const auto bytes = trace.readAll();
        if (bytes.size() < 16 || bytes.left(8) != "MNMGP001" ||
            qFromLittleEndian<quint32>((const uchar *)bytes.constData() + 8) !=
                1 ||
            qFromLittleEndian<quint32>((const uchar *)bytes.constData() + 12) !=
                64)
          return;
        QSet<quint32> ticks;
        for (int at = 16; at + 64 <= bytes.size(); at += 64)
          if (qFromLittleEndian<quint32>((const uchar *)bytes.constData() + at +
                                         4) == 1)
            ticks.insert(qFromLittleEndian<quint32>(
                (const uchar *)bytes.constData() + at + 12));
        if (ticks.size() < 3)
          return;
        auto *v = dynamic_cast<GlViewport *>(
            window.findChild<QOpenGLWidget *>("nativeGameViewport"));
        if (!v || !v->isVisible()) {
          finish(false, "Native healing viewport unavailable");
          return;
        }
        const auto rect = v->imageRect();
        const auto origin = v->mapToGlobal(rect.topLeft().toPoint());
        window.activateWindow();
        window.raise();
        v->setFocus();
        run->started = true;
        run->frames = p.frames;
        run->paints = p.paints;
        run->clock.start();
        auto *input = new QProcess(&window);
        QObject::connect(input, &QProcess::errorOccurred, &window,
                         [finish](QProcess::ProcessError e) {
                           if (e == QProcess::FailedToStart)
                             finish(false, "Healing input could not start");
                         });
        QObject::connect(
            input, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            &window, [input, finish](int code, QProcess::ExitStatus status) {
              finish(!code && status == QProcess::NormalExit,
                     code || status != QProcess::NormalExit
                         ? "Healing input failed: " +
                               QString::fromLocal8Bit(
                                   input->readAllStandardError())
                         : QString());
              input->deleteLater();
            });
        QStringList arguments{
             QDir::current().filePath("tools/campaign-healing-input.py"),
             "--rect", QString::number(origin.x()), QString::number(origin.y()),
             QString::number(qRound(rect.width())),
             QString::number(qRound(rect.height())), "--experiment",
             session.evidenceDirectory(), "--owner",
             QString::number(run->owner), "--seconds",
             QString::number(
                 qEnvironmentVariableIntValue("MNM_CAMPAIGN_STRESS_SECONDS")),
             "--output", dir.filePath("healing-input.json")};
        if (crowded)
          arguments.append("--crowded");
        input->start("python3", arguments);
      });
  poll->start();
  auto ended = session.finished;
  session.finished = [ended, finish] {
    if (ended)
      ended();
    finish(false, "Original game exited before healing evidence");
  };
  session.winePrefix = QFileInfo(path).dir().filePath("wineprefix");
  session.preferencesStorePath =
      QFileInfo(path).dir().filePath("config/engine-preferences.json");
  QTimer::singleShot(
      qEnvironmentVariableIntValue("MNM_CAMPAIGN_TIMEOUT_MS"), &window,
      [finish] { finish(false, "Native healing deadline exceeded"); });
  QTimer::singleShot(0, &window, [click] { click("launchGame"); });
}

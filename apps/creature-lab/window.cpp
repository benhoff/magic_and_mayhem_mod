#include "window.hpp"
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
namespace mnm::creature_lab {
namespace {
const char *phase(game::CreaturePhase p) {
  switch (p) {
  case game::CreaturePhase::idle:
    return "Idle";
  case game::CreaturePhase::pursuing:
    return "Pursuing";
  case game::CreaturePhase::attacking:
    return "Attacking";
  case game::CreaturePhase::ready:
    return "Recovering";
  case game::CreaturePhase::stunned:
    return "Stunned";
  case game::CreaturePhase::dead:
    return "Defeated";
  case game::CreaturePhase::blocked:
    return "Blocked";
  case game::CreaturePhase::knockback:
    return "Knockback";
  }
  return "Invalid";
}
class Canvas final : public QWidget {
  Controller &c_;

public:
  explicit Canvas(Controller &c) : c_(c) {
    setMinimumSize(560, 560);
    setObjectName("creatureCanvas");
  }
  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#15252b"));
    auto s = c_.session().state();
    const auto &w = c_.session().world().state();
    int size = std::min(width(), height()) / 16;
    for (int y = 0; y < 16; ++y)
      for (int x = 0; x < 16; ++x) {
        QRect tile(x * size, y * size, size, size);
        p.fillRect(tile,
                   s.walls[y * 16 + x] ? QColor("#647477") : QColor("#1d333b"));
        p.setPen(QColor("#29444c"));
        p.drawRect(tile);
      }
    for (unsigned i = 0; i < s.actors.size(); ++i) {
      const auto &a = s.actors[i];
      const auto &e = *w.slots[i].entity;
      QColor color = a.health ? (i % 2 ? QColor("#db9171") : QColor("#8ebc83"))
                              : QColor("#657078");
      if (a.phase == game::CreaturePhase::stunned)
        color = QColor("#e0c462");
      for (int dy : {0, -16})
        for (int dx : {0, -16}) {
          int x = (e.x + dx) * size, y = (e.y + dy) * size;
          p.setBrush(color);
          p.setPen(Qt::NoPen);
          p.drawRoundedRect(QRect(x + 2, y + 2, size * 2 - 4, size * 2 - 4), 10,
                            10);
          p.setPen(QColor("#102328"));
          p.drawText(QRect(x, y, size * 2, size), Qt::AlignCenter,
                     QString("%1 · %2 HP").arg(i + 1).arg(a.health));
          p.drawText(QRect(x, y + size, size * 2, size), Qt::AlignCenter,
                     phase(a.phase));
        }
    }
  }
};
} // namespace
Window::Window(Controller &c) {
  setWindowTitle("Magic & Mayhem · Native creature lab");
  resize(750, 750);
  auto layout = new QVBoxLayout(this);
  auto title = new QLabel("Autonomous creature encounter");
  title->setStyleSheet("font-size:24px;font-weight:600");
  layout->addWidget(title);
  layout->addWidget(new QLabel(
      "Four Crocodiles · target selection, pursuit, melee reach and stun"));
  auto canvas = new Canvas(c);
  layout->addWidget(canvas);
  auto status = new QLabel("Play or step through the encounter.");
  status->setObjectName("creatureStatus");
  layout->addWidget(status);
  auto row = new QHBoxLayout;
  layout->addLayout(row);
  auto button = [&](const char *name, std::function<void()> f) {
    auto b = new QPushButton(name);
    b->setObjectName(QString("creature") + name);
    row->addWidget(b);
    QObject::connect(b, &QPushButton::clicked, this, [this, f] {
      try {
        f();
      } catch (const std::exception &e) {
        QMessageBox::warning(this, "Creature lab", e.what());
      }
    });
  };
  button("Play", [&] { c.play(); });
  button("Pause", [&] { c.pause(); });
  button("Step", [&] { c.step(); });
  button("Restart", [&] { c.restart(); });
  button("Save", [this, &c] {
    c.pause();
    auto p = QFileDialog::getSaveFileName(this, "Save creature encounter", {},
                                          "Native encounter (*.mnm)");
    if (!p.isEmpty())
      c.save(p.toStdString(), true);
  });
  button("Load", [this, &c] {
    c.pause();
    auto p = QFileDialog::getOpenFileName(this, "Load creature encounter", {},
                                          "Native encounter (*.mnm)");
    if (!p.isEmpty())
      c.load(p.toStdString());
  });
  c.changed = [&c, canvas, status] {
    canvas->update();
    QString text = QString("Tick %1 · %2")
                       .arg(c.session().world().state().tick)
                       .arg(c.playing() ? "Playing" : "Paused");
    if (c.session().finished())
      text += " · Encounter finished";
    else if (!c.last().empty()) {
      const auto &e = c.last().back();
      text += QString(" · Creature %1: %2 (%3)")
                  .arg(e.actor + 1)
                  .arg(QString::fromStdString(e.kind))
                  .arg(e.amount);
    }
    status->setText(text);
  };
  c.error = [this](const std::string &e) {
    QMessageBox::warning(this, "Creature lab", QString::fromStdString(e));
  };
}
} // namespace mnm::creature_lab

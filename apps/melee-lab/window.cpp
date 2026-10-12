#include "window.hpp"
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
namespace mnm::melee_lab {
namespace {
class Canvas final : public QWidget {
  Controller &controller_;

public:
  explicit Canvas(Controller &c) : controller_(c) {
    setMinimumSize(700, 360);
    setObjectName("meleeCanvas");
  }
  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#15252b"));
    const auto s = controller_.session().state();
    for (unsigned i = 0; i < 2; ++i) {
      const int x = width() / 2 + (i ? 140 : -140), y = height() / 2;
      QColor color = i ? QColor("#db9171") : QColor("#8ebc83");
      if (!s.actors[i].health)
        color = QColor("#657078");
      p.setBrush(color);
      p.setPen(Qt::NoPen);
      p.drawRoundedRect(QRect(x - 70, y - 22, 125, 44), 12, 12);
      QPolygon tail;
      tail << QPoint(x - 70, y - 17) << QPoint(x - 112, y)
           << QPoint(x - 70, y + 17);
      p.drawPolygon(tail);
      p.drawEllipse(QRect(x + 35, y - 15, 60, 30));
      p.setBrush(QColor("#101b1e"));
      p.drawEllipse(QPoint(x + 67, y - 7), 4, 4);
      p.setPen(QColor("#e7eeeb"));
      p.drawText(QRect(x - 120, y - 95, 240, 30), Qt::AlignCenter,
                 i ? "Rival Crocodile" : "Your Crocodile");
      p.drawText(QRect(x - 120, y + 65, 240, 30), Qt::AlignCenter,
                 QString("%1 HP · %2")
                     .arg(s.actors[i].health)
                     .arg(s.actors[i].phase == game::ExchangePhase::dead
                              ? "Defeated"
                          : s.actors[i].phase == game::ExchangePhase::recovering
                              ? "Recovering"
                              : "Attacking"));
      const auto maximum = controller_.session().maximumHealth();
      p.fillRect(QRect(x - 100, y + 43, 200, 8), QColor("#37474e"));
      p.fillRect(
          QRect(x - 100, y + 43, int(200ULL * s.actors[i].health / maximum), 8),
          color);
    }
    p.setPen(QColor("#d2ddd8"));
    p.drawText(QRect(0, 15, width(), 30), Qt::AlignCenter,
               QString("Tick %1 · %2")
                   .arg(controller_.session().world().state().tick)
                   .arg(controller_.playing() ? "Playing" : "Paused"));
  }
};
} // namespace
Window::Window(Controller &c) {
  setWindowTitle("Magic & Mayhem · Native melee lab");
  resize(820, 560);
  auto layout = new QVBoxLayout(this);
  auto title = new QLabel("Crocodile versus Crocodile");
  title->setStyleSheet("font-size:24px;font-weight:600");
  layout->addWidget(title);
  auto detail = new QLabel("Original attack animation and damage rules · "
                           "stationary native encounter");
  layout->addWidget(detail);
  auto canvas = new Canvas(c);
  layout->addWidget(canvas);
  auto status =
      new QLabel("Play to begin, or Step through one simulation tick.");
  status->setWordWrap(true);
  status->setObjectName("meleeStatus");
  layout->addWidget(status);
  auto row = new QHBoxLayout;
  layout->addLayout(row);
  const auto button = [&](const char *name,
                          const std::function<void()> &action) {
    auto b = new QPushButton(name);
    b->setObjectName(QString("melee") + name);
    row->addWidget(b);
    QObject::connect(b, &QPushButton::clicked, this, [&, action] {
      try {
        action();
      } catch (const std::exception &e) {
        QMessageBox::warning(this, "Melee lab", e.what());
      }
    });
  };
  button("Play", [&c] { c.play(); });
  button("Pause", [&c] { c.pause(); });
  button("Step", [&c] { c.step(); });
  button("Restart", [&c] { c.restart(); });
  button("Save", [this, &c] {
    c.pause();
    auto path = QFileDialog::getSaveFileName(this, "Save melee encounter", {},
                                             "Native encounter (*.mnm)");
    if (!path.isEmpty())
      c.save(path.toStdString(), true);
  });
  button("Load", [this, &c] {
    c.pause();
    auto path = QFileDialog::getOpenFileName(this, "Load melee encounter", {},
                                             "Native encounter (*.mnm)");
    if (!path.isEmpty())
      c.load(path.toStdString());
  });
  c.changed = [&c, canvas, status] {
    canvas->update();
    if (c.session().finished()) {
      auto s = c.session().state();
      status->setText(s.actors[0].health ? "Your Crocodile wins."
                                         : "Rival Crocodile wins.");
    } else if (!c.last().empty()) {
      const auto &e = c.last().back();
      status->setText(e.animation == 2
                          ? QString("Attack hit: %1 damage.").arg(e.applied)
                      : e.animation == 1 ? "Attack complete; recovering."
                                         : "Attack animation continues.");
    } else
      status->setText("Attack animation continues.");
  };
  c.error = [this](const std::string &e) {
    QMessageBox::warning(this, "Melee lab", QString::fromStdString(e));
  };
}
} // namespace mnm::melee_lab

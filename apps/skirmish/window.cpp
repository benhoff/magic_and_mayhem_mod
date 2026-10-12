#include "window.hpp"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>
#include <cmath>
namespace mnm::skirmish {
ArenaCanvas::ArenaCanvas(QWidget *parent) : QWidget(parent) {
    setObjectName("arenaCanvas");
    setFixedSize(560, 560);
}
QPoint ArenaCanvas::cellCenter(game::Point p) { return {24 + p.x * 64 + 32, 24 + p.y * 64 + 32}; }
void ArenaCanvas::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
        return;
    const auto point = e->position().toPoint();
    if (point.x() < 24 || point.y() < 24 || point.x() >= 536 || point.y() >= 536)
        return;
    if (onCell)
        onCell({(point.x() - 24) / 64, (point.y() - 24) / 64, 1}, e->button() == Qt::RightButton);
}
void ArenaCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#131c25"));
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            const bool inside = game::arenaCell({x, y, 1});
            const QRect tile(24 + x * 64, 24 + y * 64, 64, 64);
            p.fillRect(tile,
                       inside ? QColor((x + y) % 2 ? "#3d5146" : "#435a4b") : QColor("#27313a"));
            p.setPen(QColor("#273e37"));
            p.drawRect(tile.adjusted(1, 1, -1, -1));
        }
    for (const auto &u : view_.units) {
        const QPoint center = cellCenter(u.cell);
        const QColor color = u.owner ? QColor("#f09078") : QColor("#7bd9dd");
        if (!u.health) {
            p.setPen(QPen(QColor("#818a83"), 2));
            p.drawLine(center + QPoint(-7, -7), center + QPoint(7, 7));
            p.drawLine(center + QPoint(7, -7), center + QPoint(-7, 7));
            continue;
        }
        if (view_.selected == std::optional<game::Handle>(u.actor)) {
            p.setPen(QPen(QColor("#f6dd85"), 2));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(center, 26, 26);
        }
        p.setPen(QPen(color.lighter(130), 2));
        p.setBrush(color);
        if (u.role == game::UnitRole::wizard) {
            QPolygon hat;
            hat << center + QPoint(0, -21) << center + QPoint(-15, 5) << center + QPoint(15, 5);
            p.drawPolygon(hat);
            p.drawEllipse(center + QPoint(0, 10), 14, 8);
        } else {
            QPolygon shield;
            shield << center + QPoint(0, -16) << center + QPoint(15, -7) << center + QPoint(12, 13)
                   << center + QPoint(0, 20) << center + QPoint(-12, 13)
                   << center + QPoint(-15, -7);
            p.drawPolygon(shield);
            p.setPen(QPen(QColor("#233a3b"), 3));
            p.drawLine(center + QPoint(-7, 0), center + QPoint(7, 0));
        }
        if (u.action == "Attacking" || u.action.startsWith("Casting") || u.action == "Summoning") {
            p.setPen(QPen(QColor("#f5dd81"), 3));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(center, 21, 21);
        }
        p.fillRect(QRect(center.x() - 22, center.y() - 31, 44, 5), QColor("#20282f"));
        p.fillRect(QRect(center.x() - 22, center.y() - 31, int(44 * u.health / u.maximum), 5),
                   color);
    }
    for (const auto &projectile : view_.projectiles) {
        const auto a = cellCenter(projectile.origin), b = cellCenter(projectile.destination);
        const double t = 1.0 - double(projectile.remaining) / game::SkirmishRules::flightTicks;
        const QPointF position =
            a + (b - a) * t + QPointF(0, -18 * std::sin(t * 3.141592653589793));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 231, 127, 75));
        p.drawEllipse(position, 13, 13);
        p.setBrush(QColor("#ffe797"));
        p.drawEllipse(position, 6, 6);
    }
    if (view_.outcome != game::Outcome::running) {
        p.fillRect(QRect(70, 205, 420, 145), QColor(15, 23, 31, 235));
        p.setPen(QColor("#ffe6a1"));
        auto font = p.font();
        font.setPointSize(30);
        font.setBold(true);
        p.setFont(font);
        const QString title = view_.outcome == game::Outcome::victory  ? "Victory"
                              : view_.outcome == game::Outcome::defeat ? "Defeat"
                                                                       : "Draw";
        p.drawText(QRect(70, 220, 420, 60), Qt::AlignCenter, title);
        font.setPointSize(12);
        font.setBold(false);
        p.setFont(font);
        p.setPen(Qt::white);
        p.drawText(QRect(80, 290, 400, 40), Qt::AlignCenter,
                   "Start a new skirmish or load a checkpoint.");
    }
}
Window::Window(Controller &c) {
    setObjectName("skirmishWindow");
    setWindowTitle("Magic & Mayhem — Native Skirmish");
    setStyleSheet("QWidget { background: #131c25; color: #e9eeea; font-size: 14px; } QPushButton { "
                  "background: #2d414a; padding: 9px; border-radius: 5px; } QPushButton:disabled { "
                  "color: #74858b; background: #202b33; } QLabel { padding: 4px; }");
    auto *layout = new QHBoxLayout(this);
    auto *canvas = new ArenaCanvas;
    layout->addWidget(canvas);
    auto *side = new QVBoxLayout;
    layout->addLayout(side);
    auto *title = new QLabel("NATIVE SKIRMISH");
    side->addWidget(title);
    auto *player = new QLabel;
    player->setObjectName("playerStatus");
    side->addWidget(player);
    auto *enemy = new QLabel;
    enemy->setObjectName("enemyStatus");
    side->addWidget(enemy);
    auto *selection = new QLabel;
    selection->setWordWrap(true);
    selection->setObjectName("selectedStatus");
    side->addWidget(selection);
    auto button = [&](const char *label, const char *name) {
        auto *b = new QPushButton(label);
        b->setObjectName(name);
        side->addWidget(b);
        return b;
    };
    auto *summon = button("Summon Sentinel · 8 mana", "summonButton");
    auto *bolt = button("Arc Bolt · 4 mana", "boltButton");
    auto *stop = button("Stop creature", "stopButton");
    auto *play = button("Play", "playButton");
    auto *pause = button("Pause", "pauseButton");
    auto *step = button("Step", "stepButton");
    auto *save = button("Save skirmish…", "saveButton");
    auto *load = button("Load skirmish…", "loadButton");
    auto *restart = button("New skirmish", "restartButton");
    auto *help = new QLabel("Select your wizard to cast.\nSummon: click nearby empty ground.\nArc "
                            "Bolt: click an enemy.\n\nSelect a Sentinel, then right-click\nground "
                            "to move or an enemy to attack.\nSpace toggles Play/Pause.");
    help->setWordWrap(true);
    side->addWidget(help);
    auto *status = new QLabel;
    status->setWordWrap(true);
    status->setObjectName("feedbackStatus");
    side->addWidget(status);
    side->addStretch();
    auto refresh = [&c, canvas, player, enemy, selection, status, summon, bolt, stop, play, pause,
                    step] {
        auto v = c.view();
        canvas->present(v);
        player->setText(
            QString("Your wizard · %1 HP\nMana %2 / 20").arg(v.wizardHealth[0]).arg(v.mana[0]));
        enemy->setText(
            QString("Rival wizard · %1 HP\nMana %2 / 20").arg(v.wizardHealth[1]).arg(v.mana[1]));
        selection->setText(v.selection);
        status->setText(v.feedback);
        summon->setEnabled(v.canCast && !v.busy && v.mana[0] >= game::SkirmishRules::summonCost);
        bolt->setEnabled(v.canCast && !v.busy && v.mana[0] >= game::SkirmishRules::boltCost);
        stop->setEnabled(v.canOrder);
        const bool running = v.outcome == game::Outcome::running;
        play->setEnabled(running && !c.playing());
        pause->setEnabled(c.playing());
        step->setEnabled(running && !c.playing());
    };
    c.onChanged = refresh;
    auto guarded = [status](auto f) {
        return [status, f] {
            try {
                f();
            } catch (const std::exception &e) {
                status->setText(QString::fromStdString(e.what()));
            }
        };
    };
    canvas->onCell = [&c, status](game::Point p, bool right) {
        try {
            c.cell(p, right);
        } catch (const std::exception &e) {
            status->setText(QString::fromStdString(e.what()));
        }
    };
    QObject::connect(summon, &QPushButton::clicked, this,
                     guarded([&c] { c.setMode(CursorMode::summon); }));
    QObject::connect(bolt, &QPushButton::clicked, this,
                     guarded([&c] { c.setMode(CursorMode::bolt); }));
    QObject::connect(stop, &QPushButton::clicked, this, guarded([&c] { c.stop(); }));
    QObject::connect(play, &QPushButton::clicked, this, [&c] { c.play(); });
    QObject::connect(pause, &QPushButton::clicked, this, [&c] { c.pause(); });
    QObject::connect(step, &QPushButton::clicked, this, [&c] { c.step(); });
    QObject::connect(restart, &QPushButton::clicked, this, guarded([&c] { c.restart(); }));
    QObject::connect(save, &QPushButton::clicked, this, guarded([&c, this] {
                         c.pause();
                         const auto path = QFileDialog::getSaveFileName(
                             this, "Save native skirmish", {}, "Native skirmish (*.mnms)");
                         if (!path.isEmpty())
                             c.save(path.toStdString(), true);
                     }));
    QObject::connect(load, &QPushButton::clicked, this, guarded([&c, this] {
                         c.pause();
                         const auto path = QFileDialog::getOpenFileName(
                             this, "Load native skirmish", {}, "Native skirmish (*.mnms)");
                         if (!path.isEmpty())
                             c.load(path.toStdString());
                     }));
    auto *space = new QShortcut(QKeySequence(Qt::Key_Space), this);
    QObject::connect(space, &QShortcut::activated, this, [&c] {
        if (c.playing())
            c.pause();
        else
            c.play();
    });
    refresh();
}
} // namespace mnm::skirmish

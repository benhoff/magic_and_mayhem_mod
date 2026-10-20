#include "window.hpp"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
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
QPoint ArenaCanvas::presentedCenter(game::Point p) const {
    if(!view_.mission) return cellCenter(p);
    for(const auto &u:view_.units) if(u.cell==p) return view_.mission->unitCenter(p);
    return view_.mission->center(p);
}
QPoint ArenaCanvas::cellCenter(game::Point p) { return {24 + p.x * 64 + 32, 24 + p.y * 64 + 32}; }
void ArenaCanvas::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
        return;
    const auto point = e->position().toPoint();
    if(view_.mission) {
        std::vector<game::Point> actors;std::vector<unsigned> widths;
        for(const auto &u:view_.units){actors.push_back(u.cell);widths.push_back(u.health?u.width:1);}
        const auto cell=view_.mission->pick(point,actors,widths);
        if(cell && onCell) onCell(*cell,e->button()==Qt::RightButton);
        return;
    }
    if (point.x() < 24 || point.y() < 24 || point.x() >= 536 || point.y() >= 536)
        return;
    if (onCell)
        onCell({(point.x() - 24) / 64, (point.y() - 24) / 64, 1}, e->button() == Qt::RightButton);
}
void ArenaCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#131c25"));
    if(!view_.mission) for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            const bool inside = game::arenaCell({x, y, 1});
            const QRect tile(24 + x * 64, 24 + y * 64, 64, 64);
            p.fillRect(tile,
                       inside ? QColor((x + y) % 2 ? "#3d5146" : "#435a4b") : QColor("#27313a"));
            p.setPen(QColor("#273e37"));
            p.drawRect(tile.adjusted(1, 1, -1, -1));
        }
    const auto drawUnit=[&](const UnitView &u) {
        p.save();
        if(!view_.mission && u.health && u.width>1) {
            p.setPen(QPen(QColor(u.owner?"#f09078":"#7bd9dd"),1,Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawRect(QRect(24+u.cell.x*64+4,24+u.cell.y*64+4,int(u.width)*64-8,int(u.width)*64-8));
        }
        if(view_.mission && u.health && u.width>1) {
            p.setPen(QPen(QColor(u.owner?"#f09078":"#7bd9dd"),1,Qt::DashLine));p.setBrush(Qt::NoBrush);
            const auto x=u.cell.x*32,y=u.cell.y*32,z=u.cell.z*16,w=int(u.width)*32;
            QPolygon box;box<<view_.mission->fineCenter({x,y,z})<<view_.mission->fineCenter({x+w,y,z})<<view_.mission->fineCenter({x+w,y+w,z})<<view_.mission->fineCenter({x,y+w,z});p.drawPolygon(box);
        }
        p.translate(presentedCenter(u.cell));
        if(view_.mission) p.scale(0.9,0.9);
        const QPoint center{0,0};
        const QColor color = u.owner ? QColor("#f09078") : QColor("#7bd9dd");
        if (!u.health) {
            p.setPen(QPen(QColor("#818a83"), 2));
            p.drawLine(center + QPoint(-7, -7), center + QPoint(7, 7));
            p.drawLine(center + QPoint(7, -7), center + QPoint(-7, 7));
            p.restore();return;
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
        p.restore();
    };
    if(view_.mission) {
        std::vector<game::Point> actors;for(const auto &u:view_.units) actors.push_back(u.cell);
        for(const auto &stamp:view_.mission->terrain()) p.drawImage(stamp.topLeft,stamp.image);
        p.setPen(QPen(QColor(153,210,188,115),1));p.setBrush(QColor(17,40,33,22));
        for(const auto cell:view_.mission->standing()) {
            const auto at=view_.mission->center(cell);QPolygon diamond;
            diamond<<at+QPoint(0,-16)<<at+QPoint(32,0)<<at+QPoint(0,16)<<at+QPoint(-32,0);
            p.drawPolygon(diamond);
        }
        for(const auto &layer:view_.mission->layers(actors)) if(layer.actor) drawUnit(view_.units.at(layer.index));
        p.setPen(QColor("#a4bbb0"));p.drawText(QRect(12,510,536,40),Qt::AlignCenter,"PLAINS CROSSING · Installed terrain · Native rules");
    } else for(const auto &u:view_.units) drawUnit(u);
    for (const auto &cell : view_.cures) {
        p.setPen(QPen(QColor("#75e8aa"), 3));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(presentedCenter(cell), 25, 25);
    }
    for (const auto &fine : view_.spheres) {
        auto center=view_.mission?view_.mission->fineCenter(fine):cellCenter({fine.x/32,fine.y/32,fine.z/16});
        if(!view_.mission) center += QPoint((fine.x%32-16)*64/32,(fine.y%32-16)*64/32);
        p.setPen(QPen(QColor("#b6acff"),2));
        p.setBrush(QColor("#7e65e8"));
        p.drawEllipse(center,9,9);
    }
    for (const auto &projectile : view_.projectiles) {
        const auto a = presentedCenter(projectile.origin), b = presentedCenter(projectile.destination);
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
        p.drawText(QRect(80, 290, 400, 40), Qt::AlignCenter, view_.resultHelp);
    }
}
Window::Window(Controller &c) {
    setObjectName("skirmishWindow");
    setWindowTitle("Magic & Mayhem — Native Skirmish");
    setStyleSheet("QWidget { background: #131c25; color: #e9eeea; font-size: 14px; } "
                  "QPushButton { "
                  "background: #2d414a; padding: 9px; border-radius: 5px; } "
                  "QPushButton:disabled { "
                  "color: #74858b; background: #202b33; } QLabel { padding: 4px; }");
    auto *layout = new QHBoxLayout(this);
    auto *canvas = new ArenaCanvas;
    auto *sceneColumn=new QVBoxLayout;
    layout->addLayout(sceneColumn);
    auto *missionStatus=new QLabel;
    missionStatus->setObjectName("missionStatus");
    missionStatus->setWordWrap(true);
    missionStatus->setFixedWidth(560);
    missionStatus->hide();
    missionStatus->setTextFormat(Qt::PlainText);
    auto *missionScroll=new QScrollArea;
    missionScroll->setWidget(missionStatus);
    missionScroll->setWidgetResizable(true);
    missionScroll->setFixedWidth(580);
    missionScroll->setMaximumHeight(230);
    missionScroll->setFrameShape(QFrame::NoFrame);
    sceneColumn->addWidget(missionScroll);
    sceneColumn->addWidget(canvas,0,Qt::AlignTop);
    sceneColumn->addStretch();
    auto *sideWidget=new QWidget;
    sideWidget->setMinimumWidth(365);
    auto *side = new QVBoxLayout(sideWidget);
    auto *sideScroll=new QScrollArea;
    sideScroll->setObjectName("campaignControlsScroll");
    sideScroll->setWidget(sideWidget);
    sideScroll->setWidgetResizable(true);
    sideScroll->setMinimumWidth(395);
    layout->addWidget(sideScroll);
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
    auto button = [&](const char *label, const char *name, QHBoxLayout *row = nullptr) {
        auto *b = new QPushButton(label);
        b->setObjectName(name);
        if (row)
            row->addWidget(b);
        else
            side->addWidget(b);
        return b;
    };
    auto *summon = button("Summon Sentinel · 8 mana", "summonButton");
    auto *bolt = button("Arc Bolt · 4 mana", "boltButton");
    auto *sphere = button("Magic Sphere · 8 mana", "sphereButton");
    sphere->setToolTip("Launch a travelling sphere. It can hit creatures along its path.");
    auto *cure = button("Cure · 3 mana", "cureButton");
    cure->setToolTip("Heal a living friendly Sentinel by 8, up to its maximum health.");
    auto *stop = button("Stop creature", "stopButton");
    auto *playbackRow = new QHBoxLayout;
    side->addLayout(playbackRow);
    auto *play = button("Play", "playButton", playbackRow);
    auto *pause = button("Pause", "pauseButton", playbackRow);
    auto *step = button("Step", "stepButton", playbackRow);
    auto *filesRow = new QHBoxLayout;
    side->addLayout(filesRow);
    auto *save = button("Save skirmish…", "saveButton", filesRow);
    auto *load = button("Load skirmish…", "loadButton", filesRow);
    auto *newRow = new QHBoxLayout;
    side->addLayout(newRow);
    auto *restart = button("New skirmish", "restartButton", newRow);
    auto *campaignStatus = new QLabel;
    campaignStatus->setObjectName("campaignStatus");
    campaignStatus->setWordWrap(true);
    side->addWidget(campaignStatus);
    auto *newCampaign = button("New campaign", "newCampaignButton", newRow);
    auto *mapCampaign = button("New map campaign…", "newMapCampaignButton");
    auto *missionCampaign = button("New Plains mission…", "newMissionCampaignButton");
    auto *crossroadsCampaign = button("New Crossroads campaign…", "newCrossroadsCampaignButton");
    auto *missionChoices=new QListWidget;
    missionChoices->setObjectName("campaignMissionList");
    missionChoices->setAccessibleName("Campaign missions and rewards");
    missionChoices->setWordWrap(true);
    missionChoices->setFixedHeight(190);
    side->addWidget(missionChoices);
    auto *campaignFiles = new QHBoxLayout;
    side->addLayout(campaignFiles);
    auto *saveCampaign = button("Save campaign…", "saveCampaignButton", campaignFiles);
    auto *loadCampaign = button("Load campaign…", "loadCampaignButton", campaignFiles);
    auto *continueCampaign = button("Continue", "continueCampaignButton");
    auto *help = new QLabel("Select your wizard to cast.\nSummon: nearby empty cell.\n"
                            "Arc Bolt: click an enemy.\n"
                            "Sentinel: right-click to move/attack.\nSpace: Play/Pause.");
    help->setMinimumHeight(130);
    help->setWordWrap(true);
    side->addWidget(help);
    auto *status = new QLabel;
    status->setWordWrap(true);
    status->setObjectName("feedbackStatus");
    side->addWidget(status);
    side->addStretch();
    auto refresh = [&c, canvas, player, enemy, selection, status, summon, bolt, sphere, cure, stop, play, pause,
                    step, save, campaignStatus, saveCampaign, continueCampaign, help, missionStatus, missionScroll, missionChoices] {
        auto v = c.view();
        canvas->present(v);
        const auto missionText=(v.objective+"\n\n"+v.briefing+"\n\n"+v.reward).trimmed();
        missionStatus->setText(missionText);
        missionStatus->setVisible(!missionText.isEmpty());
        missionScroll->setVisible(!missionText.isEmpty());
        missionStatus->setMinimumHeight(missionText.isEmpty()?0:missionStatus->heightForWidth(560));
        {
            QSignalBlocker block(missionChoices);
            bool rebuild=missionChoices->count()!=int(v.choices.size());
            for(int i=0;!rebuild && i<missionChoices->count();++i) {
                const auto *item=missionChoices->item(i);const auto &choice=v.choices.at(i);
                rebuild=item->text()!=choice.text || bool(item->flags()&Qt::ItemIsEnabled)!=choice.enabled;
            }
            const auto scroll=missionChoices->verticalScrollBar()->value();
            if(rebuild) {
                missionChoices->clear();
                for(const auto &choice:v.choices) {
                    auto *item=new QListWidgetItem(choice.text,missionChoices);
                    item->setData(Qt::UserRole,choice.realm);item->setData(Qt::UserRole+1,choice.region);
                    item->setToolTip(choice.detail);
                    item->setSizeHint(QSize(320,42));
                    if(!choice.enabled) item->setFlags(item->flags()&~Qt::ItemIsEnabled);
                }
            }
            for(std::size_t i=0;i<v.choices.size();++i) if(v.choices[i].selected) missionChoices->setCurrentRow(int(i));
            missionChoices->verticalScrollBar()->setValue(scroll);
            missionChoices->setVisible(!v.choices.empty());
        }
        help->setText(QString("Select your wizard to cast.\nSummon: nearby empty cell.\nArc Bolt: click an enemy.\n%1: right-click to move/attack.\nSpace: Play/Pause.").arg(v.creatureName));
        player->setVisible(c.hasBattle());
        enemy->setVisible(c.hasBattle());
        player->setText(
            QString("Your wizard · %1 HP\nMana %2 / 20").arg(v.wizardHealth[0]).arg(v.mana[0]));
        enemy->setText(
            QString("Rival wizard · %1 HP\nMana %2 / 20").arg(v.wizardHealth[1]).arg(v.mana[1]));
        selection->setText(v.selection);
        status->setText(v.feedback);
        summon->setText(QString("Summon %1 · 8 mana").arg(v.creatureName));
        cure->setToolTip(QString("Heal a living friendly %1 by 8, up to its maximum health.").arg(v.creatureName));
        summon->setEnabled(v.canCast && !v.busy && v.mana[0] >= game::SkirmishRules::summonCost);
        bolt->setEnabled(v.canCast && !v.busy && v.mana[0] >= game::SkirmishRules::boltCost);
        sphere->setEnabled(v.sphereEnabled && v.canCast && !v.busy && v.mana[0] >= 8);
        cure->setEnabled(v.cureEnabled && v.canCast && !v.busy && v.mana[0] >= v.cureMana);
        stop->setEnabled(v.canOrder);
        const bool running = c.hasBattle() && v.outcome == game::Outcome::running;
        save->setEnabled(!c.campaign());
        saveCampaign->setEnabled(c.campaign());
        continueCampaign->setEnabled(c.campaign() && !c.hasBattle() &&
                                     !c.campaign()->progress().completed);
        if (c.campaign()) {
            const auto &progress = c.campaign()->progress();
            campaignStatus->setText(QString("CAMPAIGN · %1\n%2")
                                        .arg(QString::fromStdString(progress.region),
                                             progress.completed ? "Complete"
                                             : c.hasBattle()    ? "Mission in progress"
                                                                : "Between missions"));
            const bool retry = progress.lastResult &&
                               progress.lastResult->outcome != game::CampaignOutcome::victory;
            continueCampaign->setText(retry ? "Retry" : "Continue");
        } else {
            campaignStatus->setText("Standalone skirmish");
            continueCampaign->setText("Continue");
        }
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
    QObject::connect(sphere, &QPushButton::clicked, this,
                     guarded([&c] { c.setMode(CursorMode::sphere); }));
    QObject::connect(cure, &QPushButton::clicked, this,
                     guarded([&c] { c.setMode(CursorMode::cure); }));
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
    QObject::connect(mapCampaign, &QPushButton::clicked, this, guarded([&c,this] {
        if(c.hasMapRoot()) c.newMapCampaign();
        else {
            c.pause();
            const auto root=QFileDialog::getExistingDirectory(this,"Choose installed Magic & Mayhem game data");
            if(!root.isEmpty()) c.newMapCampaign(root.toStdString());
        }
    }));
    QObject::connect(missionCampaign, &QPushButton::clicked, this, guarded([&c,this] {
        if(c.hasMapRoot()) c.newMapCampaign({},true);
        else {c.pause();const auto root=QFileDialog::getExistingDirectory(this,"Choose installed Magic & Mayhem game data");if(!root.isEmpty()) c.newMapCampaign(root.toStdString(),true);}
    }));
    QObject::connect(crossroadsCampaign, &QPushButton::clicked, this, guarded([&c,this] {
        if(c.hasMapRoot()) c.newCrossroadsCampaign();
        else {c.pause();const auto root=QFileDialog::getExistingDirectory(this,"Choose installed Magic & Mayhem game data");if(!root.isEmpty()) c.newCrossroadsCampaign(root.toStdString());}
    }));
    QObject::connect(missionChoices, &QListWidget::currentRowChanged, this, [&c,missionChoices,status](int row) {
        auto *item=missionChoices->item(row);if(!item || !(item->flags()&Qt::ItemIsEnabled)) return;
        try {c.selectCampaignMission(item->data(Qt::UserRole).toString(),item->data(Qt::UserRole+1).toString());}
        catch(const std::exception &e) {status->setText(QString::fromStdString(e.what()));}
    });
    QObject::connect(newCampaign, &QPushButton::clicked, this, guarded([&c] { c.newCampaign(); }));
    QObject::connect(continueCampaign, &QPushButton::clicked, this,
                     guarded([&c] { c.continueCampaign(); }));
    QObject::connect(saveCampaign, &QPushButton::clicked, this, guarded([&c, this] {
                         c.pause();
                         const auto path = QFileDialog::getSaveFileName(
                             this, "Save native campaign", {}, "Native campaign (*.mnmc)");
                         if (!path.isEmpty())
                             c.saveCampaign(path.toStdString(), true);
                     }));
    QObject::connect(loadCampaign, &QPushButton::clicked, this, guarded([&c, this] {
                         c.pause();
                         const auto path = QFileDialog::getOpenFileName(
                             this, "Load native campaign", {}, "Native campaign (*.mnmc)");
                         if (!path.isEmpty())
                             c.loadCampaign(path.toStdString());
                     }));
    auto *space = new QShortcut(QKeySequence(Qt::Key_Space), this);
    QObject::connect(space, &QShortcut::activated, this, [&c] {
        if (c.playing())
            c.pause();
        else
            c.play();
    });
    refresh();
    resize(1030,900);
}
} // namespace mnm::skirmish

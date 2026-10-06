#include "movement_controls.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSignalBlocker>
#include <QSpinBox>
#include <stdexcept>
namespace mnm::scene {
namespace {
QString action(game::Action a) {
    switch(a) {
    case game::Action::idle:return "Idle";case game::Action::planning:return "Planning";
    case game::Action::moving:return "Moving";case game::Action::arrived:return "Arrived";
    case game::Action::blocked:return "Blocked";case game::Action::searchLimited:return "Search limited";
    case game::Action::cancelled:return "Cancelled";
    }
    throw std::invalid_argument("Unknown movement action");
}
}
MovementControls::MovementControls(QWidget* parent):QWidget(parent) {
    auto* layout=new QFormLayout(this);creatures_=new QComboBox(this);creatures_->setObjectName("creatureSelection");
    layout->addRow("Creature",creatures_);auto* coordinates=new QHBoxLayout;
    x_=new QSpinBox(this);y_=new QSpinBox(this);z_=new QSpinBox(this);
    x_->setObjectName("targetX");y_->setObjectName("targetY");z_->setObjectName("targetZ");
    for(auto* spin:{x_,y_,z_}) coordinates->addWidget(spin);
    x_->setPrefix("X ");y_->setPrefix("Y ");z_->setPrefix("Z ");
    layout->addRow("Target cell",coordinates);move_=new QPushButton("Queue move",this);move_->setObjectName("queueMove");layout->addRow(move_);
    stop_=new QPushButton("Queue stop",this);stop_->setObjectName("queueStop");layout->addRow(stop_);
    cancel_=new QPushButton("Cancel queued moves",this);cancel_->setObjectName("cancelQueuedMoves");layout->addRow(cancel_);
    stop_->setToolTip("Cancel the selected creature route on the next simulation tick. Later queued moves can restart it.");
    cancel_->setToolTip("Remove selected pending moves now; active movement and queued stops remain.");
    connect(stop_,&QPushButton::clicked,this,[this] {if(onStop) onStop();});
    connect(cancel_,&QPushButton::clicked,this,[this] {if(onCancelQueuedMoves) onCancelQueuedMoves();});
    connect(creatures_,&QComboBox::currentIndexChanged,this,[this](int i) {
        if(onSelect) onSelect(i>0 && std::size_t(i)<=choices_.size()?std::optional<game::Handle>(choices_[i-1].handle):std::nullopt);
    });
    connect(move_,&QPushButton::clicked,this,[this] {if(onMove) onMove({x_->value(),y_->value(),z_->value()});});
    spawnCell_=new QLabel("Select terrain to spawn",this);spawnCell_->setObjectName("spawnCell");layout->addRow("Spawn cell",spawnCell_);
    spawn_=new QPushButton("Spawn creature",this);spawn_->setObjectName("spawnCreature");layout->addRow(spawn_);
    connect(spawn_,&QPushButton::clicked,this,[this] {if(onSpawn) onSpawn();});
    updateSpawning({},false,true);
    updateChoices({},std::nullopt,{1,1,1});
}
void MovementControls::updateChoices(std::vector<CreatureChoice> choices,std::optional<game::Handle> selected,game::Point d) {
    if(d.x<1 || d.y<1 || d.z<1 || choices.size()>32) throw std::invalid_argument("Invalid movement controls domain");
    QSignalBlocker blocker(creatures_);creatures_->clear();creatures_->addItem("Select a creature");
    int index=0;const CreatureChoice* current=nullptr;
    for(std::size_t i=0;i<choices.size();++i) {
        const auto& c=choices[i];creatures_->addItem(QString("Creature %1 · %2,%3,%4 · %5").arg(i+1).arg(c.position.x).arg(c.position.y).arg(c.position.z).arg(action(c.action)));
        if(selected && c.handle==*selected) {index=int(i+1);current=&c;}
    }
    const bool changed=!(selected_==selected);selected_=current?selected:std::nullopt;
    x_->setRange(0,d.x-1);y_->setRange(0,d.y-1);z_->setRange(0,d.z-1);
    if(current && changed) {x_->setValue(current->position.x);y_->setValue(current->position.y);z_->setValue(current->position.z);}
    creatures_->setCurrentIndex(index);choices_=std::move(choices);
    for(auto* spin:{x_,y_,z_}) spin->setEnabled(bool(selected_));
    move_->setEnabled(bool(selected_));stop_->setEnabled(bool(selected_));cancel_->setEnabled(bool(selected_));
}
void MovementControls::updateSpawning(std::optional<game::Point> cell,bool playing,bool capacity) {
    spawnCell_->setText(cell?QString("%1,%2,%3").arg(cell->x).arg(cell->y).arg(cell->z):QString("Select terrain to spawn"));
    spawn_->setEnabled(bool(cell) && !playing && capacity);
    spawn_->setToolTip(playing?"Pause before spawning":!capacity?"Scene creature limit reached":!cell?"Left-click a terrain cell":"Spawn the scene creature in the selected terrain cell");
}
void MovementControls::setTarget(game::Point p) {
    if(p.x<x_->minimum() || p.x>x_->maximum() || p.y<y_->minimum() || p.y>y_->maximum() || p.z<z_->minimum() || p.z>z_->maximum())
        throw std::invalid_argument("Target cell outside controls domain");
    x_->setValue(p.x);y_->setValue(p.y);z_->setValue(p.z);
}

}

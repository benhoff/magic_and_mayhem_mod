#include "map_selection_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <stdexcept>
MapSelectionWidget::MapSelectionWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240);
    setWindowTitle("Magic & Mayhem — Map Selection preview");
    heading_=new QLabel("Map Selection",this); heading_->setObjectName("mapSelectionHeading");
    heading_->setTextFormat(Qt::PlainText); heading_->setAlignment(Qt::AlignCenter);
    heading_->setStyleSheet("color: #3e2313; background: transparent;");
    maps_=new QListWidget(this); maps_->setObjectName("mapSelectionList");
    maps_->installEventFilter(this);
    maps_->setSelectionMode(QAbstractItemView::SingleSelection);
    maps_->setStyleSheet("QListWidget { color: #3e2313; background: transparent; border: 1px solid #ac915a; }"
                        "QListWidget::item:selected { color: #fff5d6; background: #302719; }"
                        "QListWidget::item { padding: 4px; }");
    ok_=new QPushButton("OK",this); ok_->setObjectName("mapSelectionOk"); ok_->setEnabled(false);
    cancel_=new QPushButton("Cancel",this); cancel_->setObjectName("mapSelectionCancel");
    for (auto* button:{ok_,cancel_}) { button->setStyleSheet(mnm::ui::menuButtonStyle()); button->setCursor(Qt::PointingHandCursor); }
    connect(maps_,&QListWidget::itemSelectionChanged,this,[this]{ok_->setEnabled(!selectedMapId().isEmpty());});
    connect(maps_,&QListWidget::itemActivated,this,[this]{confirmSelection();});
    connect(ok_,&QPushButton::clicked,this,&MapSelectionWidget::confirmSelection);
    connect(cancel_,&QPushButton::clicked,this,&MapSelectionWidget::cancelled);
    QWidget::setTabOrder(maps_,ok_); QWidget::setTabOrder(ok_,cancel_);
}
bool MapSelectionWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/MapSelectionScreen","screen (Map Selection Screen).cfg");
        const auto heading=assets.layout.value("TEXT_1"),list=assets.layout.value("LISTBOX_1");
        const auto ok=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto headingRect=mnm::ui::rectangle(heading.value("Rect2")),listRect=mnm::ui::rectangle(list.value("Rect2"));
        const auto okRect=mnm::ui::rectangle(ok.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        const auto headingText=mnm::ui::textLabel(assets.strings,heading.value("Text"));
        const auto okText=mnm::ui::textLabel(assets.strings,ok.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (heading.value("Font")!="LARGE" || heading.value("TextFlags")!="CENTRE" || list.value("Font")!="SMALL" || ok.value("Font")!="LARGE" || cancel.value("Font")!="LARGE")
            throw std::runtime_error("Invalid Map Selection font or alignment role");
        background_=assets.background; headingRectangle_=headingRect; listRectangle_=listRect; okRectangle_=okRect; cancelRectangle_=cancelRect;
        heading_->setText(headingText); ok_->setText(okText); cancel_->setText(cancelText);
        arrange(); update(); return true;
    } catch (const std::exception& failure) { if (error) *error=QString::fromUtf8(failure.what()); return false; }
}
bool MapSelectionWidget::setMaps(const QVector<Map>& maps, const QString& selectedId, QString* error) {
    if (error) error->clear();
    QSet<QString> identifiers;
    for (const auto& map:maps) {
        if (map.id.isEmpty() || map.name.isEmpty() || identifiers.contains(map.id)) {
            if (error) *error="Map IDs and names must be nonempty; IDs must be unique.";
            return false;
        }
        identifiers.insert(map.id);
    }
    if (!selectedId.isEmpty() && !identifiers.contains(selectedId)) {
        if (error) *error="Requested map is not present.";
        return false;
    }
    const auto current=selectedId.isEmpty()?selectedMapId():selectedId;
    QSignalBlocker blocker(maps_); maps_->clear();
    for (const auto& map:maps) {
        auto* item=new QListWidgetItem(map.name,maps_); item->setData(Qt::UserRole,map.id);
        if (map.id==current) maps_->setCurrentItem(item);
    }
    ok_->setEnabled(!selectedMapId().isEmpty()); return true;
}
QString MapSelectionWidget::selectedMapId() const {
    const auto selected=maps_->selectedItems();
    return selected.isEmpty()?QString():selected.front()->data(Qt::UserRole).toString();
}
void MapSelectionWidget::confirmSelection() {
    const auto id=selectedMapId(); if (!id.isEmpty()) emit mapSelected(id);
}
void MapSelectionWidget::focusSelection() { (maps_->count()?static_cast<QWidget*>(maps_):static_cast<QWidget*>(cancel_))->setFocus(Qt::OtherFocusReason); }
QRect MapSelectionWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void MapSelectionWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    heading_->setGeometry(map(headingRectangle_)); maps_->setGeometry(map(listRectangle_)); ok_->setGeometry(map(okRectangle_)); cancel_->setGeometry(map(cancelRectangle_));
    QFont font("serif"); font.setBold(true); font.setPixelSize(qMax(10,qRound(26*scale)));
    heading_->setFont(font); ok_->setFont(font); cancel_->setFont(font);
    font.setPixelSize(qMax(10,qRound(20*scale))); maps_->setFont(font);
}
void MapSelectionWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void MapSelectionWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape) { event->accept(); if (!event->isAutoRepeat()) emit cancelled(); return; }
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept(); if (!event->isAutoRepeat()) { if (cancel_->hasFocus()) emit cancelled(); else confirmSelection(); } return;
    }
    QWidget::keyPressEvent(event);
}
void MapSelectionWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

bool MapSelectionWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched==maps_ && event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept(); if (!key->isAutoRepeat()) confirmSelection(); return true;
        }
    }
    return QWidget::eventFilter(watched,event);
}

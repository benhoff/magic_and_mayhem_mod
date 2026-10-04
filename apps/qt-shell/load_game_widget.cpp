#include "load_game_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <stdexcept>
LoadGameWidget::LoadGameWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240); setWindowTitle("Magic & Mayhem — Load Game preview");
    heading_=new QLabel("Load Game",this); heading_->setObjectName("loadGameHeading");
    heading_->setTextFormat(Qt::PlainText); heading_->setAlignment(Qt::AlignCenter);
    heading_->setStyleSheet("color: #3e2313; background: transparent;");
    fileName_=new QLineEdit(this); fileName_->setObjectName("loadGameFileName"); fileName_->setMaxLength(256);
    fileName_->setAccessibleName("Save filename");
    saves_=new QListWidget(this); saves_->setObjectName("loadGameList"); saves_->setSelectionMode(QAbstractItemView::SingleSelection);
    saves_->setAccessibleName("Saved games");
    fileName_->setStyleSheet("QLineEdit { color: #3e2313; background: transparent; border: 1px solid #ac915a; selection-color: #fff5d6; selection-background-color: #302719; }");
    saves_->setStyleSheet("QListWidget { color: #3e2313; background: transparent; border: 1px solid #ac915a; }"
                         "QListWidget::item:selected { color: #fff5d6; background: #302719; }"
                         "QListWidget::item { padding: 4px; }");
    load_=new QPushButton("Load",this); load_->setObjectName("loadGameLoad"); load_->setEnabled(false);
    cancel_=new QPushButton("Cancel",this); cancel_->setObjectName("loadGameCancel");
    for (auto* button:{load_,cancel_}) { button->setStyleSheet(mnm::ui::menuButtonStyle()); button->setCursor(Qt::PointingHandCursor); }
    saves_->installEventFilter(this); fileName_->installEventFilter(this);
    connect(saves_,&QListWidget::itemSelectionChanged,this,[this] {
        QSignalBlocker blocker(fileName_); const auto selected=saves_->selectedItems();
        fileName_->setText(selected.isEmpty()?QString():selected.front()->text()); load_->setEnabled(!selectedSaveId().isEmpty());
    });
    connect(fileName_,&QLineEdit::textChanged,this,&LoadGameWidget::selectFileName);
    connect(saves_,&QListWidget::itemActivated,this,[this]{confirmSelection();});
    connect(load_,&QPushButton::clicked,this,&LoadGameWidget::confirmSelection);
    connect(cancel_,&QPushButton::clicked,this,&LoadGameWidget::cancelled);
    QWidget::setTabOrder(saves_,fileName_); QWidget::setTabOrder(fileName_,load_); QWidget::setTabOrder(load_,cancel_);
}
bool LoadGameWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/LoadGame","screen (Load Game).cfg");
        const auto heading=assets.layout.value("TEXT_2"),list=assets.layout.value("LISTBOX_1"),edit=assets.layout.value("EDITBOX_1");
        const auto load=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto headingRect=mnm::ui::rectangle(heading.value("Rect2")),listRect=mnm::ui::rectangle(list.value("Rect2")),editRect=mnm::ui::rectangle(edit.value("Rect2"));
        const auto loadRect=mnm::ui::rectangle(load.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        const auto headingText=mnm::ui::textLabel(assets.strings,heading.value("Text"));
        const auto loadText=mnm::ui::textLabel(assets.strings,load.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (heading.value("Font")!="LARGE" || heading.value("TextFlags")!="CENTRE" || list.value("Font")!="SMALL" || edit.value("Font")!="SMALL" || load.value("Font")!="LARGE" || cancel.value("Font")!="LARGE")
            throw std::runtime_error("Invalid Load Game font or alignment role");
        background_=assets.background; headingRectangle_=headingRect; listRectangle_=listRect; editRectangle_=editRect; loadRectangle_=loadRect; cancelRectangle_=cancelRect;
        heading_->setText(headingText); load_->setText(loadText); cancel_->setText(cancelText); arrange(); update(); return true;
    } catch (const std::exception& failure) { if (error) *error=QString::fromUtf8(failure.what()); return false; }
}
bool LoadGameWidget::setSaves(const QVector<Save>& saves, const QString& selectedId, QString* error) {
    if (error) error->clear();
    QSet<QString> identifiers,names;
    for (const auto& save:saves) {
        if (save.id.isEmpty() || save.fileName.isEmpty() || save.fileName.size()>256 || save.fileName.contains('\n') || save.fileName.contains('\r') || identifiers.contains(save.id) || names.contains(save.fileName)) {
            if (error) *error="Save IDs and filenames must be nonempty and unique; filenames must be a single line of at most 256 characters.";
            return false;
        }
        identifiers.insert(save.id); names.insert(save.fileName);
    }
    if (!selectedId.isEmpty() && !identifiers.contains(selectedId)) { if (error) *error="Requested save is not present."; return false; }
    const auto current=selectedId.isEmpty()?selectedSaveId():selectedId;
    QSignalBlocker listBlocker(saves_),editBlocker(fileName_); saves_->clear(); fileName_->clear();
    for (const auto& save:saves) {
        auto* item=new QListWidgetItem(save.fileName,saves_); item->setData(Qt::UserRole,save.id);
        if (save.id==current) { saves_->setCurrentItem(item); fileName_->setText(save.fileName); }
    }
    load_->setEnabled(!selectedSaveId().isEmpty()); return true;
}
void LoadGameWidget::selectFileName(const QString& name) {
    QSignalBlocker blocker(saves_); saves_->clearSelection(); saves_->setCurrentRow(-1);
    for (int i=0;i<saves_->count();++i) if (saves_->item(i)->text()==name) { saves_->setCurrentRow(i); break; }
    load_->setEnabled(!selectedSaveId().isEmpty());
}
QString LoadGameWidget::selectedSaveId() const {
    const auto selected=saves_->selectedItems();
    return selected.isEmpty() || selected.front()->text()!=fileName_->text()?QString():selected.front()->data(Qt::UserRole).toString();
}
void LoadGameWidget::confirmSelection() { const auto id=selectedSaveId(); if (!id.isEmpty()) emit loadRequested(id); }
void LoadGameWidget::focusSelection() { (saves_->count()?static_cast<QWidget*>(saves_):static_cast<QWidget*>(cancel_))->setFocus(Qt::OtherFocusReason); }
QRect LoadGameWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void LoadGameWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    heading_->setGeometry(map(headingRectangle_)); saves_->setGeometry(map(listRectangle_)); fileName_->setGeometry(map(editRectangle_)); load_->setGeometry(map(loadRectangle_)); cancel_->setGeometry(map(cancelRectangle_));
    QFont font("serif"); font.setBold(true); font.setPixelSize(qMax(10,qRound(26*scale))); heading_->setFont(font); load_->setFont(font); cancel_->setFont(font);
    font.setPixelSize(qMax(10,qRound(20*scale))); saves_->setFont(font); fileName_->setFont(font);
}
void LoadGameWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
bool LoadGameWidget::eventFilter(QObject* watched, QEvent* event) {
    if ((watched==saves_ || watched==fileName_) && event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) { key->accept(); if (!key->isAutoRepeat()) confirmSelection(); return true; }
    }
    return QWidget::eventFilter(watched,event);
}
void LoadGameWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape) { event->accept(); if (!event->isAutoRepeat()) emit cancelled(); return; }
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept(); if (!event->isAutoRepeat()) { if (cancel_->hasFocus()) emit cancelled(); else confirmSelection(); } return;
    }
    QWidget::keyPressEvent(event);
}
void LoadGameWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

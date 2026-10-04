#include "save_game_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QHideEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <stdexcept>
namespace {
bool validName(const QString& name) {
    return !name.trimmed().isEmpty() && name.size()<=256 && name!="." && name!=".." &&
        !name.contains('/') && !name.contains('\\') && !name.contains(':') && !name.contains('\n') && !name.contains('\r') && !name.contains(QChar(0));
}
}
SaveGameWidget::SaveGameWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240); setWindowTitle("Magic & Mayhem — Save Game preview");
    heading_=new QLabel("Save Game",this); heading_->setObjectName("saveGameHeading");
    heading_->setTextFormat(Qt::PlainText); heading_->setAlignment(Qt::AlignCenter);
    heading_->setStyleSheet("color: #3e2313; background: transparent;");
    fileName_=new QLineEdit(this); fileName_->setObjectName("saveGameFileName"); fileName_->setMaxLength(256);
    fileName_->setAccessibleName("Save filename");
    saves_=new QListWidget(this); saves_->setObjectName("saveGameList"); saves_->setSelectionMode(QAbstractItemView::SingleSelection);
    saves_->setAccessibleName("Saved games");
    fileName_->setStyleSheet("QLineEdit { color: #3e2313; background: transparent; border: 1px solid #ac915a; selection-color: #fff5d6; selection-background-color: #302719; }");
    saves_->setStyleSheet("QListWidget { color: #3e2313; background: transparent; border: 1px solid #ac915a; }"
                         "QListWidget::item:selected { color: #fff5d6; background: #302719; }"
                         "QListWidget::item { padding: 4px; }");
    save_=new QPushButton("Save",this); save_->setObjectName("saveGameSave"); save_->setEnabled(false);
    cancel_=new QPushButton("Cancel",this); cancel_->setObjectName("saveGameCancel");
    delete_=new QPushButton("Delete",this); delete_->setObjectName("saveGameDelete"); delete_->setEnabled(false);
    for (auto* button:{save_,cancel_,delete_}) { button->setStyleSheet(mnm::ui::menuButtonStyle()); button->setCursor(Qt::PointingHandCursor); }
    saves_->installEventFilter(this); fileName_->installEventFilter(this);
    connect(saves_,&QListWidget::itemSelectionChanged,this,[this] {
        ++revision_; discardConfirmation();
        QSignalBlocker blocker(fileName_); const auto selected=saves_->selectedItems();
        fileName_->setText(selected.isEmpty()?QString():selected.front()->text()); updateActions();
    });
    connect(fileName_,&QLineEdit::textChanged,this,&SaveGameWidget::selectFileName);
    connect(saves_,&QListWidget::itemActivated,this,[this]{requestSave();});
    connect(save_,&QPushButton::clicked,this,&SaveGameWidget::requestSave);
    connect(delete_,&QPushButton::clicked,this,&SaveGameWidget::requestDelete);
    connect(cancel_,&QPushButton::clicked,this,[this]{discardConfirmation(); emit cancelled();});
    QWidget::setTabOrder(saves_,fileName_); QWidget::setTabOrder(fileName_,save_); QWidget::setTabOrder(save_,delete_); QWidget::setTabOrder(delete_,cancel_);
}
bool SaveGameWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/SaveGame","screen (Save Game).cfg");
        const auto heading=assets.layout.value("TEXT_2"),list=assets.layout.value("LISTBOX_1"),edit=assets.layout.value("EDITBOX_1");
        const auto load=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2"),remove=assets.layout.value("TEXTBUTTON_3");
        const auto headingRect=mnm::ui::rectangle(heading.value("Rect2")),listRect=mnm::ui::rectangle(list.value("Rect2")),editRect=mnm::ui::rectangle(edit.value("Rect2"));
        const auto loadRect=mnm::ui::rectangle(load.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        const auto deleteRect=mnm::ui::rectangle(remove.value("Rect2"));
        const auto deleteText=mnm::ui::textLabel(assets.strings,remove.value("Text"));
        const auto headingText=mnm::ui::textLabel(assets.strings,heading.value("Text"));
        const auto loadText=mnm::ui::textLabel(assets.strings,load.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (heading.value("Font")!="LARGE" || heading.value("TextFlags")!="CENTRE" || list.value("Font")!="SMALL" || edit.value("Font")!="SMALL" || load.value("Font")!="LARGE" || cancel.value("Font")!="LARGE" || remove.value("Font")!="LARGE")
            throw std::runtime_error("Invalid Save Game font or alignment role");
        ++revision_; discardConfirmation();
        deleteRectangle_=deleteRect; delete_->setText(deleteText);
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_=assets.background; headingRectangle_=headingRect; listRectangle_=listRect; editRectangle_=editRect; saveRectangle_=loadRect; cancelRectangle_=cancelRect;
        heading_->setText(headingText); save_->setText(loadText); cancel_->setText(cancelText); arrange(); update(); return true;
    } catch (const std::exception& failure) { if (error) *error=QString::fromUtf8(failure.what()); return false; }
}
bool SaveGameWidget::setSaves(const QVector<Save>& saves, const QString& selectedId, QString* error) {
    if (error) error->clear();
    QSet<QString> identifiers,names;
    for (const auto& save:saves) {
        if (save.id.isEmpty() || !validName(save.fileName) || identifiers.contains(save.id) || names.contains(save.fileName)) {
            if (error) *error="Save IDs and filenames must be nonempty and unique; filenames must be a single line of at most 256 characters.";
            return false;
        }
        identifiers.insert(save.id); names.insert(save.fileName);
    }
    if (!selectedId.isEmpty() && !identifiers.contains(selectedId)) { if (error) *error="Requested save is not present."; return false; }
    const auto current=selectedId.isEmpty()?selectedSaveId():selectedId;
    ++revision_; discardConfirmation();
    QSignalBlocker listBlocker(saves_),editBlocker(fileName_); saves_->clear(); fileName_->clear();
    for (const auto& save:saves) {
        auto* item=new QListWidgetItem(save.fileName,saves_); item->setData(Qt::UserRole,save.id);
        if (save.id==current) { saves_->setCurrentItem(item); fileName_->setText(save.fileName); }
    }
    updateActions(); return true;
}
void SaveGameWidget::selectFileName(const QString& name) {
    ++revision_; discardConfirmation();
    QSignalBlocker blocker(saves_); saves_->clearSelection(); saves_->setCurrentRow(-1);
    for (int i=0;i<saves_->count();++i) if (saves_->item(i)->text()==name) { saves_->setCurrentRow(i); break; }
    updateActions();
}
QString SaveGameWidget::selectedSaveId() const {
    const auto selected=saves_->selectedItems();
    return selected.isEmpty() || selected.front()->text()!=fileName_->text()?QString():selected.front()->data(Qt::UserRole).toString();
}
void SaveGameWidget::updateActions() {
    save_->setEnabled(validName(fileName_->text())); delete_->setEnabled(!selectedSaveId().isEmpty());
}
void SaveGameWidget::discardConfirmation() {
    if (!confirmation_) return;
    disconnect(confirmation_,nullptr,this,nullptr); confirmation_->close(); confirmation_->deleteLater(); confirmation_.clear();
}
void SaveGameWidget::requestSave() {
    if (!validName(fileName_->text()) || confirmation_) return;
    if (selectedSaveId().isEmpty()) emit saveRequested(fileName_->text(),QString());
    else confirmAction(false);
}
void SaveGameWidget::requestDelete() { if (!selectedSaveId().isEmpty() && !confirmation_) confirmAction(true); }
void SaveGameWidget::confirmAction(bool deleting) {
    const auto id=selectedSaveId(),name=fileName_->text(); const auto revision=revision_;
    auto* dialog=new QMessageBox(QMessageBox::Question,deleting?"Delete save":"Overwrite save",
        deleting?QString("Request deletion of %1?").arg(name):QString("Request overwrite of %1?").arg(name),QMessageBox::Yes|QMessageBox::No,this);
    dialog->setObjectName(deleting?"saveGameDeleteConfirmation":"saveGameOverwriteConfirmation");
    dialog->setTextFormat(Qt::PlainText); dialog->setDefaultButton(QMessageBox::No);
    dialog->setWindowModality(Qt::WindowModal); dialog->setAttribute(Qt::WA_DeleteOnClose); confirmation_=dialog;
    connect(dialog,&QMessageBox::finished,this,[this,id,name,revision,deleting](int result) {
        confirmation_.clear();
        if (result!=QMessageBox::Yes || revision!=revision_ || selectedSaveId()!=id || fileName_->text()!=name) return;
        if (deleting) emit deleteRequested(id); else emit saveRequested(name,id);
    });
    dialog->open();
}
void SaveGameWidget::hideEvent(QHideEvent* event) { ++revision_; discardConfirmation(); QWidget::hideEvent(event); }
void SaveGameWidget::focusSelection() { (saves_->count()?static_cast<QWidget*>(saves_):static_cast<QWidget*>(cancel_))->setFocus(Qt::OtherFocusReason); }
QRect SaveGameWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void SaveGameWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    heading_->setGeometry(map(headingRectangle_)); saves_->setGeometry(map(listRectangle_)); fileName_->setGeometry(map(editRectangle_)); save_->setGeometry(map(saveRectangle_)); cancel_->setGeometry(map(cancelRectangle_)); delete_->setGeometry(map(deleteRectangle_));
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale); heading_->setFont(font); save_->setFont(font); cancel_->setFont(font); delete_->setFont(font);
    font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Body,scale); saves_->setFont(font); fileName_->setFont(font);
}
void SaveGameWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
bool SaveGameWidget::eventFilter(QObject* watched, QEvent* event) {
    if ((watched==saves_ || watched==fileName_) && event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) { key->accept(); if (!key->isAutoRepeat()) requestSave(); return true; }
    }
    return QWidget::eventFilter(watched,event);
}
void SaveGameWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape) { event->accept(); if (!event->isAutoRepeat()) { discardConfirmation(); emit cancelled(); } return; }
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept(); if (!event->isAutoRepeat()) { if (cancel_->hasFocus()) { discardConfirmation(); emit cancelled(); } else if (delete_->hasFocus()) requestDelete(); else requestSave(); } return;
    }
    QWidget::keyPressEvent(event);
}
void SaveGameWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

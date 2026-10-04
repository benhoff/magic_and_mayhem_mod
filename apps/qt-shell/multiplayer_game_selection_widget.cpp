#include "multiplayer_game_selection_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <stdexcept>
MultiplayerGameSelectionWidget::MultiplayerGameSelectionWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240);
    setWindowTitle("Magic & Mayhem — Multiplayer Game Selection preview");
    heading_=new QLabel("Multiplayer Game Selection",this); heading_->setObjectName("multiplayerGameSelectionHeading");
    heading_->setTextFormat(Qt::PlainText); heading_->setAlignment(Qt::AlignCenter);
    heading_->setStyleSheet("color: #3e2313; background: transparent;");
    sessions_=new QListWidget(this); sessions_->setObjectName("multiplayerGameSelectionList");
    sessions_->installEventFilter(this);
    sessions_->setSelectionMode(QAbstractItemView::SingleSelection);
    sessions_->setStyleSheet("QListWidget { color: #3e2313; background: transparent; border: 1px solid #ac915a; }"
                        "QListWidget::item:selected { color: #fff5d6; background: #302719; }"
                        "QListWidget::item { padding: 4px; }");
    ok_=new QPushButton("OK",this); ok_->setObjectName("multiplayerGameSelectionOk"); ok_->setEnabled(false);
    cancel_=new QPushButton("Cancel",this); cancel_->setObjectName("multiplayerGameSelectionCancel");
    for (auto* button:{ok_,cancel_}) { button->setStyleSheet(mnm::ui::menuButtonStyle()); button->setCursor(Qt::PointingHandCursor); }
    connect(sessions_,&QListWidget::itemSelectionChanged,this,[this]{ok_->setEnabled(!selectedSessionId().isEmpty());});
    connect(sessions_,&QListWidget::itemActivated,this,[this]{confirmSelection();});
    connect(ok_,&QPushButton::clicked,this,&MultiplayerGameSelectionWidget::confirmSelection);
    connect(cancel_,&QPushButton::clicked,this,&MultiplayerGameSelectionWidget::cancelled);
    QWidget::setTabOrder(sessions_,ok_); QWidget::setTabOrder(ok_,cancel_);
}
bool MultiplayerGameSelectionWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/MultiplayerGameSelect","Screen (Multiplayer Game Selection).cfg");
        const auto heading=assets.layout.value("TEXT_1"),list=assets.layout.value("LISTBOX_1");
        const auto ok=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto headingRect=mnm::ui::rectangle(heading.value("Rect2")),listRect=mnm::ui::rectangle(list.value("Rect2"));
        const auto okRect=mnm::ui::rectangle(ok.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        const auto headingText=mnm::ui::textLabel(assets.strings,heading.value("Text"));
        const auto okText=mnm::ui::textLabel(assets.strings,ok.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (heading.value("Font")!="LARGE" || heading.value("TextFlags")!="MIDDLE" || list.value("Font")!="SMALL" || ok.value("Font")!="LARGE" || cancel.value("Font")!="LARGE")
            throw std::runtime_error("Invalid Multiplayer Game Selection font or alignment role");
        background_=assets.background; headingRectangle_=headingRect; listRectangle_=listRect; okRectangle_=okRect; cancelRectangle_=cancelRect;
        heading_->setText(headingText); ok_->setText(okText); cancel_->setText(cancelText);
        arrange(); update(); return true;
    } catch (const std::exception& failure) { if (error) *error=QString::fromUtf8(failure.what()); return false; }
}
bool MultiplayerGameSelectionWidget::setSessions(const QVector<Session>& sessions, const QString& selectedId, QString* error) {
    if (error) error->clear();
    QSet<QString> identifiers;
    for (const auto& session:sessions) {
        if (session.id.isEmpty() || session.name.isEmpty() || identifiers.contains(session.id)) {
            if (error) *error="Session IDs and names must be nonempty; IDs must be unique.";
            return false;
        }
        identifiers.insert(session.id);
    }
    if (!selectedId.isEmpty() && !identifiers.contains(selectedId)) {
        if (error) *error="Requested session is not present.";
        return false;
    }
    const auto current=selectedId.isEmpty()?selectedSessionId():selectedId;
    QSignalBlocker blocker(sessions_); sessions_->clear();
    for (const auto& session:sessions) {
        auto* item=new QListWidgetItem(session.name,sessions_); item->setData(Qt::UserRole,session.id);
        if (session.id==current) sessions_->setCurrentItem(item);
    }
    ok_->setEnabled(!selectedSessionId().isEmpty()); return true;
}
QString MultiplayerGameSelectionWidget::selectedSessionId() const {
    const auto selected=sessions_->selectedItems();
    return selected.isEmpty()?QString():selected.front()->data(Qt::UserRole).toString();
}
void MultiplayerGameSelectionWidget::confirmSelection() {
    const auto id=selectedSessionId(); if (!id.isEmpty()) emit sessionSelected(id);
}
void MultiplayerGameSelectionWidget::focusSelection() { (sessions_->count()?static_cast<QWidget*>(sessions_):static_cast<QWidget*>(cancel_))->setFocus(Qt::OtherFocusReason); }
QRect MultiplayerGameSelectionWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void MultiplayerGameSelectionWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    heading_->setGeometry(map(headingRectangle_)); sessions_->setGeometry(map(listRectangle_)); ok_->setGeometry(map(okRectangle_)); cancel_->setGeometry(map(cancelRectangle_));
    QFont font("serif"); font.setBold(true); font.setPixelSize(qMax(10,qRound(26*scale)));
    heading_->setFont(font); ok_->setFont(font); cancel_->setFont(font);
    font.setPixelSize(qMax(10,qRound(20*scale))); sessions_->setFont(font);
}
void MultiplayerGameSelectionWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void MultiplayerGameSelectionWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape) { event->accept(); if (!event->isAutoRepeat()) emit cancelled(); return; }
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept(); if (!event->isAutoRepeat()) { if (cancel_->hasFocus()) emit cancelled(); else confirmSelection(); } return;
    }
    QWidget::keyPressEvent(event);
}
void MultiplayerGameSelectionWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

bool MultiplayerGameSelectionWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched==sessions_ && event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept(); if (!key->isAutoRepeat()) confirmSelection(); return true;
        }
    }
    return QWidget::eventFilter(watched,event);
}

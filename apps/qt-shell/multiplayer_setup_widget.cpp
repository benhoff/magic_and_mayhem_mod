#include "multiplayer_setup_widget.hpp"
#include "menu_assets.hpp"
#include <QButtonGroup>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <stdexcept>
namespace {
bool validText(const QString& value) {
    return value.size()<=64 && !value.contains('\n') && !value.contains('\r') && !value.contains(QChar(0));
}
bool validTransport(MultiplayerSetupWidget::Transport transport) { return int(transport)>=0 && int(transport)<=2; }
}
MultiplayerSetupWidget::MultiplayerSetupWidget(Mode mode, QWidget* parent) : QWidget(parent), mode_(mode) {
    resize(800,600); setMinimumSize(320,240);
    setWindowTitle(mode==Mode::Join?"Magic & Mayhem — Join Multiplayer preview":"Magic & Mayhem — Create Multiplayer preview");
    for (int i=0;i<4;++i) {
        labels_[i]=new QLabel(this); labels_[i]->setObjectName(QString("multiplayerText%1").arg(i+1));
        labels_[i]->setTextFormat(Qt::PlainText); labels_[i]->setAlignment(Qt::AlignCenter); labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");
    }
    for (int i=0;i<2;++i) {
        edits_[i]=new QLineEdit(this); edits_[i]->setObjectName(QString("multiplayerEdit%1").arg(i+1)); edits_[i]->setMaxLength(64);
        edits_[i]->setStyleSheet("QLineEdit { color: #3e2313; background: transparent; border: 1px solid #ac915a; selection-color: #fff5d6; selection-background-color: #302719; }");
        edits_[i]->installEventFilter(this); connect(edits_[i],&QLineEdit::textChanged,this,[this]{updateAvailability();});
    }
    edits_[0]->setAccessibleName(mode==Mode::Join?"User name":"Game name"); edits_[1]->setAccessibleName("User name");
    labels_[3]->setVisible(mode==Mode::Create); edits_[1]->setVisible(mode==Mode::Create);
    transports_=new QButtonGroup(this);
    for (int i=0;i<3;++i) {
        radios_[i]=new QRadioButton(this); radios_[i]->setObjectName(QString("multiplayerTransport%1").arg(i+1)); transports_->addButton(radios_[i],i);
        radios_[i]->setStyleSheet("QRadioButton { color: #3e2313; background: transparent; }"
            "QRadioButton::indicator { width: 14px; height: 14px; border: 1px solid #ac915a; border-radius: 7px; background: transparent; }"
            "QRadioButton::indicator:checked { background: #3e2313; }");
        radios_[i]->installEventFilter(this); connect(radios_[i],&QRadioButton::toggled,this,[this]{updateAvailability();});
    }
    ok_=new QPushButton("OK",this); ok_->setObjectName("multiplayerOk");
    cancel_=new QPushButton("Cancel",this); cancel_->setObjectName("multiplayerCancel");
    for (auto* button:{ok_,cancel_}) { button->setStyleSheet(mnm::ui::menuButtonStyle()); button->setCursor(Qt::PointingHandCursor); button->installEventFilter(this); }
    connect(ok_,&QPushButton::clicked,this,&MultiplayerSetupWidget::submit);
    connect(cancel_,&QPushButton::clicked,this,&MultiplayerSetupWidget::cancelled);
    // Transport toggles above can fire only after the action controls exist.
    radios_[1]->setChecked(true); updateAvailability();
    QWidget::setTabOrder(edits_[0],edits_[1]); QWidget::setTabOrder(edits_[1],radios_[0]);
    QWidget::setTabOrder(radios_[0],radios_[1]); QWidget::setTabOrder(radios_[1],radios_[2]);
    QWidget::setTabOrder(radios_[2],ok_); QWidget::setTabOrder(ok_,cancel_);
}
bool MultiplayerSetupWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const bool create=mode_==Mode::Create;
        const auto assets=mnm::ui::loadMenuAssets(root,create?"Interface/SetMultiplayerScreen":"Interface/JoinMultiplayerScreen",
            create?"Screen (Set Multiplayer Game).cfg":"Screen (Join Multiplayer Game).cfg");
        std::array<QRect,4> labels{}; std::array<QString,4> texts{};
        std::array<QRect,2> edits{}; std::array<QRect,3> radios{}; std::array<QString,3> choices{};
        for (int i=0;i<(create?4:3);++i) {
            const auto entry=assets.layout.value(QString("TEXT_%1").arg(i+1));
            labels[i]=mnm::ui::rectangle(entry.value("Rect2")); texts[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
            if (entry.value("Font")!=(i==0?"LARGE":"SMALL") || entry.value("TextFlags")!="MIDDLE") throw std::runtime_error("Invalid multiplayer heading role");
        }
        for (int i=0;i<(create?2:1);++i) {
            const auto entry=assets.layout.value(QString("EDITBOX_%1").arg(i+1)); edits[i]=mnm::ui::rectangle(entry.value("Rect2"));
            if (entry.value("Font")!="LARGE") throw std::runtime_error("Invalid multiplayer edit font");
        }
        for (int i=0;i<3;++i) {
            const auto entry=assets.layout.value(QString("RADIOBUTTON_%1").arg(i+1)); radios[i]=mnm::ui::rectangle(entry.value("Rect2"));
            choices[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
            if (entry.value("Font")!="SMALL" || entry.value("TextFlags")!="LEFT") throw std::runtime_error("Invalid multiplayer transport role");
        }
        const auto ok=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto okRect=mnm::ui::rectangle(ok.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        const auto okText=mnm::ui::textLabel(assets.strings,ok.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (ok.value("Font")!="LARGE" || cancel.value("Font")!="LARGE") throw std::runtime_error("Invalid multiplayer button font");
        background_=assets.background; labelRectangles_=labels; editRectangles_=edits; radioRectangles_=radios; okRectangle_=okRect; cancelRectangle_=cancelRect;
        for (int i=0;i<4;++i) labels_[i]->setText(texts[i]);
        for (int i=0;i<3;++i) radios_[i]->setText(choices[i]);
        ok_->setText(okText); cancel_->setText(cancelText); arrange(); update(); return true;
    } catch (const std::exception& failure) { if (error) *error=QString::fromUtf8(failure.what()); return false; }
}
bool MultiplayerSetupWidget::setForm(const Form& form, QString* error) {
    if (error) error->clear();
    if (!validText(form.gameName) || !validText(form.userName) || !validTransport(form.transport) || (mode_==Mode::Join && !form.gameName.isEmpty())) {
        if (error) *error="Invalid multiplayer form: use single-line names of at most 64 characters and a known transport.";
        return false;
    }
    edits_[0]->setText(mode_==Mode::Join?form.userName:form.gameName); edits_[1]->setText(mode_==Mode::Join?QString():form.userName);
    transports_->button(int(form.transport))->setChecked(true); updateAvailability(); return true;
}
MultiplayerSetupWidget::Form MultiplayerSetupWidget::form() const {
    Form result; result.transport=Transport(transports_->checkedId());
    result.gameName=mode_==Mode::Create?edits_[0]->text():QString(); result.userName=edits_[mode_==Mode::Join?0:1]->text(); return result;
}
void MultiplayerSetupWidget::updateAvailability() {
    if (!ok_) return;
    const auto value=form();
    ok_->setEnabled(validTransport(value.transport) && validText(value.userName) && !value.userName.trimmed().isEmpty() &&
        (mode_==Mode::Join || (validText(value.gameName) && !value.gameName.trimmed().isEmpty())));
}
void MultiplayerSetupWidget::submit() {
    updateAvailability(); if (!ok_->isEnabled()) return;
    const auto value=form(); emit requestSubmitted(Request{mode_,value.transport,value.gameName.trimmed(),value.userName.trimmed()});
}
void MultiplayerSetupWidget::focusFirstField() { edits_[0]->setFocus(Qt::OtherFocusReason); edits_[0]->selectAll(); }
QRect MultiplayerSetupWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void MultiplayerSetupWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    QFont font("serif"); font.setBold(true); font.setPixelSize(qMax(10,qRound(20*scale)));
    for (int i=0;i<4;++i) { labels_[i]->setGeometry(map(labelRectangles_[i])); labels_[i]->setFont(font); }
    for (int i=0;i<3;++i) { radios_[i]->setGeometry(map(radioRectangles_[i])); radios_[i]->setFont(font); }
    font.setPixelSize(qMax(10,qRound(26*scale))); labels_[0]->setFont(font); ok_->setFont(font); cancel_->setFont(font);
    for (int i=0;i<2;++i) { edits_[i]->setGeometry(map(editRectangles_[i])); edits_[i]->setFont(font); }
    ok_->setGeometry(map(okRectangle_)); cancel_->setGeometry(map(cancelRectangle_));
}
void MultiplayerSetupWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
bool MultiplayerSetupWidget::eventFilter(QObject* watched, QEvent* event) {
    if (event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Escape || key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept(); if (!key->isAutoRepeat()) { if (key->key()==Qt::Key_Escape || watched==cancel_) emit cancelled(); else submit(); } return true;
        }
    }
    return QWidget::eventFilter(watched,event);
}
void MultiplayerSetupWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape || event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept(); if (!event->isAutoRepeat()) { if (event->key()==Qt::Key_Escape) emit cancelled(); else submit(); } return;
    }
    QWidget::keyPressEvent(event);
}
void MultiplayerSetupWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

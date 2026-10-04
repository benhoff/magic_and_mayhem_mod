#include "multiplayer_lobby_widget.hpp"
#include "menu_assets.hpp"
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QStringList>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <stdexcept>
namespace {
int number(const QString& text) {
    bool ok=false;const int n=text.toInt(&ok);
    if (!ok || n<0 || n>100000) throw std::runtime_error("Invalid battle setup slider bound or step");
    return n;
}
QString labelText(const mnm::ui::Sections& strings,const QString& text) {
    if (text.startsWith('"') && text.endsWith('"') && text.size()>=2) return text.mid(1,text.size()-2);
    return mnm::ui::textLabel(strings,text);
}
}
MultiplayerLobbyWidget::MultiplayerLobbyWidget(Mode mode,QWidget* parent):QWidget(parent),mode_(mode) {
    resize(800,600);setMinimumSize(320,240);setWindowTitle(mode_==Mode::Host?"Magic & Mayhem — Host lobby preview":"Magic & Mayhem — Guest lobby preview");
    if (mode_!=Mode::Host && mode_!=Mode::Join) throw std::invalid_argument("Invalid lobby mode");
    for (int i=0;i<40;++i) {
        labels_[i]=new QLabel(this);labels_[i]->setObjectName(QString("multiplayerLobbyText%1").arg(i+1));
        labels_[i]->setTextFormat(Qt::PlainText);labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");
        labels_[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    for (int i=0;i<17;++i) {
        auto* slider=sliders_[i]=new QSlider(Qt::Horizontal,this);
        slider->setObjectName(QString("multiplayerLobbySlider%1").arg(i+1));slider->installEventFilter(this);
        slider->setRange(minimum_[i],maximum_[i]);slider->setSingleStep(step_[i]);slider->setPageStep(step_[i]);
        slider->setStyleSheet("QSlider::groove:horizontal { height: 3px; background: #ac915a; }"
            "QSlider::handle:horizontal { width: 12px; margin: -6px 0; background: #3e2313; border: 1px solid #ac915a; }");
        connect(slider,&QSlider::valueChanged,this,[this,i](int value) {
            if (!sliders_[i]->isEnabled()) {
                QSignalBlocker blocker(sliders_[i]);sliders_[i]->setValue(i<13?lobby_.values[i]:lobby_.players[i-13].handicap);return;
            }
            // Snap pointer input as well as keyboard input to the configured step.
            const int snapped=qBound(minimum_[i],minimum_[i]+((value-minimum_[i]+step_[i]/2)/step_[i])*step_[i],maximum_[i]);
            QSignalBlocker blocker(sliders_[i]);sliders_[i]->setValue(snapped);
            if (i<13) {lobby_.values[i]=snapped;labels_[27+i]->setText(QString::number(snapped));}
            else {lobby_.players[i-13].handicap=snapped;labels_[i-9]->setText(QString::number(snapped));}
            arrange();
        });
    }
    auto button=[this](const QString& name,const QString& text) {
        auto* result=new QPushButton(text,this);result->setObjectName(name);result->installEventFilter(this);
        result->setStyleSheet(mnm::ui::menuButtonStyle());result->setCursor(Qt::PointingHandCursor);return result;
    };
    for (int i=0;i<4;++i) {
        portraits_[i]=button(QString("multiplayerLobbyPortrait%1").arg(i),"+");
        colours_[i]=button(QString("multiplayerLobbyColour%1").arg(i),"—");
        portraits_[i]->setAccessibleName(QString("Choose player %1 portrait").arg(i+1));
        colours_[i]->setAccessibleName(QString("Choose player %1 colour").arg(i+1));
        connect(portraits_[i],&QPushButton::clicked,this,[this,i]{emit playerChangeRequested(i);});
        connect(colours_[i],&QPushButton::clicked,this,[this,i]{emit colourChangeRequested(i);});
    }
    for (int i=0;i<3;++i) {
        remove_[i]=button(QString("multiplayerLobbyRemove%1").arg(i+1),"×");
        remove_[i]->setAccessibleName(QString("Remove player %1").arg(i+2));
        connect(remove_[i],&QPushButton::clicked,this,[this,i]{emit playerRemovalRequested(i+1);});
    }
    cancel_=button("multiplayerLobbyCancel","Cancel");start_=button("multiplayerLobbyStart","Start");map_=button("multiplayerLobbyMap","Map");
    start_->setCheckable(mode_==Mode::Join);
    start_->setStyleSheet(mnm::ui::menuButtonStyle()+"QPushButton:checked { color: #fff5d6; background: #302719; }");
    chat_=new QPlainTextEdit(this);chat_->setObjectName("multiplayerLobbyChat");chat_->setReadOnly(true);
    chat_->setMaximumBlockCount(100);chat_->setStyleSheet("color: #3e2313; background: transparent; border: 1px solid #ac915a;");
    chat_->setAccessibleName("Lobby messages");chat_->installEventFilter(this);
    composer_=new QLineEdit(this);composer_->setObjectName("multiplayerLobbyComposer");composer_->setMaxLength(256);
    composer_->setAccessibleName("Chat message; Enter to send");composer_->installEventFilter(this);
    composer_->setStyleSheet("color: #3e2313; background: transparent; border: 1px solid #ac915a;");
    connect(cancel_,&QPushButton::clicked,this,&MultiplayerLobbyWidget::cancelled);
    connect(start_,&QPushButton::clicked,this,&MultiplayerLobbyWidget::start);
    connect(map_,&QPushButton::clicked,this,&MultiplayerLobbyWidget::mapRequested);
    QWidget* previous=portraits_[0];
    for (int i=0;i<4;++i) {
        if (i) QWidget::setTabOrder(previous,portraits_[i]);
        QWidget::setTabOrder(portraits_[i],colours_[i]);QWidget::setTabOrder(colours_[i],sliders_[13+i]);previous=sliders_[13+i];
        if (i>=1) {QWidget::setTabOrder(previous,remove_[i-1]);previous=remove_[i-1];}
    }
    QWidget::setTabOrder(previous,map_);previous=map_;
    for (auto* slider:sliders_) { if (slider==sliders_[13]) break;QWidget::setTabOrder(previous,slider);previous=slider; }
    QWidget::setTabOrder(previous,start_);QWidget::setTabOrder(start_,cancel_);QWidget::setTabOrder(cancel_,chat_);QWidget::setTabOrder(chat_,composer_);populate();
}
bool MultiplayerLobbyWidget::loadAssets(const QString& root,QString* error) {
    if (error) error->clear();
    try {
        const QString directory="Interface/MultiplayerBattleSetup";
        const auto assets=mnm::ui::loadMenuAssets(root,directory,"screen (MultiPlayer Battle Setup).cfg");
        const auto host=mnm::ui::loadMenuLayout(root,directory,"screen (MultiPlayer Battle Create).cfg");
        const auto modeLayout=mode_==Mode::Host?host:mnm::ui::loadMenuLayout(root,directory,"screen (Multiplayer Battle Join).cfg");
        std::array<QRect,40> textRects;std::array<QString,40> texts;std::array<Qt::Alignment,40> alignments;
        for (int i=0;i<40;++i) {
            const auto entry=assets.layout.value(QString("TEXT_%1").arg(i+1));textRects[i]=mnm::ui::rectangle(entry.value("Rect2"));
            if (entry.value("Font")!="SMALL") throw std::runtime_error("Invalid battle setup text font");
            const auto flags=entry.value("Textflags");
            if (flags!="LEFT" && flags!="MIDDLE") throw std::runtime_error("Invalid battle setup text alignment");
            alignments[i]=flags=="LEFT"?Qt::AlignLeft|Qt::AlignVCenter:Qt::AlignCenter;
            texts[i]=labelText(assets.strings,entry.value("Text"));
        }
        std::array<QRect,17> sliderRects;std::array<int,17> minimum,maximum,step;
        for (int i=0;i<17;++i) {
            const auto entry=i<13?host.value(QString("SLIDERBAR_%1").arg(i+1)):assets.layout.value(QString("SLIDERBAR_%1").arg(i-12));sliderRects[i]=mnm::ui::rectangle(entry.value("Rect2"));
            minimum[i]=number(entry.value("minValue"));maximum[i]=number(entry.value("maxValue"));step[i]=number(entry.value("step"));
            if (minimum[i]>=maximum[i] || step[i]<1 || step[i]>maximum[i]-minimum[i] || (maximum[i]-minimum[i])%step[i])
                throw std::runtime_error("Invalid battle setup slider range or step");
        }
        std::array<QRect,4> portraits,colours;std::array<QRect,3> remove;
        for (int i=0;i<4;++i) {
            portraits[i]=mnm::ui::rectangle(assets.layout.value(QString("STANDARDBUTTON_%1").arg(i+1)).value("Rect2"));
            colours[i]=mnm::ui::rectangle(assets.layout.value(QString("STANDARDBUTTON_%1").arg(i+5)).value("Rect2"));
            if (i>=1) remove[i-1]=mnm::ui::rectangle(host.value(QString("STANDARDBUTTON_%1").arg(i)).value("Rect2"));
        }
        std::array<QRect,3> buttons;std::array<QString,3> buttonTexts;
        for (int i=0;i<3;++i) {
            const auto entry=i==0?assets.layout.value("TEXTBUTTON_1"):i==1?modeLayout.value(mode_==Mode::Host?"TEXTBUTTON_2":"TEXTBUTTON_1"):host.value("TEXTBUTTON_1");buttons[i]=mnm::ui::rectangle(entry.value("Rect2"));
            if (entry.value("Font")!="SMALL") throw std::runtime_error("Invalid battle setup button font");
            buttonTexts[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
        }
        const auto chat=assets.layout.value("MESSAGELISTBOX_1"),composer=assets.layout.value("EDITBOX_1");
        const auto chatRect=mnm::ui::rectangle(chat.value("Rect2")),composerRect=mnm::ui::rectangle(composer.value("Rect2"));
        if (chat.value("Font")!="SMALL" || composer.value("Font")!="SMALL") throw std::runtime_error("Invalid lobby chat font");
        if (!validLobby(lobby(),minimum,maximum,step)) throw std::runtime_error("Battle setup outside configured slider ranges");
        chatRectangle_=chatRect;composerRectangle_=composerRect;
        background_=assets.background;labelRectangles_=textRects;sliderRectangles_=sliderRects;minimum_=minimum;maximum_=maximum;step_=step;
        portraitRectangles_=portraits;colourRectangles_=colours;removeRectangles_=remove;noPlayer_=texts[0];
        cancelRectangle_=buttons[0];startRectangle_=buttons[1];mapRectangle_=buttons[2];
        cancel_->setText(buttonTexts[0]);start_->setText(buttonTexts[1]);map_->setText(buttonTexts[2]);
        for (int i=0;i<40;++i) {labels_[i]->setText(texts[i]);labels_[i]->setAlignment(alignments[i]);labels_[i]->setVisible(!texts[i].isEmpty());}
        for (int i=0;i<17;++i) {
            QSignalBlocker blocker(sliders_[i]);sliders_[i]->setRange(minimum[i],maximum[i]);sliders_[i]->setSingleStep(step[i]);sliders_[i]->setPageStep(step[i]);
            sliders_[i]->setAccessibleName(i<13?texts[14+i]:QString("Player %1 handicap").arg(i-12));
        }
        populate();arrange();update();return true;
    } catch (const std::exception& failure) {if (error) *error=QString::fromUtf8(failure.what());return false;}
}
bool MultiplayerLobbyWidget::validLobby(const Lobby& value,const std::array<int,17>& minimum,const std::array<int,17>& maximum,const std::array<int,17>& step) const {
    if (value.mapId.isEmpty()!=value.mapName.isEmpty() || value.sessionId.isEmpty()!=value.gameName.isEmpty() ||
        value.localSlot<0 || value.localSlot>3 || (mode_==Mode::Host && (value.localSlot!=0 || value.localReady))) return false;
    for (int i=0;i<17;++i) {
        const int n=i<13?value.values[i]:value.players[i-13].handicap;
        if (n<minimum[i] || n>maximum[i] || (n-minimum[i])%step[i]) return false;
    }
    for (const auto& player:value.players)
        if (player.active && (player.name.trimmed().isEmpty() || player.name.size()>64 || player.name.contains('\n') || player.name.contains('\r') || player.name.contains(QChar(0)) || player.portraitId.isEmpty() || player.portraitText.isEmpty() || player.colourId.isEmpty() || player.colourText.isEmpty())) return false;
    return true;
}
bool MultiplayerLobbyWidget::setLobby(const Lobby& value,QString* error) {
    if (error) error->clear();
    if (!validLobby(value,minimum_,maximum_,step_)) {if (error) *error="Invalid battle setup model.";return false;}
    lobby_=value;populate();arrange();return true;
}
MultiplayerLobbyWidget::Lobby MultiplayerLobbyWidget::lobby() const { return lobby_; }
void MultiplayerLobbyWidget::populate() {
    for (int i=0;i<4;++i) {
        const auto& player=lobby_.players[i];labels_[i]->setText(player.active?player.name:noPlayer_);
        labels_[4+i]->setText(QString::number(player.handicap));labels_[4+i]->setVisible(player.active);
        portraits_[i]->setText(player.active?player.portraitText:"+");colours_[i]->setText(player.active?player.colourText:"—");
        portraits_[i]->setToolTip(player.active?player.name:"Choose sample player");colours_[i]->setToolTip(player.colourText);
        portraits_[i]->setEnabled(player.active && i==lobby_.localSlot);
        colours_[i]->setEnabled(player.active && i==lobby_.localSlot);
        sliders_[13+i]->setEnabled(player.active && (mode_==Mode::Host || i==lobby_.localSlot));
        labels_[8+i]->setVisible(false); // Shared caption overlaps the native handicap slider.
        if (i>=1) {remove_[i-1]->setVisible(mode_==Mode::Host);remove_[i-1]->setEnabled(mode_==Mode::Host && player.active && i!=lobby_.localSlot);}
    }
    labels_[12]->setText(lobby_.gameName);labels_[12]->setVisible(!lobby_.gameName.isEmpty());
    labels_[13]->setText(lobby_.mapName);labels_[13]->setVisible(!lobby_.mapName.isEmpty());
    for (int i=0;i<17;++i) {QSignalBlocker blocker(sliders_[i]);sliders_[i]->setValue(i<13?lobby_.values[i]:lobby_.players[i-13].handicap);}
    for (int i=0;i<13;++i) labels_[27+i]->setText(QString::number(lobby_.values[i]));
    bool opponent=false;for (int i=1;i<4;++i) opponent|=lobby_.players[i].active;
    for (int i=0;i<13;++i) {sliders_[i]->setVisible(mode_==Mode::Host);sliders_[i]->setEnabled(mode_==Mode::Host);}
    map_->setVisible(mode_==Mode::Host);map_->setEnabled(mode_==Mode::Host);
    const bool local=lobby_.players[lobby_.localSlot].active;
    start_->setEnabled(!lobby_.sessionId.isEmpty() && local && (mode_==Mode::Join || (!lobby_.mapId.isEmpty() && opponent)));
    start_->setChecked(mode_==Mode::Join && lobby_.localReady);
    composer_->setEnabled(!lobby_.sessionId.isEmpty() && local);
}
void MultiplayerLobbyWidget::start() {
    if (!start_->isEnabled()) return;
    if (mode_==Mode::Host) emit startRequested(lobby());
    else {lobby_.localReady=start_->isChecked();emit readyRequested(lobby_.localReady);}
}
void MultiplayerLobbyWidget::focusFirstControl() {composer_->setFocus(Qt::OtherFocusReason);}
QRect MultiplayerLobbyWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void MultiplayerLobbyWidget::arrange() {
    const auto canvas=contentRect();const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    QFont font("serif");font.setBold(true);font.setPixelSize(qMax(10,qRound(20*scale)));
    for (int i=0;i<40;++i) {
        auto* label=labels_[i];label->setGeometry(map(labelRectangles_[i]));auto fitted=font;
        const int available=label->width()-(i>=14 && i<27?qRound(6*scale):0);
        while (fitted.pixelSize()>10 && QFontMetrics(fitted).horizontalAdvance(label->text())>available) fitted.setPixelSize(fitted.pixelSize()-1);
        label->setFont(fitted);
    }
    for (int i=0;i<17;++i) sliders_[i]->setGeometry(map(sliderRectangles_[i]));
    for (int i=0;i<4;++i) {
        portraits_[i]->setGeometry(map(portraitRectangles_[i]));colours_[i]->setGeometry(map(colourRectangles_[i]));
        auto compact=font;compact.setPixelSize(qMax(10,qRound(12*scale)));portraits_[i]->setFont(compact);colours_[i]->setFont(compact);
        if (i>=1) {remove_[i-1]->setGeometry(map(removeRectangles_[i-1]));remove_[i-1]->setFont(font);}
    }
    cancel_->setGeometry(map(cancelRectangle_));start_->setGeometry(map(startRectangle_));map_->setGeometry(map(mapRectangle_));
    for (auto* button:{cancel_,start_,map_}) button->setFont(font);
    chat_->setGeometry(map(chatRectangle_));composer_->setGeometry(map(composerRectangle_));
    auto chatFont=font;chatFont.setPixelSize(qMax(10,qRound(16*scale)));chat_->setFont(chatFont);composer_->setFont(chatFont);
}
void MultiplayerLobbyWidget::resizeEvent(QResizeEvent* event) {QWidget::resizeEvent(event);arrange();}
bool MultiplayerLobbyWidget::eventFilter(QObject* watched,QEvent* event) {
    if (event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Escape || key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept();if (!key->isAutoRepeat()) {
                if (key->key()==Qt::Key_Escape) emit cancelled();
                else if (watched==composer_) sendChat();
                else if (watched==chat_) { /* Reading chat does not start a game. */ }
                else if (auto* button=qobject_cast<QPushButton*>(watched)) button->click();
                else if (mode_==Mode::Host) start();
            }return true;
        }
    }return QWidget::eventFilter(watched,event);
}
void MultiplayerLobbyWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape || event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept();if (!event->isAutoRepeat()) {if (event->key()==Qt::Key_Escape) emit cancelled();else if (mode_==Mode::Host) start();}return;
    }QWidget::keyPressEvent(event);
}
void MultiplayerLobbyWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else {painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.drawImage(contentRect(),background_);}
}

namespace {
bool chatPart(const QString& text,int limit) {
    return !text.trimmed().isEmpty() && text.size()<=limit && !text.contains('\n') && !text.contains('\r') && !text.contains(QChar(0));
}
}
bool MultiplayerLobbyWidget::setMessages(const QVector<Message>& messages,QString* error) {
    if (error) error->clear();
    if (messages.size()>100) {if (error) *error="Too many lobby messages.";return false;}
    for (const auto& message:messages) if (!chatPart(message.sender,64) || !chatPart(message.text,256)) {
        if (error) *error="Invalid lobby message.";
        return false;
    }
    messages_=messages;displayMessages();return true;
}
bool MultiplayerLobbyWidget::appendMessage(const Message& message,QString* error) {
    auto messages=messages_;messages.append(message);if (messages.size()>100) messages.removeFirst();
    return setMessages(messages,error);
}
void MultiplayerLobbyWidget::displayMessages() {
    QStringList lines;for (const auto& message:messages_) lines.append(message.sender+": "+message.text);
    chat_->setPlainText(lines.join('\n'));auto cursor=chat_->textCursor();cursor.movePosition(QTextCursor::End);chat_->setTextCursor(cursor);
}
void MultiplayerLobbyWidget::clearChat() {messages_.clear();chat_->clear();composer_->clear();}
void MultiplayerLobbyWidget::sendChat() {
    const auto text=composer_->text().trimmed();
    if (!composer_->isEnabled() || !chatPart(text,256)) return;
    composer_->clear();emit chatRequested(text);
}

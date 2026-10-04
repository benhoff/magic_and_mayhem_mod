#include "single_player_battle_widget.hpp"
#include "menu_assets.hpp"
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
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
SinglePlayerBattleWidget::SinglePlayerBattleWidget(QWidget* parent):QWidget(parent) {
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Single Player Battle preview");
    for (int i=0;i<40;++i) {
        labels_[i]=new QLabel(this);labels_[i]->setObjectName(QString("singlePlayerText%1").arg(i+1));
        labels_[i]->setTextFormat(Qt::PlainText);labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");
        labels_[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    for (int i=0;i<17;++i) {
        auto* slider=sliders_[i]=new QSlider(Qt::Horizontal,this);
        slider->setObjectName(QString("singlePlayerSlider%1").arg(i+1));slider->installEventFilter(this);
        slider->setRange(minimum_[i],maximum_[i]);slider->setSingleStep(step_[i]);slider->setPageStep(step_[i]);
        slider->setStyleSheet("QSlider::groove:horizontal { height: 3px; background: #ac915a; }"
            "QSlider::handle:horizontal { width: 12px; margin: -6px 0; background: #3e2313; border: 1px solid #ac915a; }");
        connect(slider,&QSlider::valueChanged,this,[this,i](int value) {
            // Snap pointer input as well as keyboard input to the configured step.
            const int snapped=qBound(minimum_[i],minimum_[i]+((value-minimum_[i]+step_[i]/2)/step_[i])*step_[i],maximum_[i]);
            QSignalBlocker blocker(sliders_[i]);sliders_[i]->setValue(snapped);
            if (i<13) {setup_.values[i]=snapped;labels_[27+i]->setText(QString::number(snapped));}
            else {setup_.players[i-13].handicap=snapped;labels_[i-9]->setText(QString::number(snapped));}
            arrange();
        });
    }
    auto button=[this](const QString& name,const QString& text) {
        auto* result=new QPushButton(text,this);result->setObjectName(name);result->installEventFilter(this);
        result->setStyleSheet(mnm::ui::menuButtonStyle());result->setCursor(Qt::PointingHandCursor);return result;
    };
    for (int i=0;i<4;++i) {
        portraits_[i]=button(QString("singlePlayerPortrait%1").arg(i),"+");
        colours_[i]=button(QString("singlePlayerColour%1").arg(i),"—");
        portraits_[i]->setAccessibleName(QString("Choose player %1 portrait").arg(i+1));
        colours_[i]->setAccessibleName(QString("Choose player %1 colour").arg(i+1));
        connect(portraits_[i],&QPushButton::clicked,this,[this,i]{emit playerChangeRequested(i);});
        connect(colours_[i],&QPushButton::clicked,this,[this,i]{emit colourChangeRequested(i);});
    }
    for (int i=0;i<2;++i) {
        remove_[i]=button(QString("singlePlayerRemove%1").arg(i+2),"×");
        remove_[i]->setAccessibleName(QString("Remove player %1").arg(i+3));
        connect(remove_[i],&QPushButton::clicked,this,[this,i]{emit playerRemovalRequested(i+2);});
    }
    cancel_=button("singlePlayerCancel","Cancel");start_=button("singlePlayerStart","Start");map_=button("singlePlayerMap","Map");
    connect(cancel_,&QPushButton::clicked,this,&SinglePlayerBattleWidget::cancelled);
    connect(start_,&QPushButton::clicked,this,&SinglePlayerBattleWidget::start);
    connect(map_,&QPushButton::clicked,this,&SinglePlayerBattleWidget::mapRequested);
    QWidget* previous=portraits_[0];
    for (int i=0;i<4;++i) {
        if (i) QWidget::setTabOrder(previous,portraits_[i]);
        QWidget::setTabOrder(portraits_[i],colours_[i]);QWidget::setTabOrder(colours_[i],sliders_[13+i]);previous=sliders_[13+i];
        if (i>=2) {QWidget::setTabOrder(previous,remove_[i-2]);previous=remove_[i-2];}
    }
    QWidget::setTabOrder(previous,map_);previous=map_;
    for (auto* slider:sliders_) { if (slider==sliders_[13]) break;QWidget::setTabOrder(previous,slider);previous=slider; }
    QWidget::setTabOrder(previous,start_);QWidget::setTabOrder(start_,cancel_);populate();
}
bool SinglePlayerBattleWidget::loadAssets(const QString& root,QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/SinglePlayerBattle","screen (Single Player Battle).cfg");
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
            const auto entry=assets.layout.value(QString("SLIDERBAR_%1").arg(i+1));sliderRects[i]=mnm::ui::rectangle(entry.value("Rect2"));
            minimum[i]=number(entry.value("minValue"));maximum[i]=number(entry.value("maxValue"));step[i]=number(entry.value("step"));
            if (minimum[i]>=maximum[i] || step[i]<1 || step[i]>maximum[i]-minimum[i] || (maximum[i]-minimum[i])%step[i])
                throw std::runtime_error("Invalid battle setup slider range or step");
        }
        std::array<QRect,4> portraits,colours;std::array<QRect,2> remove;
        for (int i=0;i<4;++i) {
            portraits[i]=mnm::ui::rectangle(assets.layout.value(i==0?"STANDARDBUTTON_1":QString("PICTUREBOX_%1").arg(i)).value("Rect2"));
            colours[i]=mnm::ui::rectangle(assets.layout.value(i==0?"STANDARDBUTTON_2":QString("PICTUREBOX_%1").arg(i+3)).value("Rect2"));
            if (i>=2) remove[i-2]=mnm::ui::rectangle(assets.layout.value(QString("STANDARDBUTTON_%1").arg(i+1)).value("Rect2"));
        }
        std::array<QRect,3> buttons;std::array<QString,3> buttonTexts;
        for (int i=0;i<3;++i) {
            const auto entry=assets.layout.value(QString("TEXTBUTTON_%1").arg(i+1));buttons[i]=mnm::ui::rectangle(entry.value("Rect2"));
            if (entry.value("Font")!="SMALL") throw std::runtime_error("Invalid battle setup button font");
            buttonTexts[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
        }
        if (!validSetup(setup(),minimum,maximum,step)) throw std::runtime_error("Battle setup outside configured slider ranges");
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
bool SinglePlayerBattleWidget::validSetup(const Setup& value,const std::array<int,17>& minimum,const std::array<int,17>& maximum,const std::array<int,17>& step) const {
    if (value.mapId.isEmpty()!=value.mapName.isEmpty()) return false;
    for (int i=0;i<17;++i) {
        const int n=i<13?value.values[i]:value.players[i-13].handicap;
        if (n<minimum[i] || n>maximum[i] || (n-minimum[i])%step[i]) return false;
    }
    for (const auto& player:value.players)
        if (player.active && (player.name.trimmed().isEmpty() || player.portraitId.isEmpty() || player.portraitText.isEmpty() || player.colourId.isEmpty() || player.colourText.isEmpty())) return false;
    return true;
}
bool SinglePlayerBattleWidget::setSetup(const Setup& value,QString* error) {
    if (error) error->clear();
    if (!validSetup(value,minimum_,maximum_,step_)) {if (error) *error="Invalid battle setup model.";return false;}
    setup_=value;populate();arrange();return true;
}
SinglePlayerBattleWidget::Setup SinglePlayerBattleWidget::setup() const { return setup_; }
void SinglePlayerBattleWidget::populate() {
    for (int i=0;i<4;++i) {
        const auto& player=setup_.players[i];labels_[i]->setText(player.active?player.name:noPlayer_);
        labels_[4+i]->setText(QString::number(player.handicap));labels_[4+i]->setVisible(player.active);
        portraits_[i]->setText(player.active?player.portraitText:"+");colours_[i]->setText(player.active?player.colourText:"—");
        portraits_[i]->setToolTip(player.active?player.name:"Choose sample player");colours_[i]->setToolTip(player.colourText);
        colours_[i]->setEnabled(player.active);sliders_[13+i]->setEnabled(player.active);
        if (i>=2) remove_[i-2]->setEnabled(player.active);
    }
    labels_[13]->setText(setup_.mapName);labels_[13]->setVisible(!setup_.mapName.isEmpty());
    for (int i=0;i<17;++i) {QSignalBlocker blocker(sliders_[i]);sliders_[i]->setValue(i<13?setup_.values[i]:setup_.players[i-13].handicap);}
    for (int i=0;i<13;++i) labels_[27+i]->setText(QString::number(setup_.values[i]));
    bool opponent=false;for (int i=1;i<4;++i) opponent|=setup_.players[i].active;
    start_->setEnabled(!setup_.mapId.isEmpty() && setup_.players[0].active && opponent);
}
void SinglePlayerBattleWidget::start() {if (start_->isEnabled()) emit startRequested(setup());}
void SinglePlayerBattleWidget::focusFirstControl() {portraits_[0]->setFocus(Qt::OtherFocusReason);}
QRect SinglePlayerBattleWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void SinglePlayerBattleWidget::arrange() {
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
        if (i>=2) {remove_[i-2]->setGeometry(map(removeRectangles_[i-2]));remove_[i-2]->setFont(font);}
    }
    cancel_->setGeometry(map(cancelRectangle_));start_->setGeometry(map(startRectangle_));map_->setGeometry(map(mapRectangle_));
    for (auto* button:{cancel_,start_,map_}) button->setFont(font);
}
void SinglePlayerBattleWidget::resizeEvent(QResizeEvent* event) {QWidget::resizeEvent(event);arrange();}
bool SinglePlayerBattleWidget::eventFilter(QObject* watched,QEvent* event) {
    if (event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Escape || key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept();if (!key->isAutoRepeat()) {
                if (key->key()==Qt::Key_Escape) emit cancelled();
                else if (auto* button=qobject_cast<QPushButton*>(watched)) button->click();
                else start();
            }return true;
        }
    }return QWidget::eventFilter(watched,event);
}
void SinglePlayerBattleWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape || event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept();if (!event->isAutoRepeat()) {if (event->key()==Qt::Key_Escape) emit cancelled();else start();}return;
    }QWidget::keyPressEvent(event);
}
void SinglePlayerBattleWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else {painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.drawImage(contentRect(),background_);}
}

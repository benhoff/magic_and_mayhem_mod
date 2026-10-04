#include "preferences_widget.hpp"
#include "menu_assets.hpp"
#include <QButtonGroup>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <stdexcept>
namespace {
int alignment(const QString& value) {
    if (value=="LEFT") return Qt::AlignLeft | Qt::AlignVCenter;
    if (value=="CENTRE") return Qt::AlignCenter;
    throw std::runtime_error("Invalid Preferences text alignment");
}
int number(const QString& value) {
    bool ok=false; const int result=value.toInt(&ok);
    if (!ok || result < -100000 || result > 100000) throw std::runtime_error("Invalid Preferences slider bound");
    return result;
}
}
PreferencesWidget::PreferencesWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240); setWindowTitle("Magic & Mayhem — Preferences preview");
    for (int i=0;i<11;++i) {
        labels_[i]=new QLabel(this); labels_[i]->setObjectName(QString("preferencesText%1").arg(i+1));
        labels_[i]->setTextFormat(Qt::PlainText); labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");
    }
    for (auto& group:groups_) group=new QButtonGroup(this);
    for (int i=0;i<12;++i) {
        radios_[i]=new QRadioButton(this); radios_[i]->setObjectName(QString("preferencesRadio%1").arg(i+1));
        radios_[i]->setStyleSheet("QRadioButton { color: #3e2313; background: transparent; }"
            "QRadioButton::indicator { width: 14px; height: 14px; border: 1px solid #ac915a; border-radius: 7px; background: transparent; }"
            "QRadioButton::indicator:checked { background: #3e2313; }");
        const int group=i<2?0:i<4?1:i<7?2:i<10?3:4;
        const int first=group==0?0:group==1?2:group==2?4:group==3?7:10;
        groups_[group]->addButton(radios_[i],i-first); radios_[i]->installEventFilter(this);
    }
    for (int i=0;i<2;++i) {
        sliders_[i]=new QSlider(Qt::Horizontal,this); sliders_[i]->setObjectName(QString("preferencesSlider%1").arg(i+1));
        sliders_[i]->setRange(minimum_[i],maximum_[i]); sliders_[i]->setSingleStep(100); sliders_[i]->setPageStep(500);
        sliders_[i]->setAccessibleName(i==0?"Music level":"Sound effects level"); sliders_[i]->installEventFilter(this);
        sliders_[i]->setStyleSheet("QSlider::groove:horizontal { height: 3px; background: #ac915a; }"
            "QSlider::handle:horizontal { width: 14px; margin: -6px 0; background: #3e2313; border: 1px solid #ac915a; }");
    }
    ok_=new QPushButton("OK",this); ok_->setObjectName("preferencesOk");
    cancel_=new QPushButton("Cancel",this); cancel_->setObjectName("preferencesCancel");
    for (auto* button:{ok_,cancel_}) { button->setStyleSheet(mnm::ui::menuButtonStyle()); button->setCursor(Qt::PointingHandCursor); button->installEventFilter(this); }
    connect(ok_,&QPushButton::clicked,this,&PreferencesWidget::apply);
    connect(cancel_,&QPushButton::clicked,this,&PreferencesWidget::cancel);
    QWidget::setTabOrder(sliders_[0],sliders_[1]); QWidget::setTabOrder(sliders_[1],radios_[0]);
    for (int i=0;i<11;++i) QWidget::setTabOrder(radios_[i],radios_[i+1]);
    QWidget::setTabOrder(radios_[11],ok_); QWidget::setTabOrder(ok_,cancel_);
    populate(accepted_);
}
bool PreferencesWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/BattleOptionsScreen","screen (Battle Options).cfg");
        std::array<QRect,11> labels; std::array<QString,11> texts; std::array<int,11> alignments;
        std::array<QRect,12> radios; std::array<QString,12> choices;
        std::array<QRect,2> sliders; std::array<int,2> minimum,maximum;
        for (int i=0;i<11;++i) {
            const auto entry=assets.layout.value(QString("TEXT_%1").arg(i+1));
            labels[i]=mnm::ui::rectangle(entry.value("Rect2")); alignments[i]=alignment(entry.value("TextFlags"));
            if (entry.value("Font")!=(i==0?"LARGE":"SMALL")) throw std::runtime_error("Invalid Preferences heading font");
            if (!entry.value("Text").isEmpty()) texts[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
        }
        for (int i=0;i<12;++i) {
            const auto entry=assets.layout.value(QString("RADIOBUTTON_%1").arg(i+1));
            radios[i]=mnm::ui::rectangle(entry.value("Rect2")); choices[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
            if (entry.value("Font")!="SMALL") throw std::runtime_error("Invalid Preferences radio font");
        }
        for (int i=0;i<2;++i) {
            const auto entry=assets.layout.value(QString("SLIDERBAR_%1").arg(i+1));
            sliders[i]=mnm::ui::rectangle(entry.value("Rect2")); minimum[i]=number(entry.value("minValue")); maximum[i]=number(entry.value("maxValue"));
            if (minimum[i]>=maximum[i]) throw std::runtime_error("Invalid Preferences slider range");
        }
        const auto ok=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto okRect=mnm::ui::rectangle(ok.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        const auto okText=mnm::ui::textLabel(assets.strings,ok.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (ok.value("Font")!="LARGE" || cancel.value("Font")!="LARGE") throw std::runtime_error("Invalid Preferences button font");
        const auto draft=draftSettings();
        if (!validSettings(accepted_,minimum,maximum) || !validSettings(draft,minimum,maximum)) throw std::runtime_error("Preferences state outside configured slider range");
        background_=assets.background; labelRectangles_=labels; radioRectangles_=radios; sliderRectangles_=sliders; minimum_=minimum; maximum_=maximum;
        okRectangle_=okRect; cancelRectangle_=cancelRect; ok_->setText(okText); cancel_->setText(cancelText);
        for (int i=0;i<11;++i) { labels_[i]->setText(texts[i]); labels_[i]->setAlignment(Qt::Alignment(alignments[i])); labels_[i]->setVisible(!texts[i].isEmpty()); }
        for (int i=0;i<12;++i) radios_[i]->setText(choices[i]);
        for (int i=0;i<2;++i) sliders_[i]->setRange(minimum[i],maximum[i]);
        populate(draft); arrange(); update(); return true;
    } catch (const std::exception& failure) { if (error) *error=QString::fromUtf8(failure.what()); return false; }
}
bool PreferencesWidget::validSettings(const Settings& settings, const std::array<int,2>& minimum, const std::array<int,2>& maximum) const {
    return settings.musicLevel>=minimum[0] && settings.musicLevel<=maximum[0] && settings.soundLevel>=minimum[1] && settings.soundLevel<=maximum[1] &&
        int(settings.resolution)>=0 && int(settings.resolution)<=1 && int(settings.animation)>=0 && int(settings.animation)<=1 &&
        int(settings.dialogueSpeed)>=0 && int(settings.dialogueSpeed)<=2 && int(settings.gameSpeed)>=0 && int(settings.gameSpeed)<=2;
}
bool PreferencesWidget::setSettings(const Settings& settings, QString* error) {
    if (error) error->clear();
    if (!validSettings(settings,minimum_,maximum_)) { if (error) *error="Invalid Preferences snapshot."; return false; }
    accepted_=settings; populate(settings); return true;
}
void PreferencesWidget::populate(const Settings& settings) {
    sliders_[0]->setValue(settings.musicLevel); sliders_[1]->setValue(settings.soundLevel);
    groups_[0]->button(int(settings.resolution))->setChecked(true); groups_[1]->button(int(settings.animation))->setChecked(true);
    groups_[2]->button(int(settings.dialogueSpeed))->setChecked(true); groups_[3]->button(int(settings.gameSpeed))->setChecked(true);
    groups_[4]->button(settings.borderPicture?0:1)->setChecked(true);
}
PreferencesWidget::Settings PreferencesWidget::draftSettings() const {
    Settings settings; settings.musicLevel=sliders_[0]->value(); settings.soundLevel=sliders_[1]->value();
    settings.resolution=Resolution(groups_[0]->checkedId()); settings.animation=Animation(groups_[1]->checkedId());
    settings.dialogueSpeed=Speed(groups_[2]->checkedId()); settings.gameSpeed=Speed(groups_[3]->checkedId()); settings.borderPicture=groups_[4]->checkedId()==0;
    return settings;
}
void PreferencesWidget::apply() { accepted_=draftSettings(); emit settingsApplied(accepted_); }
void PreferencesWidget::cancel() { populate(accepted_); emit cancelled(); }
void PreferencesWidget::focusFirstControl() { sliders_[0]->setFocus(Qt::OtherFocusReason); }
QRect PreferencesWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void PreferencesWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    QFont font("serif"); font.setBold(true); font.setPixelSize(qMax(10,qRound(20*scale)));
    for (int i=0;i<11;++i) { labels_[i]->setGeometry(map(labelRectangles_[i])); labels_[i]->setFont(font); }
    for (int i=0;i<12;++i) { radios_[i]->setGeometry(map(radioRectangles_[i])); radios_[i]->setFont(font); }
    for (int i=0;i<2;++i) sliders_[i]->setGeometry(map(sliderRectangles_[i]));
    font.setPixelSize(qMax(10,qRound(26*scale))); labels_[0]->setFont(font); ok_->setFont(font); cancel_->setFont(font);
    ok_->setGeometry(map(okRectangle_)); cancel_->setGeometry(map(cancelRectangle_));
}
void PreferencesWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
bool PreferencesWidget::eventFilter(QObject* watched, QEvent* event) {
    if (event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Escape || key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept(); if (!key->isAutoRepeat()) { if (key->key()==Qt::Key_Escape || watched==cancel_) cancel(); else apply(); } return true;
        }
    }
    return QWidget::eventFilter(watched,event);
}
void PreferencesWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape || event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept(); if (!event->isAutoRepeat()) { if (event->key()==Qt::Key_Escape) cancel(); else apply(); } return;
    }
    QWidget::keyPressEvent(event);
}
void PreferencesWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

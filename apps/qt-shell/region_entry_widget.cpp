#include "region_entry_widget.hpp"
#include "menu_assets.hpp"
#include "menu_sprites.hpp"
#include <QButtonGroup>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <stdexcept>
namespace {
class DifficultyRadio final : public QRadioButton {
public:
    explicit DifficultyRadio(QWidget* parent):QRadioButton(parent) {}
protected:
    bool hitButton(const QPoint& point) const override {return rect().contains(point);}
};
QString artworkName(const RegionEntryWidget::Region& region) {
    const std::array<QString,3> realms{"Celtic","Greek","Medieval"};
    return QString("%1_Region_%2.JPG").arg(realms[int(region.artworkRealm)]).arg(region.artworkNumber,2,10,QLatin1Char('0'));
}
}
RegionEntryWidget::RegionEntryWidget(QWidget* parent):QWidget(parent) {
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Region Entry preview");
    heading_=new QLabel(defaultHeading_,this);heading_->setObjectName("regionEntryHeading");heading_->setTextFormat(Qt::PlainText);
    heading_->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);heading_->setStyleSheet("color: #3e2313; background: transparent;");
    difficulties_=new QButtonGroup(this);
    for (int i=0;i<4;++i) {
        radios_[i]=new DifficultyRadio(this);radios_[i]->setObjectName(QString("regionEntryDifficulty%1").arg(i));
        radios_[i]->setCursor(Qt::PointingHandCursor);
        difficulties_->addButton(radios_[i],i);radios_[i]->installEventFilter(this);
        connect(radios_[i],&QRadioButton::toggled,this,[this,i](bool checked){if (checked) region_.difficulty=Difficulty(i);});
    }
    auto button=[this](const QString& name,const QString& text) {
        auto* result=new mnm::ui::SpriteButton(text,this);result->setObjectName(name);result->installEventFilter(this);
        result->setStyleSheet(mnm::ui::menuButtonStyle());result->setCursor(Qt::PointingHandCursor);return result;
    };
    const std::array<QString,3> labels{"Grimoire","Spellbox","Character"};
    const std::array<QString,3> initials{"G","S","C"};
    for (int i=0;i<3;++i) {
        auxiliary_[i]=button(QString("regionEntryAuxiliary%1").arg(i),initials[i]);
        auxiliary_[i]->setAccessibleName(labels[i]);auxiliary_[i]->setToolTip(labels[i]);
        connect(auxiliary_[i],&QPushButton::clicked,this,[this,i]{emit auxiliaryRequested(AuxiliaryAction(i));});
    }
    enter_=button("regionEntryEnter","Enter Region");cancel_=button("regionEntryCancel","Cancel");
    connect(enter_,&QPushButton::clicked,this,&RegionEntryWidget::enter);
    connect(cancel_,&QPushButton::clicked,this,&RegionEntryWidget::cancelled);
    for (int i=0;i<3;++i) QWidget::setTabOrder(radios_[i],radios_[i+1]);
    QWidget::setTabOrder(radios_[3],auxiliary_[2]);QWidget::setTabOrder(auxiliary_[2],auxiliary_[0]);
    QWidget::setTabOrder(auxiliary_[0],auxiliary_[1]);QWidget::setTabOrder(auxiliary_[1],enter_);QWidget::setTabOrder(enter_,cancel_);
    populate();
}
bool RegionEntryWidget::validRegion(const Region& region) {
    const int realm=int(region.artworkRealm),difficulty=int(region.difficulty);
    if (realm<0 || realm>2 || difficulty<0 || difficulty>3) return false;
    const std::array<int,3> counts{8,12,16};
    return region.artworkNumber>=1 && region.artworkNumber<=counts[realm] &&
        region.id.isEmpty()==region.name.isEmpty() && region.name.size()<=256 &&
        (region.name.isEmpty() || !region.name.trimmed().isEmpty()) &&
        !region.name.contains('\n') && !region.name.contains('\r') && !region.name.contains(QChar(0));
}
bool RegionEntryWidget::setRegion(const Region& region,QString* error) {
    if (error) error->clear();
    if (!validRegion(region)) {if (error) *error="Invalid Region Entry model.";return false;}
    if (!assetRoot_.isEmpty() && (region.artworkRealm!=region_.artworkRealm || region.artworkNumber!=region_.artworkNumber))
        return load(assetRoot_,region,error);
    region_=region;populate();arrange();return true;
}
bool RegionEntryWidget::loadAssets(const QString& root,QString* error) {return load(root,region_,error);}
bool RegionEntryWidget::loadAssets(const QString& root,const Region& region,QString* error) {return load(root,region,error);}
bool RegionEntryWidget::load(const QString& root,const Region& region,QString* error) {
    if (error) error->clear();
    try {
        if (!validRegion(region)) throw std::runtime_error("Invalid Region Entry model");
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/RegionEntry","screen (Region Entry).cfg","JPG",QSize(800,600),QString(),artworkName(region));
        const auto sprites=mnm::ui::loadMenuSprites(root,"Sprites/Buttons.spr");
        std::array<std::array<mnm::ui::MenuSpriteFrame,3>,3> icons;
        const auto heading=assets.layout.value("TEXT_1");const auto headingRect=mnm::ui::rectangle(heading.value("Rect2"));
        if (heading.value("Font")!="LARGE" || heading.value("TextFlags")!="LEFT" || heading.value("Text").isEmpty())
            throw std::runtime_error("Invalid Region Entry heading role");
        std::array<QRect,4> radios;std::array<QString,4> choices;std::array<QRect,3> auxiliary;
        for (int i=0;i<4;++i) {
            const auto entry=assets.layout.value(QString("RADIOBUTTON_%1").arg(i+1));radios[i]=mnm::ui::rectangle(entry.value("Rect2"));
            if (entry.value("Font")!="SMALL") throw std::runtime_error("Invalid Region Entry difficulty font");
            choices[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
            if (i<3) {
                const auto icon=assets.layout.value(QString("STANDARDBUTTON_%1").arg(i+1));auxiliary[i]=mnm::ui::rectangle(icon.value("Rect2"));
                const auto indexes=icon.value("SpriteIndexes").split(',');const int first=i==0?3:i==1?6:0;
                icons[i]=mnm::ui::spriteStates(sprites,first);
                if (indexes.size()!=3) throw std::runtime_error("Invalid Region Entry icon role");
                for (int j=0;j<3;++j) if (indexes[j].trimmed()!=QString::number(first+j)) throw std::runtime_error("Unexpected Region Entry icon mapping");
            }
        }
        const auto enter=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto enterRect=mnm::ui::rectangle(enter.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        if (enter.value("Font")!="LARGE" || cancel.value("Font")!="LARGE") throw std::runtime_error("Invalid Region Entry button font");
        const auto enterText=mnm::ui::textLabel(assets.strings,enter.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        for (int i=0;i<3;++i) static_cast<mnm::ui::SpriteButton*>(auxiliary_[i])->setSprites(icons[i],auxiliary[i].size());
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_=assets.background;headingRectangle_=headingRect;radioRectangles_=radios;auxiliaryRectangles_=auxiliary;
        enterRectangle_=enterRect;cancelRectangle_=cancelRect;defaultHeading_=heading.value("Text");
        for (int i=0;i<4;++i) radios_[i]->setText(choices[i]);
        enter_->setText(enterText);cancel_->setText(cancelText);region_=region;assetRoot_=root;
        populate();arrange();update();return true;
    } catch (const std::exception& failure) {if (error) *error=QString::fromUtf8(failure.what());return false;}
}
void RegionEntryWidget::populate() {
    heading_->setText(region_.name.isEmpty()?defaultHeading_:region_.name);
    radios_[int(region_.difficulty)]->setChecked(true);
    enter_->setEnabled(!region_.id.isEmpty() && region_.enterAvailable);
    for (int i=0;i<3;++i) auxiliary_[i]->setEnabled(!region_.id.isEmpty() && region_.auxiliaryAvailable[i]);
}
void RegionEntryWidget::enter() {if (enter_->isEnabled()) emit enterRequested({region_.id,region_.difficulty});}
void RegionEntryWidget::focusFirstControl() {radios_[int(region_.difficulty)]->setFocus(Qt::OtherFocusReason);}
QRect RegionEntryWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void RegionEntryWidget::arrange() {
    const auto canvas=contentRect();const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Body,scale);
    for (int i=0;i<4;++i) {
        radios_[i]->setGeometry(map(radioRectangles_[i]));radios_[i]->setFont(font);
        const int indicator=qMax(5,qMin(qRound(14*scale),radios_[i]->height()-6));
        radios_[i]->setStyleSheet(QString(
            "QRadioButton { color:#3e2313; background:transparent; border:1px solid transparent; border-radius:3px; spacing:%1px; }"
            "QRadioButton:hover { background:rgba(255,245,214,100); }"
            "QRadioButton:checked { background:rgba(255,245,214,170); border-color:#ac915a; }"
            "QRadioButton:focus { border-color:#3e2313; }"
            "QRadioButton::indicator { width:%2px; height:%2px; border:1px solid #ac915a; border-radius:%3px; background:#fff5d6; }"
            "QRadioButton::indicator:checked { background:#3e2313; border-color:#3e2313; }")
            .arg(qMax(2,qRound(6*scale))).arg(indicator).arg((indicator+2)/2));
    }
    for (int i=0;i<3;++i) {auxiliary_[i]->setGeometry(map(auxiliaryRectangles_[i]));auxiliary_[i]->setFont(font);}
    font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale);enter_->setFont(font);cancel_->setFont(font);
    enter_->setGeometry(map(enterRectangle_));cancel_->setGeometry(map(cancelRectangle_));heading_->setGeometry(map(headingRectangle_));
    while (font.pixelSize()>10 && QFontMetrics(font).horizontalAdvance(heading_->text())>heading_->width()) font.setPixelSize(font.pixelSize()-1);
    heading_->setFont(font);
}
void RegionEntryWidget::resizeEvent(QResizeEvent* event) {QWidget::resizeEvent(event);arrange();}
bool RegionEntryWidget::eventFilter(QObject* watched,QEvent* event) {
    if (event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Escape || key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept();if (!key->isAutoRepeat()) {
                if (key->key()==Qt::Key_Escape) emit cancelled();
                else if (auto* button=qobject_cast<QPushButton*>(watched)) button->click();
                else enter();
            }return true;
        }
    }return QWidget::eventFilter(watched,event);
}
void RegionEntryWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape || event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept();if (!event->isAutoRepeat()) {if (event->key()==Qt::Key_Escape) emit cancelled();else enter();}return;
    }QWidget::keyPressEvent(event);
}
void RegionEntryWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else {painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.drawImage(contentRect(),background_);}
}

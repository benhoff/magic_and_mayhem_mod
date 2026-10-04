#include "character_screen_widget.hpp"
#include "menu_assets.hpp"
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QPixmap>
#include <QPushButton>
#include <stdexcept>
namespace {
class TexturedStatBar final : public QProgressBar {
public:
    explicit TexturedStatBar(QWidget* parent):QProgressBar(parent) {}
    QImage texture;
protected:
    void paintEvent(QPaintEvent* event) override {
        if (texture.isNull()) {QProgressBar::paintEvent(event);return;}
        QPainter painter(this);painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const int half=texture.width()/2;
        painter.drawImage(rect(),texture,QRect(half,0,half,texture.height()));
        const int filled=qRound(width()*double(value()-minimum())/(maximum()-minimum()));
        painter.save();painter.setClipRect(QRect(0,0,filled,height()));
        painter.drawImage(rect(),texture,QRect(0,0,half,texture.height()));painter.restore();
    }
};
QImage keyedPortrait(const QImage& source) {
    auto image=source.convertToFormat(QImage::Format_ARGB32);
    // Native presentation policy: remove JPEG's saturated blue backdrop, including codec noise.
    for (int y=0;y<image.height();++y) {
        auto* row=reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x=0;x<image.width();++x) if (qRed(row[x])<40 && qGreen(row[x])<40 && qBlue(row[x])>200) row[x]=0;
    }
    return image;
}
int number(const QString& text) {
    bool ok=false;const int value=text.toInt(&ok);
    if (!ok || value<0 || value>1000000) throw std::runtime_error("Invalid character stat bound or increment");
    return value;
}
Qt::Alignment alignment(const QString& value) {
    if (value=="CENTRE") return Qt::AlignCenter;
    if (value=="RIGHT") return Qt::AlignRight|Qt::AlignVCenter;
    throw std::runtime_error("Invalid character text alignment");
}
bool plainText(const QString& text,int limit) {
    return text.size()<=limit && !text.contains('\n') && !text.contains('\r') && !text.contains(QChar(0));
}
QRect talismanRectangle(const QString& value) {
    // Legacy sprite bars provide an anchor and zero right/bottom, not an area.
    const auto parts=value.split(',');
    if (parts.size()!=4 || parts[2].trimmed()!="0" || parts[3].trimmed()!="0") throw std::runtime_error("Invalid character sprite bar anchor");
    const int x=number(parts[0]),y=number(parts[1]);
    if (x+350>800 || y+45>600) throw std::runtime_error("Character sprite bar outside canvas");
    return {x,y+15,350,30}; // Native text replacement sized to the matching row.
}
}
CharacterScreenWidget::CharacterScreenWidget(QWidget* parent):QWidget(parent) {
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Character Screen preview");
    for (int i=0;i<17;++i) {
        labels_[i]=new QLabel(this);labels_[i]->setObjectName(QString("characterText%1").arg(i+1));labels_[i]->setTextFormat(Qt::PlainText);
        labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");labels_[i]->setWordWrap(i==1);
    }
    portrait_=new QLabel(this);portrait_->setObjectName("characterPortrait");portrait_->setTextFormat(Qt::PlainText);portrait_->setAlignment(Qt::AlignCenter);portrait_->setWordWrap(true);
    portrait_->setStyleSheet("color: #3e2313; background: transparent;");portrait_->setAttribute(Qt::WA_TransparentForMouseEvents);portrait_->lower();
    for (int i=0;i<3;++i) {
        bars_[i]=new TexturedStatBar(this);bars_[i]->setObjectName(QString("characterStatBar%1").arg(i));bars_[i]->setRange(minimum_[i],maximum_[i]);
        bars_[i]->setFormat("%v / %m");bars_[i]->setStyleSheet("QProgressBar { color: #3e2313; background: transparent; border: 1px solid #ac915a; text-align: center; } QProgressBar::chunk { background: #ac915a; }");
        talismans_[i]=new QLabel(this);talismans_[i]->setObjectName(QString("characterTalisman%1").arg(i));talismans_[i]->setAlignment(Qt::AlignCenter);
        talismans_[i]->setStyleSheet("color: #3e2313; background: transparent;");
    }
    auto button=[this](const QString& name,const QString& text) {
        auto* result=new QPushButton(text,this);result->setObjectName(name);result->installEventFilter(this);result->setStyleSheet(mnm::ui::menuButtonStyle());
        result->setCursor(Qt::PointingHandCursor);return result;
    };
    for (int i=0;i<12;++i) {
        buttons_[i]=button(QString("characterAdjustment%1").arg(i),i%2==0?"+":"−");
        connect(buttons_[i],&QPushButton::clicked,this,[this,i]{change(i/2,i%2==0?1:-1);});
    }
    ok_=button("characterOk","OK");cancel_=button("characterCancel","Cancel");
    connect(ok_,&QPushButton::clicked,this,&CharacterScreenWidget::accept);connect(cancel_,&QPushButton::clicked,this,&CharacterScreenWidget::cancel);
    for (int i=0;i<11;++i) {QWidget::setTabOrder(buttons_[i],buttons_[i+1]);}
    QWidget::setTabOrder(buttons_[11],ok_);QWidget::setTabOrder(ok_,cancel_);
    populate();
}
bool CharacterScreenWidget::validCharacter(const Character& character,const std::array<int,6>& minimum,const std::array<int,6>& maximum,const std::array<int,6>& increment) const {
    if (character.id.isEmpty()!=character.name.isEmpty() || !plainText(character.name,256) ||
        (!character.name.isEmpty() && character.name.trimmed().isEmpty()) || !plainText(character.portraitText,64) || !plainText(character.rating,128) ||
        character.portraitIndex<-1 || character.portraitIndex>2 || character.experiencePoints<0 || character.experiencePoints>1000000000) return false;
    for (int i=0;i<6;++i) {
        const auto& stat=character.stats[i];
        if (stat.value<minimum[i] || stat.value>maximum[i] || stat.upgradeCosts.size()>1000 || stat.upgradeCosts.size()>(maximum[i]-stat.value)/increment[i]) return false;
        for (int cost:stat.upgradeCosts) if (cost<0 || cost>1000000) return false;
    }return true;
}
bool CharacterScreenWidget::setCharacter(const Character& character,QString* error) {
    if (error) error->clear();
    if (!validCharacter(character,minimum_,maximum_,increment_)) {if (error) *error="Invalid character snapshot or upgrade costs.";return false;}
    accepted_=character;purchased_.fill(0);populate();arrange();return true;
}
bool CharacterScreenWidget::loadAssets(const QString& root,QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/CharacterScreen","screen (Character Screen).cfg","JPG",QSize(800,600),QString(),"CharacterScreen.JPG");
        std::array<QRect,17> rectangles;std::array<QString,17> texts;std::array<Qt::Alignment,17> alignments;
        const std::array<QString,7> variables{"VAR_EXPERIENCE","VAR_MANACOST","VAR_HEALTHCOST","VAR_CONTROLCOST","VAR_LAWCOST","VAR_NEUTRALCOST","VAR_CHAOSCOST"};
        for (int i=0;i<17;++i) {
            const auto entry=assets.layout.value(QString("TEXT_%1").arg(i+1));rectangles[i]=mnm::ui::rectangle(entry.value("Rect2"));alignments[i]=alignment(entry.value("TextFlags"));
            if (entry.value("Font")!=((i==1 || i==9)?"SMALL":"LARGE")) throw std::runtime_error("Invalid character font role");
            if (i==8 || (i>=10 && i<=15)) {
                if (entry.value("Text")!=variables[i==8?0:i-9]) throw std::runtime_error("Invalid character dynamic text role");
            } else if (i==16) {
                if (entry.value("Text")!="\"\"") throw std::runtime_error("Invalid character rating role");
            } else texts[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
        }
        std::array<QImage,3> textures,faces;
        for (int i=0;i<3;++i) faces[i]=keyedPortrait(mnm::ui::loadMenuImage(root,QString("Interface/CharacterScreen/800x600/WizardFace%1.JPG").arg(i),QSize(400,300)));
        std::array<QRect,3> bars,talismans;std::array<int,6> minimum,maximum,increment;
        const auto steps=assets.layout.value("STEPS_COSTS");
        const std::array<QString,6> incrementKeys{"Mana_IncrementAmount","Health_IncrementAmount","Control_IncrementAmount","Talisman_IncrementAmount","Talisman_IncrementAmount","Talisman_IncrementAmount"};
        for (int i=0;i<6;++i) {
            const auto entry=assets.layout.value(QString(i<3?"STATBAR_%1":"SPRITEBAR_%1").arg(i%3+1));
            minimum[i]=number(entry.value("minValue"));maximum[i]=number(entry.value("maxValue"));increment[i]=number(steps.value(incrementKeys[i]));
            if (minimum[i]>=maximum[i] || increment[i]==0 || increment[i]>maximum[i]-minimum[i]) throw std::runtime_error("Invalid character stat domain");
            if (i<3) {
                bars[i]=mnm::ui::rectangle(entry.value("Rect2"));
                const auto name=entry.value("Text");
                if (name.isEmpty() || name.contains('/') || name.contains('\\') || name.contains(':') || !name.endsWith(".BMP",Qt::CaseInsensitive))
                    throw std::runtime_error("Invalid character stat texture filename");
                textures[i]=mnm::ui::loadMenuImage(root,"Interface/CharacterScreen/800x600/"+name,QSize(bars[i].width()*2,bars[i].height()));
            }
            else {
                talismans[i-3]=talismanRectangle(entry.value("Rect2"));
                if (entry.value("SpriteIndexes")!=QString("%1,%2").arg(i-3).arg(i)) throw std::runtime_error("Invalid character talisman role");
            }
        }
        std::array<QRect,12> buttons;
        for (int i=0;i<12;++i) {
            const auto entry=assets.layout.value(QString("STANDARDBUTTON_%1").arg(i+1));buttons[i]=mnm::ui::rectangle(entry.value("Rect2"));
            if (entry.value("SpriteIndexes")!=(i%2==0?"6,7,8":"9,10,11")) throw std::runtime_error("Invalid character adjustment role");
        }
        const auto ok=assets.layout.value("TEXTBUTTON_1"),cancel=assets.layout.value("TEXTBUTTON_2");
        const auto okRect=mnm::ui::rectangle(ok.value("Rect2")),cancelRect=mnm::ui::rectangle(cancel.value("Rect2"));
        if (ok.value("Font")!="LARGE" || cancel.value("Font")!="LARGE") throw std::runtime_error("Invalid character action font");
        const auto okText=mnm::ui::textLabel(assets.strings,ok.value("Text")),cancelText=mnm::ui::textLabel(assets.strings,cancel.value("Text"));
        if (!validCharacter(accepted_,minimum,maximum,increment)) throw std::runtime_error("Character snapshot outside configured domains");
        for (int i=0;i<6;++i) if (purchased_[i] && increment[i]!=increment_[i]) throw std::runtime_error("Character increment changed during pending edits");
        const auto draft=draftRequest();for (int i=0;i<6;++i) if (draft.values[i]>maximum[i]) throw std::runtime_error("Character draft outside configured domains");
        faces_=faces;
        for (int i=0;i<3;++i) static_cast<TexturedStatBar*>(bars_[i])->texture=textures[i];
        background_=assets.background;labelRectangles_=rectangles;barRectangles_=bars;talismanRectangles_=talismans;buttonRectangles_=buttons;
        minimum_=minimum;maximum_=maximum;increment_=increment;okRectangle_=okRect;cancelRectangle_=cancelRect;ok_->setText(okText);cancel_->setText(cancelText);
        for (int i=0;i<17;++i) {labels_[i]->setText(texts[i]);labels_[i]->setAlignment(alignments[i]);}
        for (int i=0;i<3;++i) {bars_[i]->setRange(minimum[i],maximum[i]);bars_[i]->setAccessibleName(texts[2+i]);talismans_[i]->setAccessibleName(texts[5+i]);}
        for (int i=0;i<12;++i) buttons_[i]->setAccessibleName((i%2==0?"Increase ":"Undo increase ")+texts[2+i/2]);
        populate();arrange();update();return true;
    } catch (const std::exception& failure) {if (error) *error=QString::fromUtf8(failure.what());return false;}
}
CharacterScreenWidget::Request CharacterScreenWidget::draftRequest() const {
    Request request;request.characterId=accepted_.id;request.remainingExperience=accepted_.experiencePoints;request.purchasedIncrements=purchased_;
    for (int i=0;i<6;++i) {
        request.values[i]=accepted_.stats[i].value+purchased_[i]*increment_[i];
        for (int j=0;j<purchased_[i];++j) request.remainingExperience-=accepted_.stats[i].upgradeCosts[j];
    }return request;
}
void CharacterScreenWidget::populate() {
    const auto draft=draftRequest();labels_[0]->setText(accepted_.name.isEmpty()?"Character":accepted_.name);labels_[8]->setText(QString::number(draft.remainingExperience));
    labels_[16]->setText(accepted_.rating);labels_[16]->setVisible(!accepted_.rating.isEmpty());portrait_->setText(accepted_.portraitText);portrait_->setVisible(accepted_.portraitIndex>=0 || !accepted_.portraitText.isEmpty());
    for (int i=0;i<6;++i) {
        const auto& stat=accepted_.stats[i];const int n=purchased_[i];const bool priced=n<stat.upgradeCosts.size();
        labels_[10+i]->setText(priced?QString::number(stat.upgradeCosts[n]):"—");
        buttons_[2*i]->setEnabled(!accepted_.id.isEmpty() && stat.upgradeAvailable && priced && stat.upgradeCosts[n]<=draft.remainingExperience && draft.values[i]<=maximum_[i]-increment_[i]);
        buttons_[2*i+1]->setEnabled(!accepted_.id.isEmpty() && n>0);
        if (i<3) bars_[i]->setValue(draft.values[i]);else talismans_[i-3]->setText(QString("%1 / %2").arg(draft.values[i]).arg(maximum_[i]));
    }
    ok_->setEnabled(!accepted_.id.isEmpty());
}
void CharacterScreenWidget::change(int attribute,int direction) {
    const int button=attribute*2+(direction<0?1:0);if (!buttons_[button]->isEnabled()) return;
    purchased_[attribute]+=direction;populate();arrange();
}
void CharacterScreenWidget::accept() {
    if (!ok_->isEnabled()) return;
    const auto request=draftRequest();accepted_.experiencePoints=request.remainingExperience;
    for (int i=0;i<6;++i) {accepted_.stats[i].value=request.values[i];accepted_.stats[i].upgradeCosts.remove(0,purchased_[i]);}
    purchased_.fill(0);populate();arrange();emit characterAccepted(request);
}
void CharacterScreenWidget::cancel() {purchased_.fill(0);populate();arrange();emit cancelled();}
void CharacterScreenWidget::focusFirstControl() {
    for (auto* button:buttons_) if (button->isEnabled()) {button->setFocus(Qt::OtherFocusReason);return;}
    (ok_->isEnabled()?ok_:cancel_)->setFocus(Qt::OtherFocusReason);
}
QRect CharacterScreenWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void CharacterScreenWidget::arrange() {
    const auto canvas=contentRect();const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    QFont font("serif");font.setBold(true);
    for (int i=0;i<17;++i) {
        labels_[i]->setGeometry(map(labelRectangles_[i]));font.setPixelSize(qMax(10,qRound((i==1 || i==9?20:26)*scale)));
        while (i!=1 && font.pixelSize()>10 && QFontMetrics(font).horizontalAdvance(labels_[i]->text())>labels_[i]->width()) font.setPixelSize(font.pixelSize()-1);
        labels_[i]->setFont(font);
    }
    font.setPixelSize(qMax(10,qRound(20*scale)));portrait_->setGeometry(map(accepted_.portraitIndex>=0?QRect(0,0,400,300):QRect(25,50,180,150)));portrait_->setFont(font);
    if (accepted_.portraitIndex>=0 && !faces_[accepted_.portraitIndex].isNull()) {
        portrait_->setPixmap(QPixmap::fromImage(faces_[accepted_.portraitIndex]).scaled(portrait_->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        portrait_->setAccessibleName(accepted_.portraitText.isEmpty()?accepted_.name:accepted_.portraitText);
    } else {portrait_->setPixmap(QPixmap());portrait_->setText(accepted_.portraitText);}
    for (int i=0;i<3;++i) {talismans_[i]->setGeometry(map(talismanRectangles_[i]));talismans_[i]->setFont(font);bars_[i]->setGeometry(map(barRectangles_[i]));auto small=font;small.setPixelSize(qMax(10,qRound(14*scale)));bars_[i]->setFont(small);}
    for (int i=0;i<12;++i) {buttons_[i]->setGeometry(map(buttonRectangles_[i]));buttons_[i]->setFont(font);}
    font.setPixelSize(qMax(10,qRound(26*scale)));ok_->setFont(font);cancel_->setFont(font);ok_->setGeometry(map(okRectangle_));cancel_->setGeometry(map(cancelRectangle_));
}
void CharacterScreenWidget::resizeEvent(QResizeEvent* event) {QWidget::resizeEvent(event);arrange();}
bool CharacterScreenWidget::eventFilter(QObject* watched,QEvent* event) {
    if (event->type()==QEvent::KeyPress) {
        auto* key=static_cast<QKeyEvent*>(event);
        if (key->key()==Qt::Key_Escape || key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
            key->accept();if (!key->isAutoRepeat()) {if (key->key()==Qt::Key_Escape) cancel();else if (auto* button=qobject_cast<QPushButton*>(watched)) button->click();}return true;
        }
    }return QWidget::eventFilter(watched,event);
}
void CharacterScreenWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape || event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept();if (!event->isAutoRepeat()) {if (event->key()==Qt::Key_Escape) cancel();else accept();}return;
    }QWidget::keyPressEvent(event);
}
void CharacterScreenWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else {painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.drawImage(contentRect(),background_);}
}

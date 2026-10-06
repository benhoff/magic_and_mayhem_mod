#include "battle_result_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <stdexcept>

namespace {
int alignment(const QString& value) {
    if (value == "LEFT") return Qt::AlignLeft | Qt::AlignVCenter;
    if (value == "RIGHT") return Qt::AlignRight | Qt::AlignVCenter;
    if (value == "CENTRE" || value == "MIDDLE") return Qt::AlignHCenter | Qt::AlignVCenter;
    throw std::runtime_error("Invalid result text alignment");
}
int fontSize(const QString& value) {
    if (value == "LARGE") return 26;
    if (value == "SMALL") return 20;
    throw std::runtime_error("Invalid result font role");
}
}
BattleResultWidget::BattleResultWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240);
    for (int i=0;i<21;++i) {
        labels_[i]=new QLabel(this);
        labels_[i]->setObjectName(QString("battleResultText%1").arg(i+1));
        labels_[i]->setTextFormat(Qt::PlainText);
        labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");
        fontSizes_[i]=26; alignments_[i]=Qt::AlignLeft | Qt::AlignVCenter;
    }
    title_=new QLabel(this); title_->setObjectName("battleResultTitle");
    title_->setTextFormat(Qt::PlainText); title_->setAlignment(Qt::AlignCenter);
    title_->setStyleSheet("color: #3e2313; background: transparent;");
    continue_=new QPushButton("OK",this);
    continue_->setObjectName("battleResultContinue");
    continue_->setStyleSheet(mnm::ui::menuButtonStyle());
    continue_->setCursor(Qt::PointingHandCursor);
    connect(continue_,&QPushButton::clicked,this,&BattleResultWidget::continueRequested);
    setResults({});
}
bool BattleResultWidget::loadAssets(const QString& root, Outcome outcome, QString* error) {
    if (error) error->clear();
    try {
        const QString name=outcome==Outcome::Victory?"Victory":"Defeat";
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/BattleEnd",
            "screen (Battle End - "+name+").cfg","JPG",QSize(800,600),"Battle "+name+" Screen");
        std::array<QRect,21> rectangles;
        std::array<int,21> alignments,fonts;
        std::array<QString,21> texts;
        for (int i=0;i<21;++i) {
            const auto entry=assets.layout.value(QString("TEXT_%1").arg(i+1));
            rectangles[i]=mnm::ui::rectangle(entry.value("Rect2"));
            alignments[i]=alignment(entry.value("TextFlags")); fonts[i]=fontSize(entry.value("Font"));
            // Only these three sections contain fixed string-table labels.
            if (i==0 || i==1 || i==16 || i==20) texts[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
        }
        const auto button=assets.layout.value("TEXTBUTTON_1");
        const auto buttonRect=mnm::ui::rectangle(button.value("Rect2"));
        const auto buttonLabel=mnm::ui::textLabel(assets.strings,button.value("Text"));
        if (button.value("Font")!="LARGE") throw std::runtime_error("Invalid result button font");
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_=assets.background; outcome_=outcome; rectangles_=rectangles;
        alignments_=alignments; fontSizes_=fonts; continueRect_=buttonRect;
        for (int i=0;i<21;++i) { labels_[i]->setText(texts[i]); labels_[i]->setAlignment(Qt::Alignment(alignments_[i])); }
        ratingLabel_=texts[20]; continue_->setText(buttonLabel);
        setWindowTitle("Magic & Mayhem — "+name.toLower()+" results preview");
        setResults(results_); arrange(); update(); return true;
    } catch (const std::exception& failure) {
        if (error) *error=QString::fromUtf8(failure.what());
        return false;
    }
}
void BattleResultWidget::setResults(const Results& results) {
    results_=results;
    title_->setText(results.title); title_->setVisible(!results.title.isEmpty());
    bool anyReward=false;
    for (int i=0;i<7;++i) {
        const bool active=!results.rewards[i].achievement.isEmpty() || !results.rewards[i].points.isEmpty();
        anyReward=anyReward || active;
        labels_[2+i*2]->setText(results.rewards[i].achievement);
        labels_[3+i*2]->setText(results.rewards[i].points);
        labels_[2+i*2]->setVisible(active); labels_[3+i*2]->setVisible(active);
    }
    labels_[0]->setVisible(anyReward); labels_[1]->setVisible(anyReward);
    // Rating and totals occupy overlapping rectangles in the installed layouts.
    const bool totals=results.rating.isEmpty() && !results.totalPoints.isEmpty();
    labels_[16]->setVisible(totals); labels_[17]->setVisible(totals);
    labels_[17]->setText(results.totalPoints);
    labels_[18]->setText("/  "+results.maximumPoints);
    labels_[18]->setVisible(totals && !results.maximumPoints.isEmpty());
    labels_[19]->setText(results.summary); labels_[19]->setVisible(!results.summary.isEmpty());
    labels_[20]->setText(ratingLabel_+" "+results.rating); labels_[20]->setVisible(!results.rating.isEmpty());
}
void BattleResultWidget::setOriginalReport(const QString& title,const std::array<QString,21>& texts) {
    title_->setText(title);title_->setVisible(!title.isEmpty());
    for(int i=0;i<21;++i){labels_[i]->setText(texts[i]);labels_[i]->setVisible(!texts[i].isEmpty());}
}
QRect BattleResultWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void BattleResultWidget::focusContinue() { continue_->setFocus(Qt::OtherFocusReason); }
void BattleResultWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r) { return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale)); };
    for (int i=0;i<21;++i) {
        labels_[i]->setGeometry(map(rectangles_[i]));
        auto font=mnm::ui::menuFont(this,fontSizes_[i]==26?mnm::ui::MenuFontRole::Heading:mnm::ui::MenuFontRole::Body,scale); labels_[i]->setFont(font);
    }
    // Native banner placement: the CFG provides no title rectangle.
    title_->setGeometry(map(QRect(50,25,700,55)));
    auto titleFont=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale); title_->setFont(titleFont);
    continue_->setGeometry(map(continueRect_));
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale); continue_->setFont(font);
}
void BattleResultWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void BattleResultWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter || event->key()==Qt::Key_Escape) {
        event->accept(); if (!event->isAutoRepeat()) emit continueRequested(); return;
    }
    QWidget::keyPressEvent(event);
}
void BattleResultWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

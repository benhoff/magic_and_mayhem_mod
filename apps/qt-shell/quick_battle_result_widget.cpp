#include "quick_battle_result_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <stdexcept>
namespace {
int alignment(const QString& value) {
    if (value=="LEFT") return Qt::AlignLeft | Qt::AlignVCenter;
    if (value=="RIGHT") return Qt::AlignRight | Qt::AlignVCenter;
    if (value=="MIDDLE" || value=="CENTRE") return Qt::AlignCenter;
    throw std::runtime_error("Invalid Quick Battle result alignment");
}
int fontSize(const QString& role) {
    if (role=="LARGE") return 26;
    if (role=="SMALL") return 20;
    throw std::runtime_error("Invalid Quick Battle result font role");
}
}
QuickBattleResultWidget::QuickBattleResultWidget(QWidget* parent) : QWidget(parent) {
    resize(800,600); setMinimumSize(320,240); setFocusPolicy(Qt::StrongFocus);
    setWindowTitle("Magic & Mayhem — Quick Battle results preview");
    for (int i=0;i<26;++i) {
        labels_[i]=new QLabel(this); labels_[i]->setObjectName(QString("quickResultText%1").arg(i+1));
        labels_[i]->setTextFormat(Qt::PlainText); labels_[i]->setStyleSheet("color: #3e2313; background: transparent;");
        fontSizes_[i]=26; alignments_[i]=Qt::AlignCenter;
    }
    for (int i=0;i<4;++i) {
        portraits_[i]=new QLabel(this); portraits_[i]->setObjectName(QString("quickResultPortrait%1").arg(i+1));
        portraits_[i]->setTextFormat(Qt::PlainText); portraits_[i]->setAlignment(Qt::AlignCenter); portraits_[i]->setWordWrap(true);
        portraits_[i]->setStyleSheet("color: #3e2313; background: transparent; border: 1px solid #ac915a;");
    }
    const std::array<const char*,3> names{"Spectate","Continue","Quit"};
    for (int i=0;i<3;++i) {
        buttons_[i]=new QPushButton(QString::fromLatin1(names[i]),this);
        buttons_[i]->setObjectName(QString("quickResultAction%1").arg(i));
        buttons_[i]->setStyleSheet(mnm::ui::menuButtonStyle()); buttons_[i]->setCursor(Qt::PointingHandCursor);
        connect(buttons_[i],&QPushButton::clicked,this,[this,i] {
            if (buttons_[i]->isEnabled()) emit actionRequested(static_cast<Action>(i));
        });
    }
    setResults({});
}
bool QuickBattleResultWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        const auto assets=mnm::ui::loadMenuAssets(root,"Interface/QuickBattleEnd","screen (Quick Battle End).cfg");
        std::array<QRect,26> rectangles;
        std::array<int,26> alignments,fonts;
        std::array<QString,6> headings;
        std::array<QRect,4> portraits;
        std::array<QRect,3> buttons;
        std::array<QString,3> buttonLabels;
        for (int i=0;i<26;++i) {
            const auto entry=assets.layout.value(QString("TEXT_%1").arg(i+1));
            rectangles[i]=mnm::ui::rectangle(entry.value("Rect2"));
            alignments[i]=alignment(entry.value("Textflags")); fonts[i]=fontSize(entry.value("Font"));
            if (i<6) headings[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
        }
        for (int i=0;i<4;++i) portraits[i]=mnm::ui::rectangle(assets.layout.value(QString("PICTUREBOX_%1").arg(i+1)).value("Rect2"));
        for (int i=0;i<3;++i) {
            const auto entry=assets.layout.value(QString("TEXTBUTTON_%1").arg(i+1));
            buttons[i]=mnm::ui::rectangle(entry.value("Rect2"));
            buttonLabels[i]=mnm::ui::textLabel(assets.strings,entry.value("Text"));
            if (entry.value("Font")!="LARGE") throw std::runtime_error("Invalid Quick Battle result button font");
        }
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_=assets.background; rectangles_=rectangles; alignments_=alignments; fontSizes_=fonts;
        portraitRectangles_=portraits; buttonRectangles_=buttons;
        for (int i=0;i<26;++i) labels_[i]->setAlignment(Qt::Alignment(alignments_[i]));
        for (int i=0;i<6;++i) labels_[i]->setText(headings[i]);
        // Native wrapping keeps the two-word handicap header within its CFG column.
        labels_[4]->setWordWrap(true);
        for (int i=0;i<3;++i) buttons_[i]->setText(buttonLabels[i]);
        setResults(results_); arrange(); update(); return true;
    } catch (const std::exception& failure) {
        if (error) *error=QString::fromUtf8(failure.what());
        return false;
    }
}
void QuickBattleResultWidget::setResults(const Results& results) {
    const bool actionsChanged=results.primaryAction!=results_.primaryAction || results.canQuit!=results_.canQuit;
    results_=results;
    for (int i=0;i<4;++i) {
        const auto& player=results.players[i];
        const std::array<QString,5> values{player.name,player.kills,player.deaths,player.handicapBonus,player.score};
        for (int column=0;column<5;++column) {
            auto* label=labels_[6+column*4+i]; label->setText(player.active?values[column]:QString()); label->setVisible(player.active);
        }
        portraits_[i]->setText(player.active?player.portraitText:QString());
        portraits_[i]->setVisible(player.active && !player.portraitText.isEmpty());
    }
    const bool spectate=results.primaryAction==PrimaryAction::Spectate;
    const bool proceed=results.primaryAction==PrimaryAction::Continue;
    buttons_[0]->setVisible(spectate); buttons_[0]->setEnabled(spectate);
    buttons_[1]->setVisible(proceed); buttons_[1]->setEnabled(proceed);
    buttons_[2]->setEnabled(results.canQuit);
    auto* focused=focusWidget();
    if (isVisible() && (actionsChanged || !focused || !focused->isVisibleTo(this) || !focused->isEnabled())) focusFirstAction();
}
void QuickBattleResultWidget::focusFirstAction() {
    for (auto* button:buttons_) if (button->isEnabled()) { button->setFocus(Qt::OtherFocusReason); return; }
    setFocus(Qt::OtherFocusReason);
}
QRect QuickBattleResultWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void QuickBattleResultWidget::arrange() {
    const auto canvas=contentRect(); const double scale=canvas.width()/800.0;
    auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    for (int i=0;i<26;++i) {
        labels_[i]->setGeometry(map(rectangles_[i])); auto font=mnm::ui::menuFont(this,fontSizes_[i]==26?mnm::ui::MenuFontRole::Heading:mnm::ui::MenuFontRole::Body,scale); labels_[i]->setFont(font);
    }
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale);
    for (int i=0;i<3;++i) { buttons_[i]->setGeometry(map(buttonRectangles_[i])); buttons_[i]->setFont(font); }
    font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Tooltip,scale);
    for (int i=0;i<4;++i) { portraits_[i]->setGeometry(map(portraitRectangles_[i])); portraits_[i]->setFont(font); }
}
void QuickBattleResultWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void QuickBattleResultWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key()==Qt::Key_Escape) {
        event->accept(); if (!event->isAutoRepeat() && results_.canQuit) emit actionRequested(Action::Quit); return;
    }
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) {
        event->accept();
        if (!event->isAutoRepeat()) for (auto* button:buttons_) if (button->hasFocus() && button->isEnabled()) { button->click(); break; }
        return;
    }
    QWidget::keyPressEvent(event);
}
void QuickBattleResultWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(),Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(),QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(),background_); }
}

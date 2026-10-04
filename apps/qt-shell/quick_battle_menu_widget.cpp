#include "quick_battle_menu_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <stdexcept>

QuickBattleMenuWidget::QuickBattleMenuWidget(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Magic & Mayhem — Quick Battle preview");
    resize(800, 600);
    setMinimumSize(320, 240);
    heading_ = new QLabel("Quick Battle", this);
    heading_->setTextFormat(Qt::PlainText);
    heading_->setAlignment(Qt::AlignCenter);
    heading_->setStyleSheet("color: #3e2313; background: transparent;");
    const std::array<const char*,4> labels{"Create Multiplayer Game", "Join Multiplayer Game", "Create Single Player Game", "Cancel"};
    for (int i = 0; i < 4; ++i) {
        auto* button = new QPushButton(QString::fromLatin1(labels[i]), this);
        buttons_[i] = button;
        button->setObjectName(QString("quickBattleAction%1").arg(i));
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(mnm::ui::menuButtonStyle());
        connect(button, &QPushButton::clicked, this, [this, i] { emit actionRequested(static_cast<Action>(i)); });
    }
    focusFirstAction();
    arrange();
}
bool QuickBattleMenuWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        auto assets = mnm::ui::loadMenuAssets(root, "Interface/QuickBattleMainMenu", "Screen (Quick Battle Main Menu).cfg");
        std::array<QRect, 4> rectangles;
        std::array<QString, 4> labels;
        for (int i = 0; i < 4; ++i) {
            const auto entry = assets.layout.value(QString("TEXTBUTTON_%1").arg(i + 1));
            rectangles[i] = mnm::ui::rectangle(entry.value("Rect2"));
            labels[i] = mnm::ui::textLabel(assets.strings, entry.value("Text"));
        }
        const auto heading = assets.layout.value("TEXT_1");
        const auto headingRect = mnm::ui::rectangle(heading.value("Rect2"));
        const auto headingText = mnm::ui::textLabel(assets.strings, heading.value("Text"));
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_ = std::move(assets.background);
        rectangles_ = rectangles;
        headingRectangle_ = headingRect;
        heading_->setText(headingText);
        for (int i = 0; i < 4; ++i) buttons_[i]->setText(labels[i]);
        arrange();
        update();
        return true;
    } catch (const std::exception& failure) {
        if (error) *error = QString::fromUtf8(failure.what());
        return false;
    }
}
QRect QuickBattleMenuWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void QuickBattleMenuWidget::focusFirstAction() { buttons_[0]->setFocus(Qt::OtherFocusReason); }
void QuickBattleMenuWidget::arrange() {
    const auto canvas = contentRect();
    const double scale = canvas.width() / 800.0;
    auto map = [&](const QRect& rect) {
        return QRect(canvas.x() + qRound(rect.x()*scale), canvas.y() + qRound(rect.y()*scale),
                     qRound(rect.width()*scale), qRound(rect.height()*scale));
    };
    for (int i = 0; i < 4; ++i) {
        buttons_[i]->setGeometry(map(rectangles_[i]));
        auto font=mnm::ui::menuFont(this,i==3?mnm::ui::MenuFontRole::Heading:mnm::ui::MenuFontRole::Body,scale);
        buttons_[i]->setFont(font);
    }
    heading_->setGeometry(map(headingRectangle_));
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale);
    heading_->setFont(font);
}
void QuickBattleMenuWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void QuickBattleMenuWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape && !event->isAutoRepeat()) {
        event->accept(); emit actionRequested(Action::Cancel); return;
    }
    QWidget::keyPressEvent(event);
}
void QuickBattleMenuWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(), QColor("#b39a6a"));
    else { painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(contentRect(), background_); }
}

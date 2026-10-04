#include "mini_menu_widget.hpp"
#include "menu_assets.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <stdexcept>

namespace {
constexpr std::array<MiniMenuWidget::Action, 8> menuActions{
    MiniMenuWidget::Action::LoadGame, MiniMenuWidget::Action::SaveGame,
    MiniMenuWidget::Action::Preferences, MiniMenuWidget::Action::QuitGame,
    MiniMenuWidget::Action::Cancel, MiniMenuWidget::Action::Preferences,
    MiniMenuWidget::Action::QuitBattle, MiniMenuWidget::Action::Cancel};
}
MiniMenuWidget::MiniMenuWidget(QWidget* parent) : QWidget(parent) {
    resize(800, 600); setMinimumSize(320, 240);
    const std::array<const char*, 8> labels{"Load Game", "Save Game", "Preferences", "Quit Game", "Cancel", "Preferences", "Quit Battle", "Cancel"};
    for (int i = 0; i < 8; ++i) {
        const int top = i < 5 ? 180 + 50*i : 230 + 50*(i-5);
        rectangles_[i] = QRect(250, top, 300, 40);
        auto* button = new QPushButton(QString::fromLatin1(labels[i]), this);
        buttons_[i] = button;
        button->setObjectName(QString("miniMenuButton%1").arg(i + 1));
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(mnm::ui::menuButtonStyle());
        connect(button, &QPushButton::clicked, this, [this, i] {
            // Guard inactive controls even if invoked programmatically.
            if ((mode_ == Mode::Campaign) == (i < 5)) emit actionRequested(menuActions[i]);
        });
    }
    arrange(); setMode(Mode::Campaign);
}
bool MiniMenuWidget::loadAssets(const QString& root, QString* error) {
    if (error) error->clear();
    try {
        auto assets = mnm::ui::loadMenuAssets(root, "Interface/MiniMenu", "screen (Mini Menu).cfg", "BMP", QSize(600,400));
        std::array<QRect, 8> rectangles;
        std::array<QString, 8> labels;
        for (int i = 0; i < 8; ++i) {
            const auto entry = assets.layout.value(QString("TEXTBUTTON_%1").arg(i + 1));
            rectangles[i] = mnm::ui::rectangle(entry.value("Rect2"));
            labels[i] = mnm::ui::textLabel(assets.strings, entry.value("Text"));
        }
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_ = std::move(assets.background); rectangles_ = rectangles;
        for (int i = 0; i < 8; ++i) buttons_[i]->setText(labels[i]);
        arrange(); update(); return true;
    } catch (const std::exception& failure) {
        if (error) *error = QString::fromUtf8(failure.what());
        return false;
    }
}
void MiniMenuWidget::setMode(Mode mode) {
    mode_ = mode;
    for (int i = 0; i < 8; ++i) {
        const bool active = (mode_ == Mode::Campaign) == (i < 5);
        buttons_[i]->setVisible(active);
        buttons_[i]->setEnabled(active);
    }
    setWindowTitle(mode_ == Mode::Campaign ? "Magic & Mayhem — campaign Mini Menu preview" : "Magic & Mayhem — battle Mini Menu preview");
    focusFirstAction();
}
void MiniMenuWidget::focusFirstAction() { buttons_[mode_ == Mode::Campaign ? 0 : 5]->setFocus(Qt::OtherFocusReason); }
QRect MiniMenuWidget::contentRect() const { return mnm::ui::menuContentRect(size()); }
void MiniMenuWidget::arrange() {
    const auto canvas = contentRect();
    const double scale = canvas.width()/800.0;
    for (int i = 0; i < 8; ++i) {
        const auto rect = rectangles_[i];
        buttons_[i]->setGeometry(canvas.x() + qRound(rect.x()*scale), canvas.y() + qRound(rect.y()*scale),
                                qRound(rect.width()*scale), qRound(rect.height()*scale));
        auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale);
        buttons_[i]->setFont(font);
    }
}
void MiniMenuWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void MiniMenuWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape && !event->isAutoRepeat()) {
        event->accept(); emit actionRequested(Action::Cancel); return;
    }
    QWidget::keyPressEvent(event);
}
void MiniMenuWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.fillRect(rect(), Qt::black);
    if (background_.isNull()) painter.fillRect(contentRect(), QColor("#b39a6a"));
    else {
        const auto canvas = contentRect();
        const double scale = canvas.width()/800.0;
        const QRect panel(canvas.x()+qRound(100*scale), canvas.y()+qRound(100*scale), qRound(600*scale), qRound(400*scale));
        painter.setRenderHint(QPainter::SmoothPixmapTransform); painter.drawImage(panel, background_);
    }
}

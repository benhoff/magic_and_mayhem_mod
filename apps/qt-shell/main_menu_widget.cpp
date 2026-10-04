#include "main_menu_widget.hpp"
#include "menu_assets.hpp"
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <stdexcept>


MainMenuWidget::MainMenuWidget(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Magic & Mayhem — main menu preview");
    resize(800, 600);
    setMinimumSize(320, 240);
    const std::array<const char*, 6> labels{"New Game", "Load Game", "Quick Battle", "Preferences", "Quit", "CommandLine Battle"};
    for (int i = 0; i < 6; ++i) {
        rectangles_[i] = i < 5 ? QRect(150, 310 + i * 50, 500, 40) : QRect(250, 265, 300, 35);
        auto* button = new QPushButton(QString::fromLatin1(labels[i]), this);
        buttons_[i] = button;
        button->setObjectName(QString("mainMenuAction%1").arg(i));
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(mnm::ui::menuButtonStyle());
        connect(button, &QPushButton::clicked, this, [this, i] { emit actionRequested(static_cast<Action>(i)); });
    }
    buttons_[5]->hide();
    version_ = new QLabel(this);
    version_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    version_->setStyleSheet("color: #3e2313; background: transparent;");
    buttons_[0]->setFocus();
    arrange();
}

bool MainMenuWidget::loadAssets(const QString& installationRoot, QString* error) {
    if (error) error->clear();
    try {
        auto assets = mnm::ui::loadMenuAssets(installationRoot, "Interface/MainScreen", "screen (MainMenu).cfg");
        const auto& layout = assets.layout;
        std::array<QRect, 6> rectangles;
        std::array<QString, 6> labels;
        for (int i = 0; i < 6; ++i) {
            const auto entry = layout.value(QString("TEXTBUTTON_%1").arg(i + 1));
            rectangles[i] = mnm::ui::rectangle(entry.value("Rect2"));
            labels[i] = mnm::ui::textLabel(assets.strings, entry.value("Text"));
        }
        const auto versionRectangle = mnm::ui::rectangle(layout.value("TEXT_1").value("Rect2"));
        mnm::ui::installMenuFonts(this,assets.fonts);
        background_ = std::move(assets.background);
        rectangles_ = rectangles;
        versionRectangle_ = versionRectangle;
        for (int i = 0; i < 6; ++i) buttons_[i]->setText(labels[i]);
        arrange();
        update();
        return true;
    } catch (const std::exception& failure) {
        if (error) *error = QString::fromUtf8(failure.what());
        return false;
    }
}

void MainMenuWidget::setCommandLineBattleVisible(bool visible) { buttons_[5]->setVisible(visible); }
void MainMenuWidget::setVersionText(const QString& text) { version_->setText(text); }
QRect MainMenuWidget::contentRect() const {
    return mnm::ui::menuContentRect(size());
}
void MainMenuWidget::arrange() {
    const auto canvas = contentRect();
    const double scale = canvas.width() / 800.0;
    auto map = [&](const QRect& rect) {
        return QRect(canvas.x() + qRound(rect.x() * scale), canvas.y() + qRound(rect.y() * scale),
                     qRound(rect.width() * scale), qRound(rect.height() * scale));
    };
    for (int i = 0; i < 6; ++i) {
        buttons_[i]->setGeometry(map(rectangles_[i]));
        auto font=mnm::ui::menuFont(this,i==5?mnm::ui::MenuFontRole::Tooltip:mnm::ui::MenuFontRole::Heading,scale);
        buttons_[i]->setFont(font);
    }
    version_->setGeometry(map(versionRectangle_));
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Tooltip,scale);
    version_->setFont(font);
}
void MainMenuWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); arrange(); }
void MainMenuWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    const auto canvas = contentRect();
    if (!background_.isNull()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(canvas, background_);
    } else {
        painter.fillRect(canvas, QColor("#b39a6a"));
        painter.setPen(QColor("#3e2313"));
        painter.drawText(QRect(canvas.x(), canvas.y(), canvas.width(), canvas.height() / 3), Qt::AlignCenter, "Magic & Mayhem");
    }
}

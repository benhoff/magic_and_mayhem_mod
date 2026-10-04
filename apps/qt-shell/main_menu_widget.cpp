#include "main_menu_widget.hpp"
#include "asset_file.hpp"
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <stdexcept>

namespace {
using Sections = QMap<QString, QMap<QString, QString>>;
Sections parse(const QByteArray& bytes) {
    Sections result;
    QString section;
    for (QString line : QString::fromLatin1(bytes).split('\n')) {
        line = line.section(';', 0, 0).trimmed();
        if (line.isEmpty()) continue;
        if (line.startsWith('[') && line.endsWith(']')) {
            section = line.mid(1, line.size() - 2).trimmed();
            continue;
        }
        const auto equal = line.indexOf('=');
        if (section.isEmpty() || equal < 1) throw std::runtime_error("Invalid menu configuration entry");
        const auto key = line.left(equal).trimmed();
        if (result[section].contains(key)) throw std::runtime_error("Duplicate menu configuration key");
        result[section][key] = line.mid(equal + 1).trimmed();
    }
    return result;
}
QRect rectangle(const QString& text) {
    const auto parts = text.split(',');
    if (parts.size() != 4) throw std::runtime_error("Invalid menu rectangle");
    std::array<int, 4> values{};
    for (int i = 0; i < 4; ++i) {
        bool ok = false;
        values[i] = parts[i].trimmed().toInt(&ok);
        if (!ok) throw std::runtime_error("Invalid menu rectangle coordinate");
    }
    const auto [left, top, right, bottom] = values;
    if (left < 0 || top < 0 || right > 800 || bottom > 600 || right <= left || bottom <= top)
        throw std::runtime_error("Menu rectangle outside 800x600 canvas");
    return {left, top, right - left, bottom - top};
}
QByteArray read(const mnm::assets::AssetStore& store, const QString& path, int limit) {
    auto opened = store.open(path.toStdString());
    if (auto* error = std::get_if<mnm::assets::Error>(&opened))
        throw std::runtime_error(path.toStdString() + ": " + error->detail);
    auto bytes = mnm::assets::readWhole(*std::get<std::unique_ptr<mnm::assets::AssetFile>>(opened), limit);
    if (auto* error = std::get_if<mnm::assets::Error>(&bytes))
        throw std::runtime_error(path.toStdString() + ": " + error->detail);
    const auto& data = std::get<std::vector<std::uint8_t>>(bytes);
    return {reinterpret_cast<const char*>(data.data()), qsizetype(data.size())};
}
}

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
        button->setStyleSheet("QPushButton { color: #3e2313; background: transparent; border: 1px solid transparent; }"
                              "QPushButton:hover, QPushButton:focus { color: #fff5d6; background: #302719; border-color: #ac915a; }"
                              "QPushButton:pressed { background: #51402a; }"
                              "QPushButton:disabled { color: #888888; }");
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
        auto created = mnm::assets::AssetStore::create(std::filesystem::path(installationRoot.toStdString()));
        if (auto* failure = std::get_if<mnm::assets::Error>(&created)) throw std::runtime_error(failure->detail);
        const auto& store = std::get<mnm::assets::AssetStore>(created);
        const auto layout = parse(read(store, "Interface/MainScreen/screen (MainMenu).cfg", 65536));
        const auto strings = parse(read(store, "CFG/interface screens text.cfg", 1024 * 1024));
        auto name = layout.value("GLOBALS").value("BackgroundFile");
        if (name.startsWith('"') && name.endsWith('"')) name = name.mid(1, name.size() - 2);
        if (name.isEmpty() || name.contains('/') || name.contains('\\') || name.contains(':'))
            throw std::runtime_error("Invalid menu background name");
        QImage background;
        const auto image = read(store, "Interface/MainScreen/800x600/" + name + " 800-600.JPG", 8 * 1024 * 1024);
        if (!background.loadFromData(image, "JPG") || background.size() != QSize(800, 600))
            throw std::runtime_error("Expected an 800x600 JPEG menu background");
        std::array<QRect, 6> rectangles;
        std::array<QString, 6> labels;
        for (int i = 0; i < 6; ++i) {
            const auto entry = layout.value(QString("TEXTBUTTON_%1").arg(i + 1));
            rectangles[i] = rectangle(entry.value("Rect2"));
            bool ok = false;
            const int id = entry.value("Text").toInt(&ok);
            if (!ok || id < 0 || id > 999) throw std::runtime_error("Invalid menu text ID");
            labels[i] = strings.value("STRINGS").value(QString("STR_%1").arg(id, 2, 10, QLatin1Char('0')));
            if (labels[i].isEmpty() || labels[i].size() > 256) throw std::runtime_error("Missing or oversized menu label");
        }
        const auto versionRectangle = rectangle(layout.value("TEXT_1").value("Rect2"));
        background_ = std::move(background);
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
    QSize canvas(800, 600);
    canvas.scale(size(), Qt::KeepAspectRatio);
    return {(width() - canvas.width()) / 2, (height() - canvas.height()) / 2, canvas.width(), canvas.height()};
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
        QFont font("serif");
        font.setBold(true);
        font.setPixelSize(qMax(10, qRound((i == 5 ? 16 : 26) * scale)));
        buttons_[i]->setFont(font);
    }
    version_->setGeometry(map(versionRectangle_));
    auto font = version_->font();
    font.setPixelSize(qMax(10, qRound(16 * scale)));
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

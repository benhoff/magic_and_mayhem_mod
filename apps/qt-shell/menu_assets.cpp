#include "menu_assets.hpp"
#include "asset_file.hpp"
#include <array>
#include <stdexcept>

namespace mnm::ui {
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

MenuAssets loadMenuAssets(const QString& root, const QString& directory, const QString& config, const char* imageFormat, const QSize& imageSize, const QString& backgroundName) {
    auto created = mnm::assets::AssetStore::create(std::filesystem::path(root.toStdString()));
    if (auto* failure = std::get_if<mnm::assets::Error>(&created)) throw std::runtime_error(failure->detail);
    const auto& store = std::get<mnm::assets::AssetStore>(created);
    MenuAssets result;
    result.layout = parse(read(store, directory + "/" + config, 65536));
    result.strings = parse(read(store, "CFG/interface screens text.cfg", 1024 * 1024));
    auto name = backgroundName.isEmpty() ? result.layout.value("GLOBALS").value("BackgroundFile") : backgroundName;
    if (name.startsWith('"') && name.endsWith('"')) name = name.mid(1, name.size() - 2);
    if (name.isEmpty() || name.contains('/') || name.contains('\\') || name.contains(':'))
        throw std::runtime_error("Invalid menu background name");
    const auto image = read(store, directory + "/800x600/" + name + " 800-600." + QString::fromLatin1(imageFormat), 8 * 1024 * 1024);
    if (!result.background.loadFromData(image, imageFormat) || result.background.size() != imageSize)
        throw std::runtime_error("Invalid menu background dimensions or format");
    return result;
}
QString textLabel(const Sections& strings, const QString& textId) {
    bool ok = false;
    const int id = textId.toInt(&ok);
    if (!ok || id < 0 || id > 999) throw std::runtime_error("Invalid menu text ID");
    const auto label = strings.value("STRINGS").value(QString("STR_%1").arg(id, 2, 10, QLatin1Char('0')));
    if (label.isEmpty() || label.size() > 256) throw std::runtime_error("Missing or oversized menu label");
    return label;
}
QRect menuContentRect(const QSize& size) {
    QSize canvas(800, 600);
    canvas.scale(size, Qt::KeepAspectRatio);
    return {(size.width() - canvas.width()) / 2, (size.height() - canvas.height()) / 2, canvas.width(), canvas.height()};
}
QString menuButtonStyle() {
    return "QPushButton { color: #3e2313; background: transparent; border: 1px solid transparent; }"
           "QPushButton:hover, QPushButton:focus { color: #fff5d6; background: #302719; border-color: #ac915a; }"
           "QPushButton:pressed { background: #51402a; }"
           "QPushButton:disabled { color: #888888; }";
}
}

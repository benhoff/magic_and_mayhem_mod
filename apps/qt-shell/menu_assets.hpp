#pragma once
#include "menu_fonts.hpp"
#include <QImage>
#include <QMap>
#include <QRect>

namespace mnm::ui {
using Sections = QMap<QString, QMap<QString, QString>>;
struct MenuAssets {
    Sections layout;
    Sections strings;
    QImage background;
    MenuFontSet fonts;
};
// Bounded read-only installed menu inputs. Throws on missing/invalid inputs.
MenuAssets loadMenuAssets(const QString& root, const QString& directory, const QString& config,
                         const char* imageFormat = "JPG", const QSize& imageSize = QSize(800, 600),
                         const QString& backgroundName = QString(), const QString& imageFileName = QString());
// Owned RGB decoded through the bounded native BMP/JPEG loaders.
QImage loadMenuImage(const QString& root, const QString& path, const QSize& expectedSize);
Sections loadMenuLayout(const QString& root, const QString& directory, const QString& config);
QRect rectangle(const QString& text);
QString textLabel(const Sections& strings, const QString& textId);
QRect menuContentRect(const QSize& size);
QString menuButtonStyle();
}

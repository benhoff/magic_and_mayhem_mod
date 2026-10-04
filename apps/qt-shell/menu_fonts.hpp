#pragma once
#include <QFont>
#include <QByteArray>
#include <QString>
#include <memory>

class QWidget;
namespace mnm::assets { struct SftFont; }
namespace mnm::ui {
enum class MenuFontRole { Heading, Body, Tooltip, Yellow };
class MenuFonts;
using MenuFontSet = std::shared_ptr<const MenuFonts>;
// Offline conversion of owned SFT masks into a Qt-compatible outline font.
// Preserves glyph shape/origin; palette effects and engine text layout are separate.
QByteArray menuFontData(const mnm::assets::SftFont&, const QString& family);
// A completely absent font catalog permits synthetic fixtures to use one shared
// fallback. Partial/malformed catalogs fail; no partial registration survives.
MenuFontSet loadMenuFonts(const QString& root);
void installMenuFonts(QWidget*, MenuFontSet);
QFont menuFont(const QWidget*, MenuFontRole, double scale = 1.0);
}

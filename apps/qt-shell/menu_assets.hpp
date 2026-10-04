#pragma once
#include <QImage>
#include <QMap>
#include <QRect>

namespace mnm::ui {
using Sections = QMap<QString, QMap<QString, QString>>;
struct MenuAssets {
    Sections layout;
    Sections strings;
    QImage background;
};
// Bounded read-only installed menu inputs. Throws on missing/invalid inputs.
MenuAssets loadMenuAssets(const QString& root, const QString& directory, const QString& config,
                         const char* imageFormat = "JPG", const QSize& imageSize = QSize(800, 600),
                         const QString& backgroundName = QString(), const QString& imageFileName = QString());
Sections loadMenuLayout(const QString& root, const QString& directory, const QString& config);
QRect rectangle(const QString& text);
QString textLabel(const Sections& strings, const QString& textId);
QRect menuContentRect(const QSize& size);
QString menuButtonStyle();
}

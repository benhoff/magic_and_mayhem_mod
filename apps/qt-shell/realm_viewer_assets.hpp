#pragma once
#include <QPoint>
#include <QString>
#include <QVector>
#include <array>
namespace mnm::ui {
struct RealmCatalogRegion {int artworkNumber=1;QString name;QPoint flagPosition;};
// Offline display metadata only. Does not infer campaign IDs or availability.
std::array<QVector<RealmCatalogRegion>,3> loadRealmCatalog(const QString& root);
}

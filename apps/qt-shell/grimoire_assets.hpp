#pragma once
#include "menu_assets.hpp"
#include <QVector>
namespace mnm::ui {
struct GrimoireLevel {QStringList pages;QStringList dynamicLabels;};
struct GrimoireEntry {int section=0,icon=0;QString title;QMap<int,GrimoireLevel> levels;};
struct GrimoireChapter {QString title;bool sorted=false;QVector<GrimoireEntry> entries;};
struct GrimoireBook {QVector<GrimoireChapter> chapters;Sections layout;Sections tooltips;QMap<QString,QString> artwork;};
// Presentation catalog only; no campaign knowledge or recovered engine identity mapping.
GrimoireBook loadGrimoireBook(const QString& root);
}

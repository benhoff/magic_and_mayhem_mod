#pragma once
#include "menu_sprites.hpp"
#include <QRect>
namespace mnm::ui {
struct RealmRegionVisual {
    QImage mask,border;QRect maskRect,borderRect;
    QVector<QVector<QPoint>> paths;
    bool contains(const QPoint& point) const;
};
struct RealmFlagFrame {MenuSpriteFrame sprite;int ticks=1;};
struct RealmFlagAnimation {QVector<RealmFlagFrame> frames;bool loop=false;int duration=0;};
struct RealmViewerVisuals {
    std::array<QVector<RealmRegionVisual>,3> regions;
    QVector<RealmFlagAnimation> animations;
};
// Bounded owned PCX shapes/colour-keyed borders, FP paths and ANI display clips.
// Throws on incomplete or unsupported visual inputs. No campaign semantics.
RealmViewerVisuals loadRealmViewerVisuals(const QString& root,const MenuSpriteSheet& flags);
const MenuSpriteFrame& realmFlagFrame(const RealmFlagAnimation&,quint64 tick);
}

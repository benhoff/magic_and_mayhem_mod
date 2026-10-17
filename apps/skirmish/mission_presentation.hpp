#pragma once
#include "map_campaign.hpp"
#include <QImage>
#include <QPoint>
namespace mnm::skirmish {
struct TerrainStamp {
    QImage image;
    QPoint topLeft;
    int key=0;
};
struct MissionLayer {bool actor=false;unsigned index=0;};
// Diagnostic fixed-view projection with owned unshaded terrain images and native
// unit glyphs. Original sprite lighting/animation equivalence is not claimed.
class MissionPresentation {
    std::vector<TerrainStamp> terrain_;
    std::vector<game::Point> standing_;
  public:
    explicit MissionPresentation(const MapCampaignResources &);
    const auto &terrain() const {return terrain_;}
    const auto &standing() const {return standing_;}
    QPoint center(game::Point) const;
    QPoint fineCenter(game::Point) const;
    QPoint unitCenter(game::Point p) const {return center(p)-QPoint(0,20);}
    std::vector<MissionLayer> layers(const std::vector<game::Point>& actors) const;
    std::optional<game::Point> pick(QPoint,const std::vector<game::Point>& actors,const std::vector<unsigned>& widths={}) const;
};
}

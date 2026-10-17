#include "objective_definition.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <algorithm>
#include <map>
#include <stdexcept>
namespace mnm::skirmish {
namespace {
void need(bool ok, const char *why) {if(!ok) throw std::invalid_argument(why);}
void keys(const QJsonObject &o, QStringList expected) {
  expected.sort();need(o.keys()==expected,"unknown/missing mission field");
}
std::string text(const QJsonValue &v) {
  need(v.isString(),"mission string expected");return v.toString().toStdString();
}
unsigned integer(const QJsonValue &v, unsigned maximum) {
  need(v.isDouble() && v.toDouble()>=0 && v.toDouble()<=maximum && v.toDouble()==v.toInt(),"mission integer expected");
  return unsigned(v.toInt());
}
QJsonArray array(const QJsonValue &v,unsigned limit) {
  need(v.isArray() && unsigned(v.toArray().size())<=limit,"mission array expected/budget");return v.toArray();
}
QJsonObject object(const QJsonValue &v) {need(v.isObject(),"mission object expected");return v.toObject();}
}
game::MissionPlan decodeObjectiveDefinition(std::string_view bytes) {
  need(bytes.size()<=64*1024,"mission JSON byte budget");
  QJsonParseError error;const auto doc=QJsonDocument::fromJson(QByteArray(bytes.data(),int(bytes.size())),&error);
  need(error.error==QJsonParseError::NoError && doc.isObject(),"invalid mission JSON");
  const auto o=doc.object();keys(o,{"version","id","briefing","reward_spell","groups","objectives"});
  need(integer(o["version"],2)==2,"unsupported objective definition version");
  game::MissionPlan p;p.version=2;p.id=text(o["id"]);p.briefing=text(o["briefing"]);p.rewardSpell=text(o["reward_spell"]);
  const auto groups=array(o["groups"],8),nodes=array(o["objectives"],16);
  std::map<std::string,unsigned> groupIds,nodeIds;
  for(const auto value:groups) {const auto g=object(value);need(groupIds.emplace(text(g["id"]),unsigned(groupIds.size())).second,"duplicate mission group ID");}
  for(const auto value:nodes) {const auto n=object(value);need(nodeIds.emplace(text(n["id"]),unsigned(nodeIds.size())).second,"duplicate objective ID");}
  const auto after=[&](const QJsonValue &v) {
    std::vector<std::uint32_t> result;
    for(const auto d:array(v,16)) {const auto it=nodeIds.find(text(d));need(it!=nodeIds.end(),"unknown objective dependency");result.push_back(it->second);}
    return result;
  };
  for(const auto value:groups) {
    const auto g=object(value);keys(g,{"id","locations","after"});
    game::MissionGroup group;group.id=text(g["id"]);group.after=after(g["after"]);
    for(const auto location:array(g["locations"],8)) {
      const auto a=array(location,3);need(a.size()==3,"mission XYZ expected");
      group.locations.push_back({int(integer(a[0],32)),int(integer(a[1],32)),int(integer(a[2],32))});
    }
    p.groups.push_back(std::move(group));
  }
  for(const auto value:nodes) {
    const auto n=object(value);const auto kind=text(n["kind"]);
    game::MissionObjective node;node.id=text(n["id"]);node.text=text(n["text"]);node.after=after(n["after"]);
    if(kind=="survive") {
      keys(n,{"id","text","kind","ticks","after"});node.kind=game::ObjectiveKind::survive;node.ticks=integer(n["ticks"],4096);
    } else {
      keys(n,{"id","text","kind","target","after"});need(kind=="eliminate" || kind=="protect","unknown objective kind");
      node.kind=kind=="protect" ? game::ObjectiveKind::protect : game::ObjectiveKind::eliminate;
      const auto target=object(n["target"]);
      if(target.contains("group")) {
        keys(target,{"group"});const auto it=groupIds.find(text(target["group"]));need(it!=groupIds.end(),"unknown objective group");node.group=it->second;
      } else {
        keys(target,{"wizard"});const auto wizard=text(target["wizard"]);need(wizard=="player" || wizard=="rival","unknown protected wizard");
        node.target=wizard=="player" ? game::MissionTarget::player : game::MissionTarget::rival;
      }
    }
    p.nodes.push_back(std::move(node));
  }
  return p; // Geometry, graph and semantic validation follows in validateMissionPlan.
}
}

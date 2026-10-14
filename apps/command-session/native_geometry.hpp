#pragma once
#include "native_providers.hpp"
#include "../../reconstruction/pathfinding/route_line.hpp"
#include "../../reconstruction/pathfinding/route_world.hpp"
namespace mnm::command_session {
class NativeCreatureGeometry {
  Continuation &state_;
  const NativeCreatureProfile &profile_;
  std::vector<reconstruction::OccupancyCell> cells_;
  std::vector<int> rows_,layers_;
  reconstruction::CellValidityMapView view();
  void refresh();
public:
  NativeCreatureGeometry(Continuation &,const NativeCreatureProfile &);
  reconstruction::CreatureAcceptanceState actor(unsigned) const;
  reconstruction::MovementHelpers movement();
  reconstruction::CreatureAcceptanceHelpers acceptance();
  bool cell(const model::AutonomousCreature &,model::Cell);
  bool legal(const model::AutonomousCreature &,model::Cell);
  bool ground(const model::AutonomousCreature &,model::Cell);
  bool line(const model::AutonomousCreature &,model::Cell,model::Cell);
  bool reach(const model::AutonomousCreature &,const model::AutonomousCreature &,model::Cell);
  bool move(const model::AutonomousCreature &,model::Cell,model::Cell,unsigned &);
  int height(model::Cell,unsigned width=1);
  void reserve(const model::AutonomousCreature &,model::Cell,bool);
  void snap(model::AutonomousCreature &,model::Cell);
  std::shared_ptr<reconstruction::RouteWorldSnapshot> snapshot(const model::AutonomousCreature &,model::Cell);
};
} // namespace mnm::command_session

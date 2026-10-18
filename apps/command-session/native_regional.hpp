#pragma once
#include "native_geometry.hpp"
namespace mnm::command_session {
bool nativeRegionalLegal(NativeCreatureGeometry &,const model::AutonomousCreature &,model::Cell,int boundary);
void nativeRegionalAdjust(Continuation &,NativeCreatureGeometry &,const model::AutonomousCreature &,model::Cell &,int boundary);
void nativeRegionalCandidate(Continuation &,const NativeCreatureProfile &,NativeCreatureGeometry &,const model::AutonomousCreature &,model::Cell &,unsigned,model::Cell);
model::RegionalServices nativeRegionalServices(Continuation &,const NativeCreatureProfile &,NativeCreatureGeometry &,const model::AutonomousCreature &);
model::SegmentCompletionEnvironment nativeRegionalEnvironment(const Continuation &);
model::RegionalQuery nativeRegionalQuery(const NativeCreatureGeometry &,const model::AutonomousCreature &,const model::ActorRegionQuery &);
} // namespace mnm::command_session

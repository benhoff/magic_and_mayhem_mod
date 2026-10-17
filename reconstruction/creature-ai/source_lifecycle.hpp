#pragma once
#include "source_owned.hpp"
namespace mnm::reconstruction::combat {
// Original407780/407820 topology operations. Indices replace pointers; occupied
// traversal positions are adjusted on removal, including exhausted cursors.
void detachSourceChild(SourceGraph &, unsigned parent, unsigned child);
void clearSourceChildren(SourceGraph &, unsigned parent);
struct OwnerSourceStorage {
  SourceControllerState sources;
  SourceManager manager;
  SourcePendingQueues pending;
  std::vector<unsigned> pendingRoots;
};
void validateOwnerSourceStorage(const OwnerSourceStorage &, unsigned actors);
std::vector<unsigned> ownerSourceMembers(const SourceGraph &, unsigned root,
                                         int mode);
// Stable task handles are indices into recipientRoot; UINT32_MAX marks retired
// handles. No allocator addresses or native task objects are shared with PE
// code.
void bindOwnerSourceTask(OwnerSourceStorage &, unsigned root, unsigned task);
void unbindOwnerSourceTask(OwnerSourceStorage &, unsigned task);
struct OwnerSourceTopologyChange {
  unsigned root = 0;
  std::vector<unsigned> retiredTasks;
};
// Selected423590 commit projections. Candidate generation/scoring/admission
// belong to the caller; these operations do not decide whether a proposal wins.
OwnerSourceTopologyChange mergeOwnerSources(OwnerSourceStorage &, unsigned left,
                                            unsigned right);
OwnerSourceTopologyChange
splitOwnerSource(OwnerSourceStorage &, AutonomousWorld &, unsigned root,
                 const std::vector<unsigned> &movedLeaves);
// Whole417580 decision/order projection; original deque/vector storage services
// are represented by owned containers. Current/other planning ancestors protect
// empty sources. Pruned nodes retain tombstone indices for portable identity.
std::vector<unsigned> pruneOwnerSources(OwnerSourceStorage &, AutonomousWorld &,
                                        std::optional<unsigned> current = {},
                                        std::optional<unsigned> other = {});
} // namespace mnm::reconstruction::combat

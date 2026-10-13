#include "order_markers.hpp"
namespace mnm::reconstruction::combat {
void updateQueuedMarkers(const AutonomousCreature &a, CreatureExecution &e,
                         int localOwner, const QueuedMarkerServices &s) {
  if (a.ai.owner != localOwner)
    return;
  for (auto &order : e.orders) {
    if (order.kind != 2 || order.effect)
      continue;
    QueuedMarkerParameters p{};
    p[0] = p[3] = unsigned(order.destination.x) * 32u + 16u;
    p[1] = p[4] = unsigned(order.destination.y) * 32u + 16u;
    p[2] = p[5] = unsigned(order.destination.z) * 16u;
    p[6] = a.ai.slot;
    p[7] = p[8] = p[9] = UINT32_MAX;
    p[10] = p[12] = 1;
    p[11] = !(order.destination == order.referenceDestination);
    order.effect = s.allocate();
    if (order.effect)
      s.initialize(*order.effect, p);
  }
}
} // namespace mnm::reconstruction::combat

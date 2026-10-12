#include "segment_completion.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int word(unsigned n) { return std::int32_t(n); }
void addContribution(SegmentCompletionState &p, unsigned root,
                     unsigned amount) {
  unsigned index = root, steps = 0;
  while (p.contributionNodes.at(index).parent) {
    if (++steps > p.contributionNodes.size())
      throw std::invalid_argument("cyclic owned contribution chain");
    index = *p.contributionNodes.at(index).parent;
  }
  if (auto owner = p.contributionNodes.at(index).owner)
    p.contributionTotals.at(*owner) += amount;
}
} // namespace
void snapCreaturePosition(AutonomousCreature &a, Cell p, unsigned width,
                          const PositionSnapServices &s) {
  a.ai.position = p;
  a.finePosition = {word(unsigned(p.x) * 32u), word(unsigned(p.y) * 32u),
                    word(unsigned(p.z) * 16u)};
  if (width == 2)
    a.finePosition.z =
        word(unsigned(a.finePosition.z) + unsigned(s.largeHeight(p)));
  else if (auto kind = s.tileKind(p))
    a.finePosition.z =
        word(unsigned(a.finePosition.z) + unsigned(s.tileHeight(kind)));
}
void completeMotionSegment(AutonomousCreature &a, CreatureExecution &e,
                           MotionActionState &m, MovementBehaviorState &landing,
                           SegmentCompletionState &p,
                           const SegmentCompletionEnvironment &env,
                           const SegmentCompletionServices &s) {
  if (p.count > p.route.size() || p.cursor >= p.count)
    throw std::invalid_argument("owned segment route cursor/count");
  if (a.block71c && e.mode10c == 3) {
    if (p.landing110 || p.landingOverlay71d) {
      a.flag104 = 0;
      if (a.animationKind == 3 && p.landingOverlay71d) {
        a.animationKind = 0;
        p.overlayFrame809 = 0;
        s.stopOverlay(a);
        if (p.overlayLink805) {
          s.releaseOverlay(*p.overlayLink805);
          p.overlayLink805.reset();
        }
      }
      p.landingOverlay71d = false;
    }
    a.block71c = 0;
    unsigned damage =
        a.ai.type == 15 ? a.ai.health
                        : s.fallDamage(a, landing.landingKind, a.ai.position.z);
    s.defended(a, damage, -1, false);
    if (!a.ai.active)
      return;
  }
  m.heightStep = m.heightDelta = 0;
  a.ai.motion = 0;
  s.snap(a, p.route[p.cursor].position, a.ai.type);
  if (p.scorePendingFc) {
    p.scorePendingFc = false;
    if (!env.scoreDenominator)
      throw std::invalid_argument("owned finite segment scoring ratio");
    float ratio =
        static_cast<float>(static_cast<long double>(env.scoreNumerator) /
                           static_cast<long double>(env.scoreDenominator));
    unsigned amount = s.score(a.ai.type, ratio);
    for (auto root : p.contributionRoots)
      if (root)
        addContribution(p, *root, amount);
  }
  s.environment(a);
  s.animation(a);
  s.terrain(a);
  if (!a.ai.active)
    return;
  ++p.cursor;
  if (p.cursor >= p.count) {
    a.ai.marker = 0;
    if (env.regionContinuation && p.regionPending) {
      if (p.regionCursor >= p.regionPath.size())
        throw std::invalid_argument("owned region path cursor");
      int index = p.regionPath[p.regionCursor];
      if (index < 0 || unsigned(index) >= env.regions.size())
        p.regionPending = false;
      else {
        auto candidate = env.regions[unsigned(index)].position;
        s.regionCandidate(a, candidate, env.regions[unsigned(index)].parameter,
                          a.destination);
        unsigned lookahead = p.count + 2;
        bool repeated = lookahead < p.route.size() &&
                        p.route[lookahead].position == candidate;
        if (candidate == a.ai.position || repeated || !s.legal(a, candidate))
          ++p.regionCursor;
        else
          p.regionPending = false;
      }
    }
  }
  if (std::int32_t(e.block963) > 0) {
    --e.block963;
    if (!e.block963) {
      a.blockDE3 = 0;
      e.block962 = 0;
      s.reset(a);
    }
  }
}
} // namespace mnm::reconstruction::combat

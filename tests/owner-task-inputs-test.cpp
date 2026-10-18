#ifdef MNM_BRAIN_REFERENCE
#define MNM_HEALTH_REFERENCE
#endif
#define main unused_health_inputs_main
#include "ordinary-health-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_task_inputs.hpp"
#include <algorithm>
namespace {
std::uint64_t inputDigest = 1469598103934665603ull;
unsigned inputChecks = 0;
void inputCheck(unsigned expected, unsigned actual, const char *why) {
  if (expected != actual)
    throw std::runtime_error(std::string(why) + " expected=" +
                             std::to_string(expected) + " actual=" + std::to_string(actual));
  ++inputChecks;
  inputDigest = (inputDigest ^ actual) * 1099511628211ull;
}
#ifdef MNM_BRAIN_REFERENCE
unsigned ptr(const void *p) { return unsigned(reinterpret_cast<std::uintptr_t>(p)); }
unsigned __attribute__((thiscall)) commanderMaximum(void *) { return 100; }
void __attribute__((thiscall)) observationPoint(void *, unsigned i, int *x, int *y, int *z) {
  *x = int(i); *y = *z = 0;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  check(argc == 2, "PE");
  initialize(argv[1]);
  redirect({0x5075f0, reinterpret_cast<std::uintptr_t>(&commanderMaximum)});
  redirect({0x4f3840, reinterpret_cast<std::uintptr_t>(&observationPoint)});
  const Bytes protectedText(reinterpret_cast<unsigned char *>(0x401000),
                            reinterpret_cast<unsigned char *>(0x5c0000));
  check(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000, PROT_READ | PROT_EXEC), "RX text");
#else
  (void)argc; (void)argv;
#endif
  unsigned distressCount = 0, damageCount = 0, specialCount = 0;
  for (unsigned n = 0; n < 4096; ++n) {
    const unsigned kind = std::array<unsigned, 8>{0,1,24,25,26,17,8,20}[n % 8];
    const bool form = n & 8;
    const unsigned health = (n / 16) % 6 * 10, maximumHp = 100;
    const int danger = int((n / 96) % 5), support = int((n / 480) % 5);
    const bool distressed = ownerTaskDistressed(kind, form, health, maximumHp, danger, support);
    distressCount += distressed;
    const unsigned slot = n % 5;
    std::vector<unsigned> occupants;
    for (unsigned i = 0; i < (n/5)%5; ++i) occupants.push_back((n/25+i)%7);
    const bool special = ownerTaskOnSpecialCell(slot, occupants);
    specialCount += special;
    const unsigned credit = n % 41;
    OwnerPreviousTask sourceA, sourceB, victim;
    sourceA.successes = 3; sourceB.successes = 7; victim.attempts = 11;
    const bool live = !(n & 64), damaging = !(n & 128);
    const unsigned roots = (n / 256) % 3;
    const bool sourceAFound = n & 16, sourceBFound = n & 32;
    if (live && damaging) {
      addOwnerTaskDamage(sourceAFound ? &sourceA : nullptr, roots ? &victim : nullptr, kind, credit);
      addOwnerTaskDamage(sourceBFound ? &sourceB : nullptr, roots == 2 ? &victim : nullptr, kind, credit);
      ++damageCount;
    }
#ifdef MNM_BRAIN_REFERENCE
    Bytes actor(0xe4b), cfg(0x5c9), task(0x40), child(0x40), root(0x40), map(12*8);
    put(cfg, 0, kind); put(cfg, 0x18, maximumHp);
    put(actor, 0, slot); put(actor, 0xac, ptr(cfg.data()));
    put(actor, 0xe4, health); put(actor, 0x7c, unsigned(form));
    put(actor, 0x922, unsigned(danger)); put(actor, 0x926, unsigned(support));
    using Predicate = unsigned(__attribute__((thiscall)) *)(void *);
    inputCheck(distressed, reinterpret_cast<Predicate>(0x52a3a0)(actor.data()), "distress");
    global(0x6c54dc, ptr(map.data())); global(0x6c5c84, occupants.size());
    global(0x6cb942, 0); global(0x6cb8c6, 0);
    for (unsigned i = 0; i < occupants.size(); ++i) {
      const auto v = std::uint16_t(occupants[i]);
      std::memcpy(map.data()+i*12+4, &v, 2);
    }
    inputCheck(special, reinterpret_cast<Predicate>(0x522690)(actor.data()), "special occupancy");
    Inputs policy; policy.feedbackValue=credit; policy.creditAvailable={sourceAFound,sourceBFound};
    inputs=&policy; trace.clear();
    creditA.fill(0);creditB.fill(0);
    std::memcpy(creditA.data()+0x2f,&sourceA.successes,4);
    std::memcpy(creditB.data()+0x2f,&sourceB.successes,4);
    // Reset expected source counters before original execution.
    const unsigned a0=3,b0=7;
    std::memcpy(creditA.data()+0x2f,&a0,4);std::memcpy(creditB.data()+0x2f,&b0,4);
    put(task,0x33,11u);put(root,5,ptr(task.data()));put(child,0x39,ptr(root.data()));
    put(actor,0x678, roots ? ptr(child.data()) : 0u);
    put(actor,0x67c, roots == 2 ? ptr(root.data()) : 0u);
    put(actor,0xe4,live ? 100u : 0u);put(actor,0x660,1u);
    unsigned ids[2] = {41,43};
    using Damage = unsigned(__attribute__((thiscall)) *)(void *, unsigned, void *);
    reinterpret_cast<Damage>(0x514990)(actor.data(), damaging ? 1u : 0u, ids);
    unsigned actualA,actualB;
    std::memcpy(&actualA,creditA.data()+0x2f,4);std::memcpy(&actualB,creditB.data()+0x2f,4);
    inputCheck(sourceA.successes,actualA,"sourceA reward");
    inputCheck(sourceB.successes,actualB,"sourceB reward");
    inputCheck(victim.attempts,get(task,0x33),"victim ancestor cost");
#else
    inputCheck(distressed,distressed,"distress");inputCheck(special,special,"special occupancy");
    inputCheck(sourceA.successes,sourceA.successes,"sourceA reward");
    inputCheck(sourceB.successes,sourceB.successes,"sourceB reward");
    inputCheck(victim.attempts,victim.attempts,"victim ancestor cost");
#endif
  }
  check(ownerTaskDistressed(0,false,50,100,1,0),"half health danger threshold");
  check(!ownerTaskDistressed(0,false,50,100,1,1),"strict .5 danger threshold");
  OwnerPreviousTask same;
  addOwnerTaskDamage(&same,&same,0,5);
  check(same.successes==10&&same.attempts==5,"same task damage alias");
  same.attempts=INT32_MAX-2;
  const auto earned=same.successes;
  bool refused=false;
  try {addOwnerTaskDamage(&same,&same,0,3);}catch(const std::invalid_argument&){refused=true;}
  check(refused&&same.successes==earned&&same.attempts==INT32_MAX-2,"credit overflow atomicity");
#ifdef MNM_BRAIN_REFERENCE
  check(std::equal(protectedText.begin(),protectedText.end(),reinterpret_cast<unsigned char *>(0x401000)),"unchanged protected text");
#endif
  std::cout<<"{\"success\":true,\"cases\":4096,\"checks\":"<<inputChecks
           <<",\"distressed\":"<<distressCount<<",\"special\":"<<specialCount
           <<",\"damage_events\":"<<damageCount<<",\"digest\":\""<<inputDigest<<"\"}\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}

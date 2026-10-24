#pragma once
#include "owner_knowledge.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include <stdexcept>
namespace mnm::reconstruction::combat {
// 407c10 value projection for finite acyclic source forests. Source records own
// their child vectors; +35 borrows a snapshot or live actor. Missing/stale keys
// retain that pointer without reading it. Live supplies checked actor addresses.
// PE traversal cursor bytes remain the separate OwnerTaskSources contract.
template<class M, class Live>
bool resolveOwnerTaskSources(M &m, std::uint32_t root,
    const OwnerKnowledge &knowledge, unsigned owner, int tick, Extent dimensions,
    const std::array<std::array<std::uint8_t, 8>, 8> &allied, Live live) {
  using U = std::uint32_t;
  if (owner >= 8 || dimensions.x <= 0 || dimensions.y <= 0)
    throw std::invalid_argument("source resolution owner/dimensions");
  auto si = [](U v) { return v <= 0x7fffffffu ? int(v) : -1-int(~v); };
  std::vector<U> leaves, stack{root}, visited;
  while (!stack.empty()) {
    U p = stack.back(); stack.pop_back();
    if (!p || visited.size() >= 251 ||
        std::find(visited.begin(), visited.end(), p) != visited.end())
      throw std::invalid_argument("source resolution forest");
    visited.push_back(p);
    if (m.get32(p+0x35)) { leaves.push_back(p); continue; }
    U b=m.get32(p+0x29), e=m.get32(p+0x2d);
    if (e<b || (e-b)%4 || (e-b)/4>125)
      throw std::invalid_argument("source resolution children");
    for (U at=e; at!=b;) { at-=4; stack.push_back(m.get32(at)); }
  }
  bool usable=false;
  int count=0;
  for (U p:leaves) {
    m.set32(p+0x89,3);
    const auto &rows=knowledge.records[owner];
    auto row=std::find_if(rows.begin(),rows.end(),[&](const auto &v){
      return v.memory.key==si(m.get32(p+9));
    });
    if (row==rows.end() || row->markedTick!=tick) {
      m.set32(p+0x89,0); usable=true; continue;
    }
    U actor=live(row->memory.actor.slot);
    if (!actor) throw std::invalid_argument("source resolution live slot");
    m.set32(p+0x35,actor);
    U actorOwner=m.get32(actor+0x174);
    if (actorOwner>=8) throw std::invalid_argument("source resolution actor owner");
    if (!m.get32(actor+0xe4) || allied[owner][actorOwner]) {
      m.set32(p+0x89,0); continue;
    }
    usable=true; ++count;
    // Original coordinate helpers subtract raw cells; wrapping occurs after
    // each incremental average, not in the subtraction.
    for (auto field:{std::pair<U,int>{0xd,dimensions.x},{0x11,dimensions.y},{0x15,0}}) {
      U actorOffset=field.first==0xd?8:field.first==0x11?12:16;
      int before=si(m.get32(root+field.first));
      auto value=std::int64_t(before)+(std::int64_t(si(m.get32(actor+actorOffset)))-before)/count;
      if (field.second) { value%=field.second; if(value<0)value+=field.second; }
      m.set32(root+field.first,U(value));
    }
  }
  return usable;
}
} // namespace mnm::reconstruction::combat

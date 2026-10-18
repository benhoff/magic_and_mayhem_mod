#define MNM_SOURCE_DECISION_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-source-decision-test.cpp"
#pragma GCC diagnostic pop
#ifdef MNM_BRAIN_REFERENCE
namespace {
int __attribute__((thiscall)) proposalDistance(void *, void *, void *other) {
  return caseNumber & 4 ? 3 - int(currentSources->id(address(other))) : 0;
}
} // namespace
#endif
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "decision FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&sourceAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&sourceRelease));
  redirect(0x42a340, reinterpret_cast<void *>(&sourceAppend));
  redirect(0x4289e0, reinterpret_cast<void *>(&sourceInstall));
  redirect(0x416fe0, reinterpret_cast<void *>(&sourceRegister));
  redirect(0x42b7c0, reinterpret_cast<void *>(&sourceErase));
  redirect(0x418050, reinterpret_cast<void *>(&sourceTeardown));
  redirect(0x429270, reinterpret_cast<void *>(&proposalDistance));
  redirect(0x523240, reinterpret_cast<void *>(&decisionSpecial));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "decision RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, split = 0, merge = 0, kept = 0, none = 0;
  for (unsigned n = 0; n < 1024; ++n) {
    caseNumber = n;
    m::AutonomousWorld world;
    auto storage = fixture(0, world);
    storage.sources.graph.resize(4);
    storage.sources.information.resize(4);
    storage.pending = {};
    storage.pendingRoots.clear();
    const unsigned shape = n % 4, root = shape >= 2 ? 0 : 3;
    for (unsigned i = 0; i < 4; ++i) {
      auto &g = storage.sources.graph[i];
      g.parent.reset();
      g.children.clear();
      g.actor = i < 3 ? std::optional<unsigned>(i) : std::nullopt;
      g.activity = 3;
    }
    for (auto &a : world.creatures)
      a.ai.owner = 0;
    storage.manager.nodes = shape == 3 ? std::vector<unsigned>{0, 1, 2}
                                       : std::vector<unsigned>{0, 1, 2, 3};
    auto &children = storage.sources.graph[3].children;
    children = shape == 0   ? std::vector<unsigned>{0, 1, 2}
               : shape == 1 ? std::vector<unsigned>{0, 1}
               : shape == 2 ? std::vector<unsigned>{1, 2}
                            : std::vector<unsigned>{};
    for (auto id : children)
      storage.sources.graph[id].parent = 3;
    m::OwnerSourceDecisionInputs input;
    input.members = m::ownerSourceMembers(storage.sources.graph, root, 3);
    for (auto id : input.members)
      input.speeds.push_back(24 * (1 + ((n >> (2 * id)) % 3)));
    if (shape && shape < 3)
      input.otherMembers =
          m::ownerSourceMembers(storage.sources.graph, shape == 1 ? 2 : 3, 3);
    const bool grouped = shape < 2;
    input.splitFirst = grouped && (n & 8);
    input.weighted = n & 16;
    if (shape && shape < 3 && (n & 32)) {
      input.otherTask.emplace();
      input.otherTask->completed = n & 64;
      input.otherTask->weighted = n & 128;
      input.otherTask->expires = 200;
      input.otherTask->value = float(int((n / 9) % 3) - 1) * .75f;
      m::bindOwnerSourceTask(storage, shape == 1 ? 2 : 3, 0);
    }
    std::optional<m::OwnerPreviousTask> currentTask;
    if (n & 256) {
      currentTask.emplace();
      currentTask->completed = n & 512;
      currentTask->weighted = n & 128;
      currentTask->expires = 200;
      currentTask->value = float(int((n / 7) % 3) - 1) * .75f;
      m::bindOwnerSourceTask(storage, root, 1);
      if (!currentTask->completed)
        input.splitFirst = false;
    }
    m::OwnerSourceDecision state;
    m::OwnerTaskSelection selection;
    auto random = m::seedRandom(n + 17);
    unsigned nativeMask = 0, nativeEmitted = 0;
    std::vector<unsigned> nativeSaved;
    m::OwnerSourceDecisionServices svc;
    if (shape == 3)
      svc.chooseOther = [&](m::RandomState &rng) {
        auto chosen = m::chooseOwnerMergeCandidate(
            {{1, n & 4 ? 2 : 0}, {2, n & 4 ? 1 : 0}}, rng);
        input.otherMembers = {*chosen};
      };
    svc.select = [&](const auto &members, bool rootSource, float bias,
                     const auto *old, bool reject, int &b) {
      m::OwnerTaskSelectionServices lower;
      lower.count = 1;
      lower.root = rootSource;
      lower.prepare = [&](unsigned) {
        nativeMask = 0;
        nativeEmitted = 0;
        for (auto id : members)
          nativeMask |= 1u << id;
      };
      lower.next = [&](unsigned, int &budget) -> std::optional<float> {
        if (nativeEmitted || budget < 1)
          return {};
        ++nativeEmitted;
        --budget;
        return score(nativeMask);
      };
      lower.save = [&](unsigned) { nativeSaved = members; };
      lower.finish = [] {};
      return m::advanceOwnerTaskReplanning(selection, b, random, lower, old,
                                           100, {}, bias, true, reject);
    };
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(storage);
    if (currentTask) {
      auto *p = raw.tasks.data() + 64;
      const auto &t = *currentTask;
      put(p, 0x1b, bits(t.value));
      put(p, 0xc, t.started);
      put(p, 0x10, t.expires);
      p[0x15] = t.completed;
      p[0x1a] = t.weighted;
    }
    if (input.otherTask) {
      const auto &t = *input.otherTask;
      put(raw.tasks.data(), 0x1b, bits(t.value));
      put(raw.tasks.data(), 0xc, t.started);
      put(raw.tasks.data(), 0x10, t.expires);
      raw.tasks[0x15] = t.completed;
      raw.tasks[0x1a] = t.weighted;
    }
    for (unsigned j = 0; j < 3; ++j)
      put(raw.actors.data() + j * 0xe4b, 0x174, 0);
    global(0x6def58, address(raw.actors.data()));
    global(0x6def5c, 3);
    global(0x6df180, 3);
    put(raw.brain.data(), 0x14, 100);
    global(0x5faf54, address(raw.nil.data()));
    global(0x5faf58, address(raw.nil.data()));
    Bytes configs(3 * 0x5c9), object(0x160);
    std::array<unsigned, 4> table{
        {address(reinterpret_cast<void *>(&decisionPrepare)), 0,
         address(reinterpret_cast<void *>(&decisionNext)),
         address(reinterpret_cast<void *>(&decisionSave))}};
    std::array<unsigned, 1> list{{address(object.data())}};
    put(object.data(), 0, address(table.data()));
    put(raw.brain.data(), 8, address(list.data()));
    put(raw.brain.data(), 12, address(list.data() + 1));
    raw.brain[0x19] = 1;
    raw.brain[0x11dd3] = 1;
    raw.brain[0x11fe0] = 1;
    for (unsigned off : {0x11ea6u, 0x11f33u}) {
      auto *p = raw.brain.data() + off;
      for (unsigned j = 0; j < 8; ++j)
        p[0x41 + j] = 1;
    }
    for (unsigned j = 0; j < 3; ++j) {
      auto *a = raw.actors.data() + j * 0xe4b;
      auto *cfg = configs.data() + j * 0x5c9;
      put(a, 0xac, address(cfg));
      put(a, 0xde7, 0);
      put(cfg, 0x589, 24 * (1 + ((n >> (2 * j)) % 3)));
    }
    global(0x643a80 + 12, 0);
    global(0x5d865c, input.splitFirst ? 20 : 0);
    global(0x5d8650, input.weighted ? 2 : 0);
    global(0x6e1f64, 0);
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
    savedMembers.clear();
#endif
    bool done = false;
    for (unsigned tick = 0; !done && tick < 100; ++tick) {
      int budget = 1 + int((n + tick) % 4);
      const int grant = budget;
      done = m::advanceOwnerSourceDecision(state, selection, input, budget,
                                           random, svc);
      ++slices;
#ifdef MNM_BRAIN_REFERENCE
      int b[3] = {0, 0, grant};
      reinterpret_cast<void(__attribute__((thiscall)) *)(
          void *, void *, int *)>(0x423590)(raw.brain.data(),
                                            raw.nodes[root].data(), b);
      record(unsigned(budget), unsigned(b[2]), "decision budget");
      record(done, raw.brain[0x11dd3], "decision completion");
      record(unsigned(state.winner), get(raw.brain.data(), 0x11dd8),
             "automatic winner");
      record(bits(selection.value - (done ? state.winningBias : 0)),
             get(raw.brain.data(), 0x11ddd), "automatic value");
      if (!done)
        record(state.stage, get(raw.brain.data(), 0x11dd4), "decision stage");
      record(random.left, get(reinterpret_cast<void *>(0x6a29d8), 0),
             "decision RNG left");
      record(random.right, get(reinterpret_cast<void *>(0x6a29dc), 0),
             "decision RNG right");
      for (unsigned k = 0; k < 250; ++k)
        record(random.words[k], get(reinterpret_cast<void *>(0x6a29e0), k * 4),
               "decision RNG word");
      need(nativeSaved == savedMembers, "decision saved member order");
#else
      (void)grant;
      for (unsigned v :
           {unsigned(budget), unsigned(done), unsigned(state.winner),
            bits(selection.value - (done ? state.winningBias : 0))})
        record(v, v, "native decision");
      if (!done)
        record(state.stage, state.stage, "native stage");
      for (unsigned v : {random.left, random.right})
        record(v, v, "native RNG");
      for (auto v : random.words)
        record(v, v, "native RNG word");
#endif
    }
    need(done, "decision slice bound");
    const bool installed = state.winner >= 0 &&
                           m::replaceOwnerSourceTask(
                               selection.value - state.winningBias,
                               currentTask ? &*currentTask : nullptr, 100, {});
#ifdef MNM_BRAIN_REFERENCE
    record(installed, raw.installed != UINT32_MAX,
           "current-task replacement commit");
#else
    record(installed, installed, "current-task replacement commit");
#endif
    split += state.winner == 4;
    merge += state.winner == 2;
    kept += state.winner == 0;
    none += state.winner == -1;
  }
  need(split && merge && kept, "automatic decision witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "decision text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":1024,\"slices\":" << slices
            << ",\"checks\":" << checks << ",\"splits\":" << split
            << ",\"merges\":" << merge << ",\"kept\":" << kept
            << ",\"none\":" << none << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << "case " << caseNumber << ": " << e.what() << '\n';
  return 1;
}

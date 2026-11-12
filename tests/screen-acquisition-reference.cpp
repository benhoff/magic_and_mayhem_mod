#include "../reconstruction/rendering/screen_acquisition.hpp"
#include "../reconstruction/rendering/screen_resource_ownership.hpp"
#include "../reconstruction/rendering/world_screen_retirement.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#define main unused_sprite_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using U = std::uint32_t;
using V = std::vector<U>;
constexpr U Base = 0x720000, Size = 0x40000, Main = Base + 0x100, World = Base + 0x1000,
            Report = World + 0xfdef, Heap = Base + 0x14000;
constexpr U GlobalBase = 0x5c0000, GlobalSize = Base - GlobalBase;
constexpr U Globals[] = {
    0x5da330, 0x5eab18, 0x5fed7c, 0x5fc864, 0x5fc86c, 0x634754, 0x656620, 0x656624, 0x656634,
    0x659e2f, 0x68991c, 0x689920, 0x689924, 0x689938, 0x6905ac, 0x6c3778, 0x6dbe95, 0x6dbe99,
    0x6de6d5, 0x6de6e5, 0x6de6fd, 0x6e007c, 0x6e1ac8, 0x6e1acc, 0x6e1ef4, 0x6e1f0c, 0x6e1f30,
    0x6e2031, 0x6e2034, 0x6e2038, 0x6e9858, 0x6f694c, 0x5e15d8, 0x5e15dc, 0x5e15e0, 0x5e15e4,
    0x5e15e8, 0x5e15ec, 0x6de6d9, 0x5d9e38, 0x689498, 0x68949c, 0x6def58, 0x6def60, 0x6def64,
    0x6b1248, 0x6b124c, 0x6e1f94, 0x6e1f98, 0x6e266c, 0x6de558, 0x6de55c, 0x6de560, 0x6de564,
    0x6de568, 0x6de56c, 0x6de570, 0x6de574, 0x6de578, 0x6a28f8, 0x6a28fc, 0x6a2900, 0x6a2904,
    0x6a2908, 0x6a290c, 0x6a2910, 0x6a2914, 0x6a2918, 0x6903e8, 0x6e1f9c};
static void need(bool b, const char *why) {
    if (!b)
        throw std::runtime_error(why);
}
static const std::map<U, std::string> literals = {
    {0x5d9eac, "sprites\\selcur.spr"},
    {0x5d9e98, "sprites\\markers.spr"},
    {0x5d9e84, "sprites\\lines.spr"},
    {0x5d9e64, "map seed: %d"},
    {0x5e05dc, "Interface\\MainScreen\\screen (MainMenu).cfg"},
    {0x5e05c0, "c:\\Projects\\Chaos\\game.cpp"},
    {0x5e05a8, "Interface\\MainScreen\\"},
    {0x5da418, "%s%ix%i\\"},
    {0x5d9f10, "BackgroundFile"},
    {0x5da5b4, ".JPG"},
    {0x5da5a8, "%s %i-%i%s"},
    {0x5e0598, "Version %.3f"},
    {0x5da424, "c:\\Projects\\Chaos\\BattlEnd.cpp"},
    {0x5c5e20, "Interface\\BattleEnd\\"},
    {0x5da410, "Victory"},
    {0x5da408, "Defeat"},
    {0x5da400, ".jpg"},
    {0x5da3e4, "Battle %s Screen %i-%i%s"},
    {0x5da3d0, "Interface\\SpellBox\\"},
    {0x5da3c4, "MItems.spr"},
    {0x5da3bc, "%s.cfg"},
    {0x5da3ac, "cfg\\MItems.cfg"},
    {0x5da3a4, "  ( "},
    {0x5da3a0, " )"},
    {0x5c5e38, "screen (Battle End - Victory).cfg"},
    {0x5c5e5c, "screen (Battle End - Defeat).cfg"},
    {0x5da398, "%s%s"},
    {0x5da394, "/ "}};
struct Memory {
    std::vector<unsigned char> bytes = std::vector<unsigned char>(GlobalSize + Size);
    bool original = false;
    unsigned char *at(U a, U n = 4) {
        need(a >= GlobalBase && a - GlobalBase <= bytes.size() - n,
             "Unadmitted acquisition storage");
#ifdef MNM_ORIGINAL_REFERENCE
        if (original)
            return reinterpret_cast<unsigned char *>(a);
#endif
        return bytes.data() + a - GlobalBase;
    }
    U get(U a) {
        U v;
        std::memcpy(&v, at(a), 4);
        return v;
    }
    void set(U a, U v) { std::memcpy(at(a), &v, 4); }
    U getByte(U a) { return *at(a, 1); }
    void byte(U a, U v) { *at(a, 1) = v; }
    std::vector<unsigned char> snapshot() {
        std::vector<unsigned char> v(Size + sizeof(Globals));
        std::memcpy(v.data(), at(Base, Size), Size);
        for (U i = 0; i < std::size(Globals); ++i)
            std::memcpy(v.data() + Size + i * 4, at(Globals[i]), 4);
        return v;
    }
};
static Memory *memory;
static U profile, mutation, nextAllocation, scopeOwner;
static bool changed;
struct Call {
    U fn, receiver;
    V args;
    std::string text;
    V delta;
    bool operator==(const Call &o) const {
        return fn == o.fn && receiver == o.receiver && args == o.args && text == o.text &&
               delta == o.delta;
    }
};
static std::vector<Call> calls;
static std::vector<unsigned char> previous;
static void event(U fn, U receiver = 0, V args = {}, std::string text = {}) {
    const auto s = memory->snapshot();
    V delta;
    for (U page = 0; page < s.size(); page += 64) {
        const U end = std::min<U>(page + 64, s.size());
        if (std::memcmp(previous.data() + page, s.data() + page, end - page))
            for (U i = page; i < end; ++i)
                if (previous[i] != s[i]) {
                    delta.push_back(i);
                    delta.push_back(s[i]);
                }
    }
    previous = s;
    calls.push_back({fn, receiver, args, text, delta});
    if (!changed && mutation == 1 && fn == 0x58ad90) {
        memory->set(0x6de6d5, !memory->get(0x6de6d5));
        changed = true;
    }
    if (!changed && mutation == 2 && fn == 0x4cb040) {
        const U old = memory->get(scopeOwner + 0x4b), alt = Base + 0x13000;
        std::memcpy(memory->at(alt, 0x54), memory->at(old, 0x54), 0x54);
        memory->set(scopeOwner + 0x4b, alt);
        changed = true;
    }
    if (!changed && mutation == 3 && fn == 0x4dc5e0) {
        memory->set(0x689920, 1);
        memory->set(0x6e1f0c, 0);
        memory->set(World + 0x10317, 1);
        changed = true;
    }
}
struct Services {
    std::string str(U a) {
        const auto it = literals.find(a);
        if (it != literals.end())
            return it->second;
        std::string s;
        while (auto c = memory->getByte(a++)) {
            need(s.size() < 511, "Bounded string");
            s += char(c);
        }
        return s;
    }
    void storeText(U a, const std::string &s) {
        need(s.size() < 256, "Text capacity");
        std::memcpy(memory->at(a, s.size() + 1), s.c_str(), s.size() + 1);
    }
    U allocate(U size) {
        event(0x597850, 0, {size});
        U a = Heap + nextAllocation;
        nextAllocation += (size + 15) & ~15u;
        need(nextAllocation < 0x20000, "Fixture allocation bound");
        return a;
    }
    void free(U a) { event(0x597870, 0, {a}); }
    void destruct(U fn, U r) { event(fn, r); }
    void dispose(U r, U flags) { event(0x4bc980, r, {flags}); }
    void destroyArray(U a, U stride, U count, U fn) { event(0x59bf07, 0, {a, stride, count, fn}); }
    void constructArray(U a, U stride, U count, U ctor, U dtor) {
        event(0x59c441, 0, {a, stride, count, ctor, dtor});
        for (U i = 0; i < count; ++i)
            std::memset(memory->at(a + i * stride, stride), 0, stride);
    }
    void freeArray(U a) { event(0x5979b0, 0, {a}); }
    void bitmapPlane(U a, U b) { event(0x57d0d0, a, {b}); }
    void layout(U filename, U dest, U kind, U index) {
        event(0x558580, filename, {dest, kind, index});
        memory->set(dest, 100 + index);
        storeText(dest + 0x24, "control" + std::to_string(index));
        memory->byte(dest + 0x138, 17 + index);
    }
    void loadLayout(U owner, U filename) {
        mnm::reconstruction::acquireOriginalScreenLayout(*memory, *this, owner, filename,
                                                         owner == Report);
    }
    void loadLayoutText(U owner, const std::string &filename) {
        event(0x474a50, owner, {}, filename);
        mnm::reconstruction::acquireOriginalScreenLayout(*memory, *this, owner, Base + 0x12f00,
                                                         true);
    }
    void surface(U owner, const std::array<U, 27> &d, U file, U line) {
        V a{file, line};
        a.insert(a.end(), d.begin(), d.end());
        event(0x58ad90, owner, a);
    }
    template <class... A> std::string format(U fmt, A... args) {
        char out[512];
        const auto text = str(fmt);
        const int n = std::snprintf(out, sizeof(out), text.c_str(), args...);
        need(n >= 0 && n < 512, "Formatting capacity");
        event(0x59c1fd, fmt, {}, out);
        return out;
    }
    double versionScale() { return double(0.0010000000474974513f); }
    std::string path(U key, U filename) {
        event(0x559230, key, {filename});
        return "Background";
    }
    U text(U fn, U receiver, const std::string &s) {
        event(fn, receiver, {}, s);
        return (fn == 0x58d320) ? U(!(profile & 4)) : 1;
    }
    U sprite(U receiver, const std::string &s, U mode) {
        event(0x57d310, receiver, {mode}, s);
        return U(!(profile & 8));
    }
    U reportConfig(U owner, const std::string &s, U n) {
        event(0x4745e0, owner, {n}, s);
        storeText(memory->get(owner + 0x257), "Player");
        storeText(memory->get(owner + 0x25b), "Realm");
        return U(!(profile & 16));
    }
    std::string decimal(U value) {
        const auto s = std::to_string(static_cast<std::int32_t>(value));
        need(s.size() <= 6, "Admitted original decimal scratch capacity");
        event(0x59de2f, 0, {value, 10}, s);
        return s;
    }
    void packet(U r, const std::array<U, 4> &a, U channel) {
        V v(a.begin(), a.end());
        v.push_back(channel);
        event(0x537c70, r, v);
    }
    void disable(U r) { event(0x4ca980, r); }
    U call(U fn, U receiver = 0, std::initializer_list<U> a = {}) {
        V args(a);
        if (fn == 0x4d2970)
            args[0] &= 255;
        event(fn, receiver, args);
        if (fn == 0x4cc720) {
            memory->set(receiver, 0x5c5f24);
            memory->set(receiver + 0x45, (profile & 2) ? Base + 0x12d00 : 0);
            return receiver;
        }
        if (fn == 0x4d27b0)
            return receiver;
        if (fn == 0x4a0d40) {
            memory->set(receiver + 4, profile % 5);
            return receiver;
        }
        if (fn == 0x4a0e10)
            return Base + 0x12000 + *a.begin() * 0x110;
        if (fn == 0x56c700)
            return (profile & 1) ? 0xfffffffeu : 123;
        if (fn == 0x474b60)
            return Base + 0x12e00;
        if (fn == 0x4e8b70)
            return profile & 1 ? 0xffffffffu : 1001;
        if (fn == 0x57d310)
            return U(!(profile & 8));
        if (fn == 0x4a5000)
            return 0xa1b2c3d4;
        if (fn == 0x5c52e8)
            return 0xcafefeed;
        return 0;
    }
};
struct RetireServices : Services {
    U call(U fn, U r = 0, U a = 0) {
        if (fn == 0x4711d0 || fn == 0x56d0a0 || fn == 0x58bc10)
            return Services::call(fn, r, {a});
        return Services::call(fn, r, {});
    }
};
#ifdef MNM_ORIGINAL_REFERENCE
#define MEMBER __attribute__((thiscall, force_align_arg_pointer))
#define PLAIN __attribute__((force_align_arg_pointer))
static void adapt(U a, void *f) {
    auto *p = (unsigned char *)a;
    p[0] = 0xe9;
    const U delta = U(f) - a - 5;
    std::memcpy(p + 1, &delta, 4);
}
template <U F> static U MEMBER member(void *r) { return Services{}.call(F, U(r)); }
template <U F> static U MEMBER one(void *r, U a) { return Services{}.call(F, U(r), {a}); }
template <U F> static U MEMBER two(void *r, U a, U b) { return Services{}.call(F, U(r), {a, b}); }
template <U F> static U MEMBER five(void *r, U a, U b, U c, U d, U e) {
    return Services{}.call(F, U(r), {a, b, c, d, e});
}
template <U F> static U PLAIN plain() { return Services{}.call(F); }
template <U F> static U MEMBER text(void *r, const char *s) { return Services{}.text(F, U(r), s); }
static U MEMBER warning(const char *s) { return Services{}.text(0x49e4e0, 0, s); }
static U PLAIN allocated(U n) { return Services{}.allocate(n); }
static void PLAIN freed(U a) { Services{}.free(a); }
static void PLAIN freedArray(U a) { Services{}.freeArray(a); }
static void MEMBER surface(void *r, const U *d, U file, U line) {
    std::array<U, 27> v;
    std::copy(d, d + 27, v.begin());
    Services{}.surface(U(r), v, file, line);
}
static U MEMBER sprite(void *r, const char *s, U n) { return Services{}.sprite(U(r), s, n); }
static void __attribute__((stdcall, force_align_arg_pointer)) constructed(U a, U s, U n, U c, U d) {
    Services{}.constructArray(a, s, n, c, d);
}
static void __attribute__((stdcall, force_align_arg_pointer)) destroyed(U a, U s, U n, U d) {
    Services{}.destroyArray(a, s, n, d);
}
static void __attribute__((fastcall, force_align_arg_pointer)) layout(U f, U dest, U kind,
                                                                      U index) {
    Services{}.layout(f, dest, kind, index);
}
static void __attribute__((fastcall, force_align_arg_pointer)) path(U key, U filename, char *out) {
    const auto s = Services{}.path(key, filename);
    std::strcpy(out, s.c_str());
}
static int PLAIN formatted(char *out, U fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    const int n = std::vsnprintf(out, 512, (const char *)fmt, ap);
    va_end(ap);
    need(n >= 0 && n < 512, "Original formatting bound");
    event(0x59c1fd, fmt, {}, out);
    return n;
}
static char *PLAIN decimal(U value, char *out, U radix) {
    need(radix == 10, "Radix");
    const auto s = Services{}.decimal(value);
    std::strcpy(out, s.c_str());
    return out;
}
static U MEMBER config(void *r, const char *s, U n) { return Services{}.reportConfig(U(r), s, n); }
static void MEMBER layoutText(void *r, const char *s) {
    event(0x474a50, U(r), {}, s);
    using F = void(__attribute__((thiscall)) *)(void *, U);
    reinterpret_cast<F>(0x474a50)(r, Base + 0x12f00);
}
static void MEMBER disabled(void *r) { Services{}.disable(U(r)); }
static void MEMBER packet(void *r, const U *a, U n) {
    std::array<U, 4> v;
    std::copy(a, a + 4, v.begin());
    Services{}.packet(U(r), v, n);
}
static U MEMBER seven(void *r, U a, U b, U c, U d, U e, U f, U g) {
    return Services{}.call(0x56f000, U(r), {a, b, c, d, e, f, g});
}
static U MEMBER eight(void *r, U a, U b, U c, U d, U e, U f, U g, U h) {
    return Services{}.call(0x57daa0, U(r), {a, b, c, d, e, f, g, h});
}
static void MEMBER disposed(void *r, U flags) { Services{}.dispose(U(r), flags); }
static void MEMBER plane(void *r, U p) { Services{}.bitmapPlane(U(r), p); }
static U __attribute__((stdcall, force_align_arg_pointer)) tick() {
    return Services{}.call(0x5c52e8);
}
#endif
static Memory input(U seed) {
    Memory m;
    std::fill(m.bytes.begin(), m.bytes.end(), seed);
    for (U a : Globals)
        m.set(a, 0);
    m.set(0x6de6d5, profile & 1);
    m.set(0x6de6fd, profile & 2 ? 0xffffffecu : 20);
    m.set(0x68991c, profile & 4);
    m.set(0x689920, profile % 6);
    m.set(0x6e1f0c, profile & 8);
    m.set(0x6e1f30, profile & 1);
    m.set(0x5fed7c, profile & 2);
    m.set(0x6c3778, profile & 16 ? 0xffffffffu : 5);
    m.set(0x656634, profile % 4);
    m.set(0x6de6e5, 77);
    m.set(0x689924, profile & 1 ? 0xffffffffu : 3);
    m.byte(0x6e2031, profile & 2);
    m.byte(0x6e1ef4, profile & 16);
    m.byte(0x5eab18, profile & 8);
    m.set(0x6e1ac8, 0xfedcba98);
    m.set(0x6e1acc, 0x12345678);
    m.set(0x5da330, 91);
    m.set(Main, 0x5c63e4);
    m.set(Main + 8, 0);
    m.set(Main + 0x4b, Base + 0x12800);
    m.set(Main + 0x47, profile & 16 ? Base + 0x12c00 : 0);
    m.set(Base + 0x12c00, 0x5c6414);
    m.set(Main + 0x23, 0);
    m.set(Main + 0x27, 0);
    m.set(Report, 0x5c5ebc);
    m.set(Report + 0x53, profile & 1);
    m.set(Report + 0x23, 0);
    m.set(Report + 0x27, 0);
    m.set(World + 0x100df, profile & 1 ? Base + 0x12d00 : 0);
    m.byte(Base + 0x12d00, profile & 2);
    m.byte(Base + 0x12d01, profile & 4);
    m.set(World + 0x10317, profile & 2);
    m.set(World + 0xfc52, 0);
    m.byte(World + 0x100a4, profile & 8);
    Services s;
    auto *prior = memory;
    memory = &m;
    s.storeText(0x6dde58, "scenario");
    s.storeText(Report + 0x157, "Defeat explanation");
    s.storeText(0x6ead98, "unused");
    s.storeText(Base + 0x12e00, "Next campaign");
    for (U i = 0; i < 4; ++i) {
        s.storeText(0x658f10 + i * 0x133, (profile + i) % 3 ? "player" : "unused");
        m.set(Base + 0x12000 + i * 0x110, (i & 1) ? 0xfffffff4u : 30 + i);
        s.storeText(Base + 0x12004 + i * 0x110, "Score" + std::to_string(i));
        m.set(Base + 0x12104 + i * 0x110, (profile + i) % 3 ? 0xffffffffu : i);
    }
    // Lifecycle retirement providers are authored endpoints. Empty admitted queues
    // keep the composed test focused on acquisition/publication and release links.
    m.set(0x5d9e38, 0xffffffffu);
    m.set(0x689498, 0);
    m.set(0x68949c, 0);
    m.set(0x6b1248, 0);
    m.set(0x6b124c, 0);
    memory = prior;
    return m;
}
static void reset(Memory &m) {
    memory = &m;
    nextAllocation = 0;
    changed = false;
    calls.clear();
    previous = m.snapshot();
}
static void execute(Memory &m, bool original, U suite) {
    Services p;
#ifdef MNM_ORIGINAL_REFERENCE
    using Fn = void(__attribute__((thiscall)) *)(void *);
    auto invoke = [&](U pc, U r) { reinterpret_cast<Fn>(pc)((void *)r); };
#else
    (void)original;
#endif
    auto mainPrepare = [&] {
        scopeOwner = Main;
#ifdef MNM_ORIGINAL_REFERENCE
        if (original) {
            invoke(0x4a7110, Main);
            return;
        }
#endif
        mnm::reconstruction::prepareOriginalMainScreen(m, p, Main);
    };
    auto worldEnter = [&] {
        scopeOwner = World;
#ifdef MNM_ORIGINAL_REFERENCE
        if (original) {
            invoke(0x46a460, World);
            return;
        }
#endif
        mnm::reconstruction::enterOriginalWorldScreen(m, p, World);
    };
    auto reportPrepare = [&] {
        scopeOwner = Report;
#ifdef MNM_ORIGINAL_REFERENCE
        if (original) {
            invoke(0x473d70, Report);
            return;
        }
#endif
        mnm::reconstruction::prepareOriginalCampaignReport(m, p, Report);
    };
    auto release = [&](U owner, U table, U pc) {
#ifdef MNM_ORIGINAL_REFERENCE
        if (original) {
            invoke(pc, owner);
            return;
        }
#else
        (void)pc;
#endif
        need(mnm::reconstruction::releaseOriginalScreenResources(m, p, table, owner),
             "Release table");
    };
    if (suite == 0) {
        mainPrepare();
        mainPrepare();
        return;
    }
    if (suite == 1) {
        worldEnter();
        return;
    }
    if (suite == 2) {
        reportPrepare();
        return;
    }
#ifdef MNM_ORIGINAL_REFERENCE
    if (original)
        invoke(0x4a6a60, Main);
    else
#endif
        need(mnm::reconstruction::constructOriginalCoreMenu(m, p, 0x5c63e4, Main),
             "Construct Main");
    mainPrepare();
    need(m.get(Main + 8) == 1, "Main published initialized state");
    release(Main, 0x5c63e4, 0x4a7030);
    worldEnter();
    need(m.get(World + 8) == 1, "World published initialized state");
    const U reportFlag = m.get(Report + 8);
    reportPrepare();
    need(m.get(Report + 8) == reportFlag, "Report prepare does not publish initialized flag");
#ifdef MNM_ORIGINAL_REFERENCE
    if (original)
        invoke(0x4748a0, Report);
    else
#endif
        mnm::reconstruction::exitOriginalCampaignReport(m, p, Report);
    for (U o : {0x2a5u, 0x257u, 0x25bu, 0x4bu, 0x47u, 0x4fu, 0x3fu})
        need(!m.get(Report + o), "Report resource retired");
    // World entry's transitive services own simulation resources. Their authored
    // exit fixtures are explicitly cleared before running the local exit caller.
    for (U base : {0x6de558u, 0x6a28f8u})
        for (U i = 0; i < 36; i += 4)
            m.set(base + i, 0);
#ifdef MNM_ORIGINAL_REFERENCE
    if (original)
        invoke(0x46ab30, World);
    else
#endif
    {
        RetireServices exit;
        mnm::reconstruction::retireOriginalWorldScreen(m, exit, World);
    }
    mainPrepare();
#ifdef MNM_ORIGINAL_REFERENCE
    if (original)
        invoke(0x4a6b00, Main);
    else
#endif
        need(mnm::reconstruction::destroyOriginalCoreMenu(m, p, 0x5c63e4, Main), "Destroy Main");
    need(!m.get(Main + 8) && !m.get(Main + 0x4b) && !m.get(World + 8), "Composed owners retired");
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
    need(argc == 2, "Pinned executable required");
    map_image(read(argv[1]));
    need(mmap((void *)Base, Size, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0) != MAP_FAILED,
         "Owned map");
    std::array<unsigned char, 4096> thread{};
    user_desc segment{};
    segment.entry_number = U(-1);
    segment.base_addr = U(thread.data());
    segment.limit = 0xfffff;
    segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
    need(!syscall(SYS_set_thread_area, &segment), "Private SEH");
    unsigned short selector = (segment.entry_number << 3) | 3;
    asm volatile("movw %0, %%fs" ::"rm"(selector));
    for (const auto &[a, s] : literals)
        need(std::string((const char *)a) == s, "Original literal changed");
#define A(pc, fn) adapt(pc, (void *)fn)
    A(0x597850, allocated);
    A(0x597870, freed);
    A(0x5979b0, freedArray);
    A(0x59c441, constructed);
    A(0x59bf07, destroyed);
    A(0x558580, layout);
    A(0x559230, path);
    A(0x58ad90, surface);
    A(0x59c1fd, formatted);
    A(0x59de2f, decimal);
    A(0x4745e0, config);
    A(0x537c70, packet);
    A(0x56f000, seven);
    A(0x57daa0, eight);
    A(0x57d0d0, plane);
    A(0x49e4e0, warning);
#define M(pc) A(pc, member<pc>)
    M(0x4cc720);
    M(0x4d27b0);
    M(0x4ca910);
    M(0x4a0d40);
    M(0x56c700);
    M(0x4dc5e0);
    M(0x570440);
    M(0x52b2e0);
    M(0x52b360);
    M(0x52b3b0);
    M(0x52b400);
    M(0x52b450);
    M(0x52b4a0);
    M(0x52b4f0);
    M(0x4a08a0);
    M(0x470ee0);
    M(0x4dd010);
    M(0x46a370);
    M(0x4dc6e0);
    M(0x470410);
    M(0x58b3e0);
    M(0x475c10);
    M(0x4d27d0);
    M(0x49e860);
    M(0x4ffad0);
    M(0x52baf0);
    M(0x4943f0);
    M(0x4a5000);
    M(0x52a670);
    M(0x506d30);
    M(0x543c60);
    M(0x535970);
    M(0x4ff9a0);
    M(0x464ad0);
    M(0x5458b0);
    M(0x4ecbe0);
    M(0x573960);
    M(0x46a930);
#undef M
#define O(pc) A(pc, one<pc>)
    O(0x4cb030);
    O(0x4ca9b0);
    O(0x4d28b0);
    O(0x4d2960);
    O(0x4d2970);
    O(0x4d2930);
    O(0x4cabd0);
    O(0x4caf00);
    O(0x4a0e10);
    O(0x487a90);
    O(0x487b10);
    O(0x485a80);
    O(0x479a10);
    O(0x482010);
    O(0x4dde90);
    O(0x534ad0);
    O(0x4711d0);
    O(0x56d0a0);
    O(0x58bc10);
#undef O
#define T(pc) A(pc, two<pc>)
    T(0x4cad00);
    T(0x4dcee0);
    T(0x4de5a0);
    T(0x534aa0);
#undef T
    A(0x4cb0b0, five<0x4cb0b0>);
    A(0x58d320, text<0x58d320>);
    A(0x4cb040, text<0x4cb040>);
    A(0x4d28c0, text<0x4d28c0>);
    A(0x4d8cb0, text<0x4d8cb0>);
    A(0x57d310, sprite);
#define P(pc) A(pc, plain<pc>)
    P(0x4e8b70);
    P(0x474b60);
    P(0x549210);
    P(0x549690);
    P(0x549b00);
    P(0x549230);
    P(0x5496b0);
    P(0x549b20);
    P(0x4879f0);
#undef P
    // The report layout caller is executed unchanged. This table adapter captures
    // its stack-string argument before substituting a stable fixture filename ID.
    U fn = U((void *)layoutText);
    std::memcpy((void *)(0x5c5ebc + 0x1c), &fn, 4);
    fn = U((void *)disabled);
    std::memcpy((void *)(0x5c5f24 + 0x18), &fn, 4);
    fn = U((void *)disposed);
    std::memcpy((void *)(0x5c6414 + 4), &fn, 4);
    std::memcpy((void *)(0x5c5eec + 4), &fn, 4);
    fn = U((void *)tick);
    std::memcpy((void *)0x5c52e8, &fn, 4);
    const std::pair<U, U> ranges[] = {
        {0x4a6f70, 0x4a7024}, {0x4a7110, 0x4a756d}, {0x46a460, 0x46a92e}, {0x473d70, 0x4745c8},
        {0x474a50, 0x474b03}, {0x4a7030, 0x4a7107}, {0x473c10, 0x473d01}, {0x46ab30, 0x46ae8c},
        {0x4cacb0, 0x4cacf1}, {0x4a6a60, 0x4a6afd}, {0x4a6b00, 0x4a6b67}, {0x4cab50, 0x4cab69},
        {0x4cab70, 0x4cabcc}, {0x4748a0, 0x47493a}};
    std::vector<Bytes> bodies;
    for (auto r : ranges)
        bodies.emplace_back((unsigned char *)r.first, (unsigned char *)r.second);
#else
    (void)argv;
    need(argc == 1, "Native arguments");
#endif
    U cases = 0, journeys = 0;
    std::uint64_t checksum = 0;
    for (U suite = 0; suite < 4; ++suite)
        for (profile = 0; profile < 24; ++profile)
            for (U seed : {0u, 0xa5u})
                for (mutation = 0; mutation < 4; ++mutation) {
                    auto start = input(seed);
                    auto expected = start;
                    reset(expected);
                    execute(expected, false, suite);
                    const auto expectedCalls = calls;
#ifdef MNM_ORIGINAL_REFERENCE
                    std::memcpy((void *)Base, start.at(Base, Size), Size);
                    for (U a : Globals)
                        std::memcpy((void *)a, start.at(a), 4);
                    for (U a : {0x6dde58u, 0x6ead98u, 0x658f10u, 0x659043u, 0x659176u, 0x6592a9u})
                        std::memcpy((void *)a, start.at(a, 64), 64);
                    Memory actual;
                    actual.original = true;
                    reset(actual);
                    execute(actual, true, suite);
                    if (actual.snapshot() != expected.snapshot() || calls != expectedCalls) {
                        std::cerr << "suite " << suite << " profile " << profile << " seed " << seed
                                  << " mutation " << mutation << " calls " << calls.size() << "/"
                                  << expectedCalls.size() << '\n';
                        for (U i = 0; i < std::min(calls.size(), expectedCalls.size()); ++i)
                            if (!(calls[i] == expectedCalls[i])) {
                                std::cerr
                                    << "first event " << i << " fn " << std::hex << calls[i].fn
                                    << "/" << expectedCalls[i].fn << " receiver "
                                    << calls[i].receiver << "/" << expectedCalls[i].receiver
                                    << std::dec << " args " << calls[i].args.size() << "/"
                                    << expectedCalls[i].args.size() << " text " << calls[i].text
                                    << "/" << expectedCalls[i].text << " delta "
                                    << calls[i].delta.size() << "/" << expectedCalls[i].delta.size()
                                    << '\n';
                                break;
                            }
                        throw std::runtime_error("Acquisition mismatch");
                    }
#endif
                    for (auto b : expected.snapshot()) {
                        checksum = checksum * 131 + b;
                    }
                    for (const auto &c : expectedCalls) {
                        checksum = checksum * 131 + c.fn;
                        checksum = checksum * 131 + c.receiver;
                        for (U a : c.args)
                            checksum = checksum * 131 + a;
                        for (auto b : c.text)
                            checksum = checksum * 131 + static_cast<unsigned char>(b);
                    }
                    ++cases;
                    if (suite == 3)
                        ++journeys;
                }
#ifdef MNM_ORIGINAL_REFERENCE
    for (U i = 0; i < std::size(ranges); ++i)
        need(bodies[i] ==
                 Bytes((unsigned char *)ranges[i].first, (unsigned char *)ranges[i].second),
             "Original caller changed");
#endif
    std::cout << "{\"success\":true,\"cases\":" << cases << ",\"composed_lifecycles\":" << journeys
              << ",\"checksum\":" << checksum << "}\n";
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}

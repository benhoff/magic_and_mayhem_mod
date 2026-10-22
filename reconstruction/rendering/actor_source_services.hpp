#pragma once

#include "actor_services.hpp"
#include <algorithm>

namespace mnm::reconstruction::actor_services {

// The graph, actor roots, queues and vector backing all live in M's value-address
// storage. No second logical actor/graph copy is synchronized at call boundaries.
template<class M> class SourceServices {
    M& m;
    U r(U p) { return m.get32(p); }
    void w(U p, U v) { m.set32(p, v); }
    U call(U entry, U self, std::initializer_list<U> a = {}) {
        return m.invoke(entry, self, a);
    }
    U owner(U actor) { return 0x643a80 + r(actor + 0x174) * 0x154; }
    U begin(U node) {
        U slot = 0;
        while (slot != 8 && !m.get8(node + 0x41 + slot)) ++slot;
        if (slot == 8) { slot = r(node + 0x3d); w(node + 0x3d, (slot + 1) & 7); }
        U children = r(node + 0x29);
        w(node + 0x49 + slot * 4, children);
        w(node + 0x69 + slot * 4,
          children && ((r(node + 0x2d) - children) & ~3u) ? begin(r(children)) : 0);
        m.set8(node + 0x41 + slot, 0);
        return slot;
    }
    U next(U node, U slot) {
        U actor = r(node + 0x35), cursor = node + 0x49 + slot * 4;
        U nested = node + 0x69 + slot * 4;
        if (actor) {
            if (signed32(r(0x5d854c)) <= signed32(r(node + 0x89)) && !r(nested)) {
                w(nested, 1); return actor;
            }
        } else {
            while (r(cursor) != r(node + 0x2d)) {
                U found = next(r(r(cursor)), r(nested));
                if (found) return found;
                w(cursor, r(cursor) + 4);
                if (r(cursor) != r(node + 0x2d)) w(nested, begin(r(r(cursor))));
            }
        }
        m.set8(node + 0x41 + slot, 1);
        return 0;
    }
    void append(U vector, U value) {
        U temporary = m.temporary(4); w(temporary, value);
        call(0x42a340, vector, {r(vector + 8), 1, temporary});
    }
    void attach(U parent, U child) {
        if (r(parent + 0x35) || r(child + 0x39)) return;
        U old = r(parent + 0x29);
        append(parent + 0x25, child);
        U moved = r(parent + 0x29) - old;
        for (U slot = 0; slot != 8; ++slot)
            if (!m.get8(parent + 0x41 + slot))
                w(parent + 0x49 + slot * 4, r(parent + 0x49 + slot * 4) + moved);
        w(child + 0x39, parent);
    }
    U make(U manager, U actor, U hint) {
        U node = call(0x597850, 0, {0x8d});
        if (node) {
            U family = r(manager); w(node, family); m.set8(node + 4, family == 0);
            for (U offset : {5u, 0xdu, 0x11u, 0x15u, 0x1du, 0x29u, 0x2du,
                             0x31u, 0x39u, 0x3du}) w(node + offset, 0);
            m.set8(node + 0x25, hint); w(node + 0x35, actor);
            if (actor) { w(node + 0x89, 3); w(node + 9, ~0u); }
            w(node + 0x41, 0x01010101); w(node + 0x45, 0x01010101);
        }
        append(manager + 0x1a, node);
        return node;
    }
    bool active(U node, U& hint) {
        U previous = r(0x5d854c); w(0x5d854c, 3);
        U cursor = begin(node), first = next(node, cursor); hint = first != 0;
        while (next(node, cursor)) {}
        w(0x5d854c, previous); return first != 0;
    }
    U normalize(U value, U dimension) {
        if (!dimension || signed32(dimension) <= 0)
            throw std::invalid_argument("source coordinate extent");
        while (signed32(value) < 0) value += dimension;
        while (value >= dimension) value -= dimension;
        return value;
    }
    U distance(U actor, U peer) {
        auto axis = [&](U offset, U dimension) {
            U a = normalize(r(actor + offset), dimension);
            U b = normalize(r(peer + offset), dimension);
            U delta = a >= b ? a - b : b - a;
            return delta > dimension / 2 ? dimension - delta : delta;
        };
        U x = axis(8, r(0x6c5494)) << 5, y = axis(12, r(0x6c5498)) << 5;
        U z = (r(actor + 16) - r(peer + 16)) << 4;
        return (x*x + y*y + z*z) >> 10;
    }
    U terminal(U actor, U family) {
        U node = r(actor + 0x678 + family * 4);
        while (node && r(node + 0x39)) node = r(node + 0x39);
        return node;
    }
    void root(U actor, U family, U node) {
        w(actor + 0x678 + family * 4, r(owner(actor) + 4) && node ? 0 : node);
    }
    void group(U manager, U actor) {
        U army = owner(actor), hint = actor & 255;
        if ((r(army + 12) == ~0u && !r(actor + 0x8e5)) || r(army + 4) ||
            r(actor + 0x704) || r(actor + 0xa8) == 22 ||
            (signed32(r(0x6e1f0c)) > 1 && r(actor + 0x174) != r(0x656634))) return;
        U count = 0;
        for (U at = r(manager + 0x1e); at != r(manager + 0x22); at += 4) {
            U node = r(at);
            if (!r(node + 0x39) && active(node, hint)) {
                U cursor = begin(node), member = next(node, cursor);
                if (r(member + 0x174) == r(actor + 0x174)) ++count;
            }
        }
        bool fresh = r(army + 12) == ~0u;
        if (!fresh && signed32(count) < signed32(r(0x5d865c)))
            fresh = (call(0x54e200, 0x6a29d8) & 1) == 0;
        U minimum = 0x7fffffffu, selected = 0;
        if (!fresh) for (U at = r(manager + 0x1e); at != r(manager + 0x22); at += 4) {
            U node = r(at), peer = r(node + 0x35);
            if (peer && active(node, hint) && r(peer + 0x174) == r(actor + 0x174)) {
                U value = distance(actor, peer);
                // Original byte25 is an allocator/caller representation byte;
                // the coordinate branch uses an original stack address byte.
                // Native owned storage supplies a deterministic opaque tag.
                hint = 0;
                U candidate = terminal(peer, r(manager));
                if (signed32(value) < signed32(minimum)) { minimum = value; selected = candidate; }
            }
        }
        U leaf = make(manager, actor, hint); root(actor, r(manager), leaf);
        w(leaf + 0x89, 1); U chosen = leaf;
        if (!fresh && minimum != 0x7fffffffu) {
            if (!r(selected + 0x35)) { attach(selected, leaf); chosen = selected; }
            else if (selected != r(manager + 0x11daf) && selected != r(manager + 0x11fc8)) {
                chosen = make(manager, 0, hint); attach(chosen, selected); attach(chosen, leaf);
                w(chosen + 5, r(selected + 5)); w(selected + 5, 0);
                if (r(chosen + 5)) w(r(chosen + 5) + 8, chosen);
            }
        }
        if (!r(chosen + 5)) pending(manager, chosen);
    }
    void frontOnce(U queue, U node) {
        U cursor = r(queue + 12), last = r(queue + 28);
        U end = r(queue + 8), map = r(queue + 16);
        if (r(queue + 44)) {
            while (cursor != last && r(cursor) != node) {
                cursor += 4;
                if (cursor == end) { map += 4; cursor = r(map); end = cursor + 4096; }
            }
            if (cursor != last) return;
        }
        if (!r(queue + 44) || r(queue + 12) == r(queue + 4)) call(0x42afd0, queue);
        cursor = r(queue + 12) - 4; w(queue + 12, cursor);
        if (cursor) w(cursor, node);
        w(queue + 44, r(queue + 44) + 1);
    }
    void pending(U manager, U node) {
        if (!r(node + 5)) frontOnce(manager + 0x2a, node);
        U selected = manager + 0x8a, cursor = begin(node), actor;
        while ((actor = next(node, cursor))) {
            U kind = r(r(actor + 0xac));
            if ((kind == 0 || kind == 24 || kind == 25 || kind == 26) && !r(actor + 0x7c)) {
                selected = manager + 0x5a;
                while (next(node, cursor)) {}
                break;
            }
        }
        frontOnce(selected, node);
    }
    void growFront(U queue) {
        U block = call(0x597850, 0, {4096});
        if (!r(queue + 44)) {
            w(queue + 40, 2);
            U map = call(0x597850, 0, {8}); w(queue + 36, map); w(map + 4, block);
            for (U offset : {4u, 20u}) w(queue + offset, block);
            for (U offset : {8u, 24u}) w(queue + offset, block + 4096);
            for (U offset : {12u, 28u}) w(queue + offset, block + 0x804);
            for (U offset : {16u, 32u}) w(queue + offset, map + 4);
            return;
        }
        U first = r(queue + 16), last = r(queue + 32);
        if (r(queue + 36) < first) {
            first -= 4; w(queue + 16, first); w(first, block);
        } else if (last == first) {
            U old = r(first); w(queue + 32, last + 4); w(last, old);
            w(r(queue + 16) + 4, r(r(queue + 16))); w(r(queue + 16), block);
        } else {
            U count = (last - first) / 4 + 1, map = call(0x597850, 0, {count * 8});
            U target = map + ((count * 2) & ~3u);
            for (U at = first, out = target; at != last + 4; at += 4, out += 4) w(out, r(at));
            call(0x597870, 0, {r(queue + 36)});
            w(queue + 36, map); w(queue + 40, count * 2);
            first = target - 4; w(first, block); w(queue + 16, first);
            last = first + count * 4; w(queue + 32, last);
            U oldLast = r(last); w(queue + 20, oldLast); w(queue + 24, oldLast + 4096);
        }
        U current = r(r(queue + 16));
        w(queue + 4, current); w(queue + 8, current + 4096); w(queue + 12, block + 4096);
    }
    void popBack(U queue) {
        auto releaseBlock = [&]() {
            U slot = r(queue + 32); w(queue + 32, slot - 4);
            call(0x597870, 0, {r(slot)});
            if (!r(queue + 44)) {
                if (r(queue + 16) == r(queue + 32)) call(0x597870, 0, {r(r(queue + 16))});
                for (U offset = 4; offset <= 32; offset += 4) w(queue + offset, 0);
                call(0x597870, 0, {r(queue + 36)});
            } else {
                U block = r(r(queue + 32));
                w(queue + 20, block); w(queue + 24, block + 4096); w(queue + 28, block + 4096);
            }
        };
        if (r(queue + 28) == r(queue + 20)) releaseBlock();
        if (r(queue + 44)) w(queue + 28, r(queue + 28) - 4);
        w(queue + 44, r(queue + 44) - 1);
        if (!r(queue + 44)) releaseBlock();
    }
    U element(U queue, U index) {
        U at = r(queue + 12) + index * 4, end = r(queue + 8), map = r(queue + 16);
        while (at >= end) { at = r(map + 4) + (at - end); map += 4; end = r(map) + 4096; }
        return at;
    }
    void eraseQueued(U queue, U node) {
        U count = r(queue + 44), keep = 0;
        for (U i = 0; i < count; ++i) {
            U value = r(element(queue, i));
            if (value != node) { if (keep != i) w(element(queue, keep), value); ++keep; }
        }
        for (U removed = count - keep; removed; --removed) popBack(queue);
    }
    void removeValue(U vector, U node) {
        U output = r(vector + 4), end = r(vector + 8);
        for (U at = output; at != end; at += 4) if (r(at) != node) {
            if (at != output) w(output, r(at));
            output += 4;
        }
        w(vector + 8, output);
    }
    void detach(U parent, U child) {
        if (r(parent + 0x35)) return;
        U end = r(parent + 0x2d), at = r(parent + 0x29);
        while (at != end && r(at) != child) at += 4;
        if (at == end) return;
        for (U next = at + 4; next != end; next += 4) w(next - 4, r(next));
        w(parent + 0x2d, end - 4);
        for (U slot = 0; slot < 8; ++slot) {
            U cursor = parent + 0x49 + slot * 4;
            if (!m.get8(parent + 0x41 + slot) && at < r(cursor)) w(cursor, r(cursor) - 4);
        }
        w(child + 0x39, 0);
    }
    void destruct(U node) {
        U previous = r(0x5d854c); w(0x5d854c, 0); U slot = begin(node);
        U cursor = node + 0x49 + slot * 4;
        while (r(cursor) != r(node + 0x2d)) {
            U child = r(r(cursor)); w(cursor, r(cursor) + 4);
            if (!child) break;
            if (!r(child + 0x35) || signed32(r(child + 0x89)) >= signed32(r(0x5d854c)))
                w(child + 0x39, 0);
        }
        m.set8(node + 0x41 + slot, 1);
        if (r(node + 0x39)) detach(r(node + 0x39), node);
        w(0x5d854c, previous); call(0x597870, 0, {r(node + 0x29)});
        w(node + 0x29, 0); w(node + 0x2d, 0); w(node + 0x31, 0);
    }
    void prune(U manager) {
        U previous = r(0x5d854c); w(0x5d854c, 0);
        for (U at = r(manager + 0x1e); at != r(manager + 0x22);) {
            U node = r(at); bool keep = false;
            for (U ancestor = node; ancestor; ancestor = r(ancestor + 0x39)) keep |= r(ancestor + 5) != 0;
            if (!keep) {
                w(0x5d854c, 1); U cursor = begin(node), populated = next(node, cursor);
                while (next(node, cursor)) {} w(0x5d854c, 0);
                keep = populated != 0;
                if (!keep) for (U ancestor = node; ancestor; ancestor = r(ancestor + 0x39))
                    keep |= ancestor == r(manager + 0x11daf) || ancestor == r(manager + 0x11fc8);
            }
            if (keep) { at += 4; continue; }
            U end = r(manager + 0x22) - 4, last = r(end); w(at, last); w(end, node); w(manager + 0x22, end);
            for (U offset : {0x2au, 0x5au, 0x8au}) eraseQueued(manager + offset, node);
            for (U offset : {0x11db3u, 0x11de5u, 0x11e05u}) removeValue(manager + offset, node);
            U cursor = begin(node), actor;
            while ((actor = next(node, cursor))) if (r(actor + 0x678 + r(manager)*4) == node) root(actor, r(manager), 0);
            destruct(node); call(0x597870, 0, {node});
        }
        w(0x5d854c, previous);
    }
public:
    explicit SourceServices(M& memory) : m(memory) {}
    // Native session ownership policy: only release backing after the recovered
    // pruning operation has retired every node and pending admission. A planner
    // that still protects a source must relinquish it before pool destruction.
    bool quiescent(U manager) {
        if (r(manager + 0x1e) != r(manager + 0x22)) return false;
        for (U offset : {0x2au, 0x5au, 0x8au}) if (r(manager + offset + 44)) return false;
        for (U offset : {0x11db3u, 0x11de5u, 0x11e05u})
            if (r(manager + offset + 4) != r(manager + offset + 8)) return false;
        return true;
    }
    void releaseEmptyBacking(U manager) {
        if (!quiescent(manager)) throw std::logic_error("Source owner is not quiescent");
        for (U offset : {0x1au, 0x11db3u, 0x11de5u, 0x11e05u}) {
            U vector = manager + offset;
            call(0x597870, 0, {r(vector + 4)});
            w(vector + 4, 0); w(vector + 8, 0); w(vector + 12, 0);
        }
    }
    U run(U entry, U self, std::initializer_list<U> args = {}) {
        U a[3]{}; unsigned n = 0;
        for (U v : args) { if (n == 3) throw std::invalid_argument("source argument extent"); a[n++] = v; }
        switch (entry) {
        case 0x407920: return begin(self);
        case 0x4079f0: return next(self, a[0]);
        case 0x416590: group(self, a[0]); break;
        case 0x416fe0: pending(self, a[0]); break;
        case 0x514810: root(self, a[0], a[1]); break;
        case 0x5183f0: return terminal(self, a[0]);
        case 0x51e790: return r(owner(self) + 12) == ~0u;
        case 0x42afd0: growFront(self); break;
        case 0x429900: popBack(self); break;
        case 0x407780: detach(self, a[0]); break;
        case 0x407820:
            while (r(self + 0x29) && ((r(self + 0x2d) - r(self + 0x29)) & ~3u)) {
                w(r(r(self + 0x2d) - 4) + 0x39, 0); w(self + 0x2d, r(self + 0x2d) - 4);
            } break;
        case 0x407650: destruct(self); break;
        case 0x417e60: destruct(self); if (a[0] & 1) call(0x597870, 0, {self}); return self;
        case 0x417580: prune(self); break;
        default: throw std::invalid_argument("Unrecovered source service");
        }
        return 0;
    }
};
} // namespace mnm::reconstruction::actor_services

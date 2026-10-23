#pragma once
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

namespace mnm::reconstruction::combat {
// NoCD 40209ca7 value-address storage. Only whole-tree normal destruction is
// exposed: iterator erasure, insertion/rebalancing and exception unwind differ.
// M owns checked PE32 storage, allocate/free and the source-destruction service.
template<class M> class OwnerTaskStorage {
    using U = std::uint32_t;
    M& m;
    U r(U p) { return m.get32(p); }
    void w(U p, U v) { m.set32(p, v); }
    U allocate(U n) {
        U p = m.allocate(n);
        if (!p) throw std::runtime_error("task nested allocation failed");
        return p;
    }
    void tree(U p, U global, U size) {
        // Constructor tags come from undefined caller-stack bytes in the PE.
        // Zero is an explicit native representation, not recovered provenance.
        m.set8(p, 0); m.set8(p + 1, 0); m.set8(p + 8, size == 24);
        U nil = r(global);
        if (!nil) {
            nil = allocate(size); w(global, nil);
            w(nil + 4, 0); w(nil + size - 4, 1); w(nil, 0); w(nil + 8, 0);
        }
        w(global + 4, r(global + 4) + 1);
        U head = allocate(size);
        w(head + 4, nil); w(head + size - 4, 0);
        w(p + 4, head); w(p + 12, 0); w(head, head); w(head + 8, head);
    }
    void nodes(U node, U nil, unsigned depth = 0) {
        if (node == nil) return;
        if (!node || depth == 128) throw std::invalid_argument("task tree domain");
        // Actual helper visits right recursively, then walks left; value
        // destructors for these four scalar/POD maps have no extra effects.
        nodes(r(node + 8), nil, depth + 1);
        U left = r(node); m.free(node); nodes(left, nil, depth + 1);
    }
    void destroyTree(U p, U global) {
        U head = r(p + 4), nil = r(global);
        if (!head || !nil || !r(global + 4))
            throw std::invalid_argument("task tree ownership");
        nodes(r(head + 4), nil);
        // Whole-range erase sets head links before its storage is released.
        w(head + 4, nil); w(p + 12, 0); w(head, head); w(head + 8, head);
        m.free(head); w(p + 4, 0); w(p + 12, 0);
        U count = r(global + 4) - 1; w(global + 4, count);
        if (!count) { m.free(nil); w(global, 0); }
    }
    void vector(U p) {
        m.free(r(p + 4)); w(p + 4, 0); w(p + 8, 0); w(p + 12, 0);
    }
public:
    explicit OwnerTaskStorage(M& memory) : m(memory) {}
    // 419e40 ignores the old evaluator's contents; this is fresh construction,
    // including fresh heads and shared nil references, rather than a deep copy.
    U construct(U size, U kind, U vtable) {
        U p = m.allocate(size); if (!p) return 0;
        w(p + 4, kind); w(p + 0x23, ~U(0)); m.set8(p + 0x37, 0);
        for (U o : {0x3bu, 0x3fu, 0x43u}) w(p + o, 0);
        for (U o = 0; o < 60; o += 4) w(p + 0x47 + o, 0);
        tree(p + 0xa4, 0x5faf2c, 24); tree(p + 0xb4, 0x5faf34, 56);
        for (U o : {0xc4u, 0xd4u}) { m.set8(p + o, 0); for(U q : {4u,8u,12u}) w(p+o+q,0); }
        tree(p + 0x101, 0x5faf3c, 24); tree(p + 0x111, 0x5faf44, 56);
        for (U o : {0x121u,0x131u}) { m.set8(p + o,0); for(U q : {4u,8u,12u}) w(p+o+q,0); }
        w(p, vtable); return p;
    }
    U fallback() { return construct(0x14b, 0, 0x5c53c0); }
    // 426070 fresh task6 evaluator. Candidate/saved feature vectors begin empty;
    // source-vector ownership is shared with the normal base destruction path.
    U task6() {
        U p=construct(0x1b4,6,0x5c596c);if(!p)return 0;
        for(U off:{0x15fu,0x17bu}){m.set8(p+off,0);for(U o:{4u,8u,12u})w(p+off+o,0);}
        return p;
    }
    // 418050 is a base destructor. Caller must unbind and free the task object;
    // the source pointer at+8 is borrowed and is deliberately not followed here.
    void destroy(U p) {
        w(p, 0x5c5358);
        for (U at=r(p+0x3b);at!=r(p+0x3f);at+=4)
            if (U source=r(at)) { m.destroySource(source); m.free(source); }
        w(p+0x3f,r(p+0x3b));
        vector(p+0x131); vector(p+0x121);
        destroyTree(p+0x111,0x5faf44); destroyTree(p+0x101,0x5faf3c);
        vector(p+0xd4); vector(p+0xc4);
        destroyTree(p+0xb4,0x5faf34); destroyTree(p+0xa4,0x5faf2c);
        vector(p+0x37);
    }
};
}

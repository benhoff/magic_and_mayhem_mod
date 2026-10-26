#pragma once
#include <array>
#include <cstdint>
#include <initializer_list>
#include <string>

namespace mnm::reconstruction {
// No-CD 40209ca7, normal-return local callers. Lower constructors, parsers,
// asset/OS/simulation services and CRT formatting remain explicit endpoints.
// Memory uses original DWORD addresses; Services never exposes host pointers.
template <class Memory, class Services>
void acquireOriginalScreenLayout(Memory &m, Services &p, std::uint32_t owner,
                                 std::uint32_t filename, bool report) {
    using U = std::uint32_t;
    const U count = report ? 23 : 7, allocation = p.allocate(4 + count * 0x141);
    U objects = 0;
    if (allocation) {
        m.set(allocation, count);
        objects = allocation + 4;
        p.constructArray(objects, 0x141, count, 0x559820, 0x559880);
    }
    m.set(owner + 0x3f, objects);
    for (U i = 0; i < (report ? 21u : 6u); ++i)
        p.layout(filename, m.get(owner + 0x3f) + i * 0x141, report ? 6 : 0, i + 1);
    p.layout(filename, m.get(owner + 0x3f) + (report ? 21 : 6) * 0x141, report ? 0 : 6, 1);
}

template <class Memory, class Services>
void prepareOriginalMainScreen(Memory &m, Services &p, std::uint32_t owner) {
    using U = std::uint32_t;
    if (m.get(owner + 8))
        return;
    p.loadLayout(owner, 0x5e05dc);
    auto width = [&] { return m.get(0x6de6d5) ? 800u : 640u; };
    auto height = [&] { return m.get(0x6de6d5) ? 600u : 480u; };
    std::array<U, 27> descriptor{};
    descriptor[0] = 0x6c;
    descriptor[1] = 7;
    descriptor[2] = height();
    descriptor[3] = width();
    descriptor[26] = 0x840;
    const U canvas = p.allocate(0x32);
    if (canvas) {
        for (U o : {4u, 8u, 12u, 24u, 20u, 16u, 32u, 36u, 40u})
            m.set(canvas + o, 0);
        m.byte(canvas + 44, 0);
        m.byte(canvas + 45, 0);
        m.byte(0x6f694c, 0);
    }
    m.set(owner + 0x13, canvas);
    p.surface(canvas, descriptor, 0x5e05c0, 0x175);
    if (!m.get(owner + 0x47)) {
        const U callback = p.allocate(12);
        if (callback) {
            m.set(callback + 4, 0x4a7850);
            m.set(callback + 8, owner);
            m.set(callback, 0x5c6414);
        }
        m.set(owner + 0x47, callback);
        m.set(m.get(owner + 0x13) + 0x28, callback);
    }
    std::string path = p.format(0x5da418, p.str(0x5e05a8).c_str(), width(), height());
    const std::string background = p.path(0x5d9f10, 0x5e05dc);
    path += p.format(0x5da5a8, background.c_str(), width(), height(), p.str(0x5da5b4).c_str());
    if (!p.text(0x58d320, m.get(owner + 0x13), path))
        p.text(0x49e4e0, 0, path);
    const U receiver = p.allocate(12);
    if (receiver) {
        m.set(receiver + 4, 0x4a75c0);
        m.set(receiver + 8, owner);
        m.set(receiver, 0x5c6414);
    }
    m.set(owner + 0x4f, receiver);
    auto button = [&](U i) { return m.get(m.get(owner + 0x4b) + 4 * i); };
    for (U i = 0; i < 6; ++i) {
        U object = p.allocate(0x59);
        if (object)
            object = p.call(0x4cc720, object);
        m.set(m.get(owner + 0x4b) + 4 * i, object);
        p.text(0x4cb040, button(i), p.str(m.get(owner + 0x3f) + i * 0x141 + 0x24));
        p.call(0x4cb030, button(i), {m.get(m.get(owner + 0x3f) + i * 0x141)});
        p.call(0x4ca9b0, button(i), {0x6a49e0});
        m.set(button(i) + 0x55, 0x28);
        p.call(0x4cb0b0, button(i),
               {i, m.get(owner + 0x3f) + i * 0x141 + (m.get(0x6de6d5) ? 20 : 4),
                m.get(owner + 0x4f), 0, 1});
    }
    if (const U sixth = button(5)) {
        const U data = m.get(sixth + 0x45);
        m.set(sixth, 0x5c5f24);
        if (data) {
            p.free(data);
        }
        p.call(0x4ca910, sixth);
        p.free(sixth);
    }
    m.set(m.get(owner + 0x4b) + 20, 0);
    U label = p.allocate(0x29);
    if (label)
        label = p.call(0x4d27b0, label);
    m.set(owner + 0x53, label);
    p.call(0x4d28b0, label, {m.get(m.get(owner + 0x3f) + 0x786)});
    p.call(0x4d2960, m.get(owner + 0x53), {0x6a49e0});
    const auto version = static_cast<std::int32_t>(p.call(0x4e8b70));
    p.text(0x4d28c0, m.get(owner + 0x53), p.format(0x5e0598, double(version) * p.versionScale()));
    // The callee consumes only the byte argument. Upper caller-saved EDX bits
    // at 4a74bf are not a recovered semantic parameter of the label service.
    p.call(0x4d2970, m.get(owner + 0x53), {m.getByte(m.get(owner + 0x3f) + 0x8be)});
    p.call(0x4d2930, m.get(owner + 0x53),
           {m.get(owner + 0x3f) + (m.get(0x6de6d5) ? 0x79a : 0x78a)});
    p.call(0x4cabd0, owner + 0x1f, {5});
    for (U i = 0; i < 5; ++i)
        p.call(0x4cad00, owner + 0x1f, {button(i), U(i == 0)});
    if (m.getByte(0x6e2031)) {
        p.disable(button(0));
        p.disable(button(1));
        p.call(0x4caf00, owner + 0x1f, {button(2)});
    }
    m.set(owner + 8, 1);
}

template <class Memory, class Services>
void enterOriginalWorldScreen(Memory &m, Services &p, std::uint32_t owner) {
    using U = std::uint32_t;
    auto call = [&](U f, U r = 0, std::initializer_list<U> a = {}) { return p.call(f, r, a); };
    auto timer = [&] { call(0x4dc5e0, owner + 0xfd66); };
    m.set(owner + 0xfd62, 0);
    if (m.get(0x5fed7c)) {
        call(0x487a90, 0x6c5040, {0x5d9ed0});
        call(0x487b10, 0x6c5040, {m.get(owner + 0xfd62)});
    }
    if (m.get(0x6e1f30))
        call(0x485a80, 0x657d78, {0x5d9ec0});
    for (U o : {0x100a2u, 0x1031cu, 0x100b2u, 0xfdeeu})
        m.byte(owner + o, 0);
    for (U o : {0x100b0u, 0x1031bu})
        m.byte(owner + o, 1);
    if (m.get(0x68991c)) {
        m.set(0x6dbe99, 20);
        m.set(0x6dbe95, 50);
    } else {
        const auto divisor = static_cast<std::int32_t>(m.get(0x6de6fd));
        m.set(0x6dbe99, U(divisor));
        m.set(0x6dbe95, U(1000 / divisor));
    }
    timer();
    m.byte(0x6e9858, 0);
    call(0x57daa0, 0x6de558,
         {1, m.get(0x6de6e5), m.get(0x5e15d8), m.get(0x5e15dc), m.get(0x5e15e0), m.get(0x5e15e4),
          m.get(0x5e15e8), m.get(0x5e15ec)});
    for (U o : {0x102dbu, 0x102f7u, 0x102fbu, 0x102ffu, 0x10303u})
        m.set(owner + o, 0xffffffffu);
    m.set(owner + 0x102d7, 0);
    m.set(0x659e2f, 2);
    m.set(0x6e007c, 0);
    m.byte(owner + 0x100a6, m.get(0x689920) == 5 || m.get(0x6e1f0c) != 0);
    timer();
    timer();
    const U sprites[] = {0x644528, 0x6593e0, 0x6ddd30}, names[] = {0x5d9eac, 0x5d9e98, 0x5d9e84};
    for (U i = 0; i < 3; ++i)
        if (!p.sprite(sprites[i], p.str(names[i]), 2))
            p.text(0x49e4e0, 0, p.str(names[i]));
    call(0x570440, 0x658178);
    for (U f : {0x52b2e0u, 0x52b360u, 0x52b3b0u, 0x52b400u, 0x52b450u, 0x52b4a0u, 0x52b4f0u})
        call(f, 0x6def58);
    timer();
    timer();
    for (U f : {0x549210u, 0x549690u, 0x549b00u})
        call(f);
    timer();
    timer();
    call(0x4a08a0, 0x634740);
    timer();
    timer();
    call(0x470ee0, owner);
    if ((m.get(0x689920) == 1 || m.get(0x689920) == 2) && !m.get(0x6e1f0c)) {
        call(0x4dd010, 0x6c4630);
        for (U i = 0; i < 4; ++i) {
            const U name = 0x658f10 + i * 0x133;
            if (p.str(name) != p.str(0x6ead98) && i != m.get(0x656634))
                call(0x4dcee0, 0x6c4630, {i, name});
        }
    }
    timer();
    const U request = m.get(owner + 0x100df);
    if (request && m.getByte(request) && !m.getByte(request + 1))
        call(0x479a10, 0x6c3898, {0x5d9e74});
    timer();
    call(0x482010, 0x657948, {1});
    call(0x56f000, 0x6b0198, {0x324, 0, 0, 0, 0, 0xffffffffu, 0xffffffffu});
    timer();
    timer();
    if (m.getByte(0x6e1ef4))
        p.text(0x4d8cb0, 0x6ddf78, p.format(0x5d9e64, m.get(0x656624)));
    call(0x46a370, owner);
    m.set(0x689938, 1);
    m.byte(owner + 0x100ac, 0);
    call(0x4dc6e0, owner + 0xfd66);
    if (m.get(owner + 0x10317) && !m.get(0x6e1f0c)) {
        call(0x4dde90, 0x6905a8, {m.get(0x6c3778) * 60});
        call(0x4de5a0, 0x6905a8, {m.get(0x6de6d5) ? 640u : 480u, m.get(0x6de6d5) ? 464u : 344u});
        m.set(0x6905ac, 1);
        m.byte(0x5fc86c, 0);
        m.set(0x5fc864, m.get(0x6c3778) * 60);
        if (m.get(0x689920) == 1) {
            call(0x534aa0, owner + 0x10307, {1000, 5});
            call(0x534ad0, owner + 0x10307, {0x469a60});
            m.byte(0x5fc86c, 0);
        }
    }
    m.set(owner + 8, 1);
    m.byte(owner + 0x100b1, 0);
    call(0x470410, owner);
    if (m.get(0x68991c) && m.getByte(0x5eab18))
        p.packet(0x6a2898, {0, 3, 0x10, m.get(0x6e1acc) ^ m.get(0x6e1ac8)}, m.get(owner + 0xfd62));
}

template <class Memory, class Services>
void prepareOriginalCampaignReport(Memory &m, Services &p, std::uint32_t owner) {
    using U = std::uint32_t;
    auto width = [&] { return m.get(0x6de6d5) ? 800u : 640u; };
    auto height = [&] { return m.get(0x6de6d5) ? 600u : 480u; };
    auto call = [&](U f, U r = 0, std::initializer_list<U> a = {}) { return p.call(f, r, a); };
    auto label = [&](U i) { return m.get(m.get(owner + 0x4b) + 4 * i); };
    auto button = [&] { return m.get(m.get(owner + 0x47)); };
    m.set(owner + 0x2a1, !m.get(0x6de6d5));
    std::array<U, 27> descriptor{};
    descriptor[0] = 0x6c;
    descriptor[1] = 7;
    descriptor[2] = height();
    descriptor[3] = width();
    descriptor[26] = 0x840;
    p.surface(owner + 0x25f, descriptor, 0x5da424, 0xf5);
    m.set(owner + 0x295, 0);
    m.set(owner + 0x291, 0);
    m.set(owner + 0x299, width() - 1);
    m.set(owner + 0x29d, height() - 1);
    std::string prefix = p.format(0x5da418, p.str(0x5c5e20).c_str(), width(), height());
    const auto tail = p.format(0x5da3e4, p.str(m.get(owner + 0x53) ? 0x5da410 : 0x5da408).c_str(),
                               width(), height(), p.str(0x5da400).c_str());
    if (!p.text(0x58d320, owner + 0x25f, prefix + tail))
        p.text(0x49e4e0, 0, tail);
    prefix = p.format(0x5da418, p.str(0x5da3d0).c_str(), width(), height());
    const U sprite = p.allocate(0x24);
    if (sprite)
        for (U i = 0; i < 36; i += 4)
            m.set(sprite + i, 0);
    m.set(owner + 0x2a5, sprite);
    if (!p.sprite(sprite, prefix + p.str(0x5da3c4), 2))
        p.text(0x49e4e0, 0, prefix + p.str(0x5da3c4));
    if (m.get(0x5da330))
        m.set(0x5da330, 0);
    m.set(owner + 0x257, p.allocate(0x100));
    m.set(owner + 0x25b, p.allocate(0x100));
    const auto cfg = p.format(0x5da3bc, p.str(0x6dde58).c_str());
    if (!p.reportConfig(owner, cfg, m.get(0x656620)))
        p.text(0x49e4e0, 0, p.str(0x5da3ac));
    p.storeText(owner + 0x57, p.str(m.get(owner + 0x257)) + p.str(0x5da3a4) +
                                  p.str(m.get(owner + 0x25b)) + p.str(0x5da3a0));
    const auto layout = p.format(0x5da398, p.str(0x5c5e20).c_str(),
                                 p.str(m.get(owner + 0x53) ? 0x5c5e38 : 0x5c5e5c).c_str());
    p.loadLayoutText(owner, layout);
    const U callback = p.allocate(12);
    if (callback) {
        m.set(callback + 4, 0x4747a0);
        m.set(callback + 8, owner);
        m.set(callback, 0x5c5eec);
    }
    m.set(owner + 0x4f, callback);
    const U labels = p.allocate(0x54);
    m.set(owner + 0x4b, labels);
    for (U i = 0; i < 21; ++i)
        m.set(labels + i * 4, 0);
    const U buttons = p.allocate(4);
    m.set(owner + 0x47, buttons);
    m.set(buttons, 0);
    for (U i = 0; i < 21; ++i) {
        U object = p.allocate(0x29);
        if (object)
            object = call(0x4d27b0, object);
        m.set(m.get(owner + 0x4b) + 4 * i, object);
        call(0x4d28b0, label(i), {m.get(m.get(owner + 0x3f) + i * 0x141)});
        call(0x4d2960, label(i), {0x6a49e0});
        p.text(0x4d28c0, label(i), p.str(m.get(owner + 0x3f) + i * 0x141 + 0x24));
        call(0x4d2970, label(i), {m.getByte(m.get(owner + 0x3f) + i * 0x141 + 0x138)});
        call(0x4d2930, label(i), {m.get(owner + 0x3f) + i * 0x141 + (m.get(0x6de6d5) ? 20 : 4)});
    }
    if (!m.get(owner + 0x53))
        p.text(0x4d28c0, label(19), p.str(owner + 0x157));
    else {
        if (const U object = label(19)) {
            p.destruct(0x4d27d0, object);
            p.free(object);
        }
        m.set(m.get(owner + 0x4b) + 0x4c, 0);
    }
    U object = p.allocate(0x59);
    if (object)
        object = call(0x4cc720, object);
    m.set(m.get(owner + 0x47), object);
    p.text(0x4cb040, button(), p.str(m.get(owner + 0x3f) + 0x1a79));
    call(0x4cb030, button(), {m.get(m.get(owner + 0x3f) + 0x1a55)});
    call(0x4ca9b0, button(), {0x6a49e0});
    m.set(button() + 0x55, 0x28);
    call(
        0x4cb0b0, button(),
        {21, m.get(owner + 0x3f) + (m.get(0x6de6d5) ? 0x1a69 : 0x1a59), m.get(owner + 0x4f), 0, 1});
    call(0x4cabd0, owner + 0x1f, {1});
    call(0x4cad00, owner + 0x1f, {button(), 1});
    auto ensureScores = [&] {
        U scores = m.get(0x634754);
        if (!scores) {
            scores = p.allocate(12);
            if (scores)
                scores = call(0x4a0d40, scores);
            m.set(0x634754, scores);
        }
        return scores;
    };
    const U count = m.get(ensureScores() + 4);
    m.set(owner + 0x2ad, 0xffffffffu);
    m.set(owner + 0x2a9, 0xffffffffu);
    U total = 0;
    for (U i = 0; static_cast<std::int32_t>(i) < static_cast<std::int32_t>(count); ++i) {
        const U item = call(0x4a0e10, ensureScores(), {i});
        p.text(0x4d28c0, label(2 + i * 2), p.str(item + 4));
        p.text(0x4d28c0, label(3 + i * 2), p.decimal(m.get(owner + 0x53) ? m.get(item) : 0));
        total += m.get(item);
        if (static_cast<std::int32_t>(m.get(item + 0x104)) > -1 &&
            m.get(owner + 0x2a9) == 0xffffffffu)
            m.set(owner + 0x2ad, i);
    }
    p.text(0x4d28c0, label(17), p.decimal(m.get(owner + 0x53) ? total : 0));
    p.text(0x4d28c0, label(18), p.str(0x5da394) + p.decimal(call(0x56c700, 0x6f3a70)));
    if (m.get(owner + 0x53)) {
        const U value = call(0x56c700, 0x6f3a70);
        m.set(0x6e2034, m.get(0x6e2034) + value * 4);
        m.set(0x6e2038, m.get(0x6e2038) + (m.get(0x689924) + 1) * total);
    }
    p.text(0x4d28c0, label(20), p.str(call(0x474b60)));
}
} // namespace mnm::reconstruction

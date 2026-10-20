#include "../reconstruction/rendering/world_dispatch.hpp"
#include <iostream>
#include <vector>

using namespace mnm::reconstruction;
static const char* name(QueueBackend b) {
    switch (b) {
    case QueueBackend::copy: return "copy";
    case QueueBackend::terrain: return "terrain";
    case QueueBackend::clippedCopy: return "clippedCopy";
    case QueueBackend::wordCopy: return "wordCopy";
    case QueueBackend::shadow: return "shadow";
    case QueueBackend::opaque: return "opaque";
    case QueueBackend::quarter: return "quarter";
    case QueueBackend::half: return "half";
    case QueueBackend::threeQuarter: return "threeQuarter";
    case QueueBackend::displacement: return "displacement";
    case QueueBackend::alternateShadow: return "alternateShadow";
    case QueueBackend::distortion: return "distortion";
    case QueueBackend::alternateBlend: return "alternateBlend";
    case QueueBackend::tint: return "tint";
    case QueueBackend::wave: return "wave";
    case QueueBackend::virtualObject: return "virtualObject";
    case QueueBackend::objectHelper: return "objectHelper";
    }
    throw std::logic_error("Unknown backend");
}
int main() try {
    for (unsigned video : {0u, 1u}) for (unsigned auxiliary : {0u, 1u})
    for (unsigned indexed : {0u, 1u}) for (int offset : {-127, 3})
    for (unsigned value : {0u, 1u, 0x1234u, 0xffffu,
            0x10000u, 0x10001u, 0x12340001u, 0xffff0000u, 0xffffffffu})
    for (int kind = -2; kind <= 35; ++kind) {
        QueueRow row{kind, 42, 120, 100, std::int16_t(offset), 7, bool(indexed), {77, 16, 16, 3}};
        std::vector<QueueCall> calls;
        auto backend = [&](const QueueCall& call) { calls.push_back(call); return value; };
        std::uint16_t metadata = 0x8ad0;
        auto dispatch = [&](QueueTable table) {
            const auto result = dispatchQueueRow(table, row, backend);
            if (result) metadata = *result;
        };
        dispatch(video ? QueueTable::primary : QueueTable::software);
        if (video && auxiliary) dispatch(QueueTable::auxiliary);
        std::cout << "{\"video\":" << video << ",\"auxiliary\":" << auxiliary
                  << ",\"indexed\":" << indexed << ",\"offset\":" << offset
                  << ",\"result\":" << value << ",\"kind\":" << kind
                  << ",\"metadata\":" << metadata << ",\"calls\":[";
        for (unsigned i = 0; i < calls.size(); ++i) {
            if (i) std::cout << ',';
            const auto& call = calls[i];
            std::cout << "{\"backend\":\"" << name(call.backend) << "\",\"values\":[";
            for (unsigned j = 0; j < call.count; ++j) {
                if (j) std::cout << ',';
                std::cout << call.values[j];
            }
            std::cout << "]}";
        }
        std::cout << "]}\n";
    }
    // Backend return values may differ across the initial call and fallback.
    QueueRow row{0, 42, 1, 2, -127, -32768, true, {}};
    unsigned calls = 0;
    const auto result = dispatchQueueRow(QueueTable::primary, row, [&](const QueueCall& c) {
        if (calls++ == 0) return 0x12340001u;
        if (c.backend != QueueBackend::clippedCopy || c.values[4] != 0xffff8000u)
            throw std::runtime_error("Signed fallback parameters changed");
        return 0xabcd1234u;
    });
    if (calls != 2 || result != 0x1234) throw std::runtime_error("AX fallback result changed");
    calls = 0;
    if (dispatchQueueRow(QueueTable::primary, row, [&](const QueueCall&) { ++calls; return 0x10000u; }) != 0 || calls != 1)
        throw std::runtime_error("Non-1 low AX must suppress fallback");
    try { dispatchQueueRow(QueueTable(99), row, [](const QueueCall&) { return 0; }); }
    catch (const std::invalid_argument&) { return 0; }
    throw std::runtime_error("Unknown table accepted");
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }

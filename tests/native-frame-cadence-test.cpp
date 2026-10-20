#include "../apps/qt-shell/native_frame_cadence.hpp"
#include <iostream>
#include <stdexcept>

int main() try {
    using Clock = std::chrono::steady_clock;
    const auto epoch = Clock::time_point(std::chrono::seconds(1));
    const auto period = std::chrono::duration_cast<Clock::duration>(std::chrono::seconds(1)) / 60;
    auto deadline = Clock::time_point{};
    std::uint64_t missed = 0;
    const auto require = [](bool value, const char* message) {
        if (!value) throw std::runtime_error(message);
    };
    advanceNativeFrameDeadline(deadline, epoch, missed);
    require(deadline == epoch + period && missed == 0, "Initial deadline");
    // Source imports arrive before the next display tick. They consume one
    // slot without moving the display phase to the import's arrival time.
    advanceNativeFrameDeadline(deadline, epoch + period / 2, missed);
    require(deadline == epoch + 2 * period && missed == 0, "Early source reset display phase");
    advanceNativeFrameDeadline(deadline, deadline + 2 * period + period / 4, missed);
    require(deadline == epoch + 5 * period && missed == 2, "Late turn lost phase or skipped count");
    for (unsigned i = 0; i < 6000; ++i) {
        const auto due = deadline;
        advanceNativeFrameDeadline(deadline, due + period / 10, missed);
        require(deadline == due + period, "Jitter accumulated into display phase");
    }
    require(missed == 2, "Subperiod jitter counted as missed frames");
    const auto due = deadline;
    advanceNativeFrameDeadline(deadline, due + 60 * period, missed);
    require(deadline == due + 61 * period && missed == 62, "Stall did not skip to a future slot");
    require(deadline >= epoch + std::chrono::seconds(101) && deadline <= epoch + std::chrono::seconds(102),
            "Jittered hundred-second schedule drifted");
    std::cout << "{\"success\":true,\"jittered_frames\":6000,\"missed_slots\":62,"
                 "\"early_source_phase_preserved\":true,\"catch_up_bursts\":0}\n";
    return 0;
} catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}

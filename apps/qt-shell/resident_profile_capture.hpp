#pragma once
#include <QJsonObject>
#include <QString>
#include <cstdint>
#include <memory>
#include <vector>
// Diagnostic I/O service. Owns copied bytes only; no widgets, GL contexts,
// original pointers or channel mappings cross into its bounded worker queue.
class ResidentProfileCapture final {
public:
    explicit ResidentProfileCapture(const QString& path,std::uint64_t queueBytes=256ull*1024*1024);
    ~ResidentProfileCapture();
    bool enqueue(std::uint64_t,const std::vector<std::uint8_t>& source,
                 const std::vector<std::uint8_t>& request);
    // Drain only after gameplay retirement; never called per World.
    void finish();
    QJsonObject report() const;
private:
    struct State;
    std::unique_ptr<State> state_;
};

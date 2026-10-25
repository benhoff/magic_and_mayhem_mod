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
    explicit ResidentProfileCapture(const QString& path,std::uint64_t queueBytes=256ull*1024*1024,bool sourceChunks=false);
    ~ResidentProfileCapture();
    bool enqueue(std::uint64_t,const std::vector<std::uint8_t>& source,
                 const std::vector<std::uint8_t>& request);
    bool enqueueSource(std::uint64_t generation,const std::vector<std::uint8_t>& source);
    // Drain only after gameplay retirement; never called per World.
    void finish();
    QJsonObject report() const;
private:
    bool append(std::uint64_t,const std::vector<std::uint8_t>&,const std::vector<std::uint8_t>&,bool);
    struct State;
    std::unique_ptr<State> state_;
};

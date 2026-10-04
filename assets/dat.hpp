#pragma once
#include "asset_file.hpp"
#include <array>
namespace mnm::assets {
// All numeric payloads preserve original DWORD bits, including floating NaNs.
struct DatLayer { std::uint32_t scalarBits=0, tailBits=0; std::vector<std::array<std::uint32_t,12>> nodes; std::vector<std::uint32_t> matrix; };
struct BrainModel { std::uint32_t key=0,dimension=0; std::vector<DatLayer> layers; };
struct BrainDat { std::uint64_t sourceBytes=0; std::vector<BrainModel> models; };
struct ExperienceSample { std::uint32_t key=0; std::vector<std::uint32_t> values; std::array<std::uint32_t,2> parameters{}; };
struct ExperienceDat { std::uint64_t sourceBytes=0; std::vector<ExperienceSample> samples; };
struct DatLimits {
    std::uint64_t inputBytes=32*1024*1024,decodedBytes=64*1024*1024;
    std::uint32_t records=100000,layers=64,nodes=512,values=65536;
};
enum class DatErrorCode {invalidArgument,malformedData,limitExceeded,assetInput};
struct DatError { DatErrorCode code; std::size_t offset; std::string detail; std::optional<Error> input; };
using BrainDatResult=std::variant<BrainDat,DatError>;
using ExperienceDatResult=std::variant<ExperienceDat,DatError>;
// Headerless streams: caller must select schema. Empty input is an empty catalog.
BrainDatResult decodeBrainDat(const std::vector<std::uint8_t>&,const DatLimits& = {});
ExperienceDatResult decodeExperienceDat(const std::vector<std::uint8_t>&,const DatLimits& = {});
BrainDatResult loadBrainDat(AssetFile&,const DatLimits& = {});
ExperienceDatResult loadExperienceDat(AssetFile&,const DatLimits& = {});
}

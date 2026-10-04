#pragma once
#include "asset_file.hpp"
namespace mnm::assets {
enum class TextLineEnding {none,lf,crlf,cr};
struct TextLine {std::size_t offset=0,length=0;TextLineEnding ending=TextLineEnding::none;};
struct TextDocument {
    std::vector<std::uint8_t> source; // Exact bytes; no encoding, whitespace or newline conversion.
    std::vector<TextLine> lines; // Offsets into owned source, excludes newline bytes.
    std::string lineText(std::size_t index) const;
};
struct TextLimits {
    std::uint64_t inputBytes=1024*1024,decodedBytes=4*1024*1024;
    std::uint32_t lines=65536,lineBytes=256*1024,entries=4096;
};
enum class TextErrorCode {invalidArgument,malformedData,unsupportedSyntax,limitExceeded,assetInput};
struct TextError {TextErrorCode code;std::size_t offset;std::string detail;std::optional<Error> input;};
template<class T> using TextResult=std::variant<T,TextError>;
TextResult<TextDocument> decodeText(const std::vector<std::uint8_t>&,const TextLimits& limits={});
TextResult<TextDocument> loadText(AssetFile&,const TextLimits& limits={});
struct ScrollEntry {
    std::uint32_t id=0;
    std::string heading,body; // Bytes between markers, including source whitespace/newlines.
};
struct ScrollText {TextDocument document;std::vector<ScrollEntry> entries;};
// Installed [ScrollN], ~HS/~HE and ~BS/~BE grammar; no display/trigger execution.
TextResult<ScrollText> decodeScrollText(const std::vector<std::uint8_t>&,const TextLimits& limits={});
TextResult<ScrollText> loadScrollText(AssetFile&,const TextLimits& limits={});
}

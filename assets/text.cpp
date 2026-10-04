#include "text.hpp"
#include <algorithm>
#include <charconv>
#include <limits>
#include <new>
#include <set>
#include <stdexcept>
#include <string_view>
namespace mnm::assets {
namespace {
[[noreturn]] void fail(TextErrorCode code,std::size_t at,const char* detail) {throw TextError{code,at,detail,{}};}
void budget(std::uint64_t amount,std::uint64_t& used,std::uint64_t limit,std::size_t at) {
    if(used>limit || amount>limit-used) fail(TextErrorCode::limitExceeded,at,"Text decoded storage limit exceeded");
    used+=amount;
}
std::string_view trim(std::string_view s) {
    while(!s.empty() && (s.front()==' ' || s.front()=='\t')) s.remove_prefix(1);
    while(!s.empty() && (s.back()==' ' || s.back()=='\t')) s.remove_suffix(1);
    return s;
}
template<class T,class F> TextResult<T> guarded(F f) try {return f();}
catch(const TextError& error) {return error;}
catch(const std::bad_alloc&) {return TextError{TextErrorCode::limitExceeded,0,"Text allocation failed",{}};}
catch(const std::length_error&) {return TextError{TextErrorCode::limitExceeded,0,"Text allocation too large",{}};}
template<class T,class F> TextResult<T> load(AssetFile& file,const TextLimits& limits,F decode) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return TextError{TextErrorCode::invalidArgument,0,"Text input limit exceeds file API range",{}};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes))
        return TextError{error->code==ErrorCode::limitExceeded ? TextErrorCode::limitExceeded : TextErrorCode::assetInput,0,error->detail,*error};
    return decode(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
std::string TextDocument::lineText(std::size_t index) const {
    const auto& line=lines.at(index);
    if(line.offset>source.size() || line.length>source.size()-line.offset) throw std::out_of_range("Invalid text line extent");
    if(!line.length) return {};
    return std::string(reinterpret_cast<const char*>(source.data()+line.offset),line.length);
}
TextResult<TextDocument> decodeText(const std::vector<std::uint8_t>& bytes,const TextLimits& limits) {
    return guarded<TextDocument>([&] {
        if(bytes.size()>limits.inputBytes) fail(TextErrorCode::limitExceeded,0,"Text input byte limit exceeded");
        auto nul=std::find(bytes.begin(),bytes.end(),0);
        if(nul!=bytes.end()) fail(TextErrorCode::malformedData,static_cast<std::size_t>(nul-bytes.begin()),"NUL in text input");
        std::uint64_t used=0;budget(bytes.size(),used,limits.decodedBytes,0);
        // Validate/count all lines before allocating source or line metadata.
        std::size_t count=0;
        auto scan=[&](TextDocument* document) {
            std::size_t at=0;
            while(at<bytes.size()) {
                const auto start=at;
                while(at<bytes.size() && bytes[at]!=10 && bytes[at]!=13) ++at;
                const auto length=at-start;
                if(length>limits.lineBytes) fail(TextErrorCode::limitExceeded,start,"Text line length exceeds limit");
                TextLineEnding ending=TextLineEnding::none;
                if(at<bytes.size()) {
                    if(bytes[at++]==10) ending=TextLineEnding::lf;
                    else if(at<bytes.size() && bytes[at]==10) {++at;ending=TextLineEnding::crlf;}
                    else ending=TextLineEnding::cr;
                }
                if(document) document->lines.push_back({start,length,ending});
                else {
                    if(count>=limits.lines) fail(TextErrorCode::limitExceeded,start,"Text line count exceeds limit");
                    ++count;budget(sizeof(TextLine),used,limits.decodedBytes,start);
                }
            }
        };
        scan(nullptr);TextDocument result;result.source=bytes;result.lines.reserve(count);scan(&result);return result;
    });
}
TextResult<TextDocument> loadText(AssetFile& file,const TextLimits& limits) {return load<TextDocument>(file,limits,decodeText);}
TextResult<ScrollText> decodeScrollText(const std::vector<std::uint8_t>& bytes,const TextLimits& limits) {
    return guarded<ScrollText>([&] {
        auto decoded=decodeText(bytes,limits);
        if(const auto* error=std::get_if<TextError>(&decoded)) throw *error;
        ScrollText result;result.document=std::get<TextDocument>(std::move(decoded));
        std::uint64_t used=bytes.size()+result.document.lines.size()*sizeof(TextLine);
        std::set<std::uint32_t> ids;
        std::string_view source;
        if(!bytes.empty()) source=std::string_view(reinterpret_cast<const char*>(bytes.data()),bytes.size());
        std::size_t at=0;
        auto newline=[&] {if(at<source.size() && source[at]=='\r') ++at;if(at<source.size() && source[at]=='\n') ++at;};
        auto skip=[&] {
            while(at<source.size()) {
                auto end=source.find_first_of("\r\n",at);if(end==source.npos) end=source.size();
                auto line=trim(source.substr(at,end-at));
                if(!line.empty() && line.front()!=';') break;
                at=end;newline();
            }
        };
        auto field=[&](const char* open,const char* close) {
            skip();while(at<source.size() && (source[at]==' ' || source[at]=='\t')) ++at;
            if(source.substr(at,3)!=open) fail(TextErrorCode::malformedData,at,"Missing scroll opening marker");
            at+=3;const auto start=at,end=source.find(close,at);
            if(end==source.npos) fail(TextErrorCode::malformedData,start,"Missing scroll closing marker");
            auto nested=source.find('~',at);
            if(nested<end) fail(TextErrorCode::unsupportedSyntax,nested,"Unsupported marker inside scroll field");
            budget(end-start,used,limits.decodedBytes,start);
            std::string value(source.substr(start,end-start));at=end+3;
            while(at<source.size() && (source[at]==' ' || source[at]=='\t')) ++at;
            if(at<source.size() && source[at]!='\r' && source[at]!='\n') fail(TextErrorCode::malformedData,at,"Text after scroll closing marker");
            newline();return value;
        };
        skip();
        while(at<source.size()) {
            const auto start=at;auto end=source.find_first_of("\r\n",at);if(end==source.npos) end=source.size();
            auto header=trim(source.substr(at,end-at));
            if(header.size()<9 || header.substr(0,7)!="[Scroll" || header.back()!=']')
                fail(TextErrorCode::unsupportedSyntax,start,"Expected [ScrollN] declaration");
            auto number=header.substr(7,header.size()-8);std::uint32_t id=0;
            auto parsed=std::from_chars(number.data(),number.data()+number.size(),id);
            if(parsed.ec!=std::errc{} || parsed.ptr!=number.data()+number.size() || !id)
                fail(TextErrorCode::malformedData,start,"Invalid scroll ID");
            if(!ids.insert(id).second) fail(TextErrorCode::malformedData,start,"Duplicate scroll ID");
            if(result.entries.size()>=limits.entries) fail(TextErrorCode::limitExceeded,start,"Scroll entry limit exceeded");
            budget(sizeof(ScrollEntry),used,limits.decodedBytes,start);at=end;newline();
            auto heading=field("~HS","~HE"),body=field("~BS","~BE");
            result.entries.push_back({id,std::move(heading),std::move(body)});skip();
        }
        return result;
    });
}
TextResult<ScrollText> loadScrollText(AssetFile& file,const TextLimits& limits) {return load<ScrollText>(file,limits,decodeScrollText);}
}

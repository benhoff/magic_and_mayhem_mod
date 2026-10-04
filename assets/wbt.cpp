#include "wbt.hpp"
#include <limits>
#include <new>
#include <set>
#include <stdexcept>
#include <string_view>
namespace mnm::assets {
namespace {
[[noreturn]] void fail(TextErrorCode code,std::size_t at,const char* detail) {throw TextError{code,at,detail,{}};}
bool letter(char c) {return (c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_';}
bool digit(char c) {return c>='0' && c<='9';}
std::string fold(std::string_view text) {
    std::string result(text);for(auto& c:result) if(c>='A' && c<='Z') c+= 'a'-'A';return result;
}
struct Parser {
    std::string_view line;std::size_t base=0,at=0;
    void spaces() {while(at<line.size() && (line[at]==' ' || line[at]=='\t')) ++at;}
    void expect(char c) {spaces();if(at==line.size() || line[at]!=c) fail(TextErrorCode::malformedData,base+at,"Unexpected WBT syntax");++at;}
    std::string identifier() {
        spaces();const auto start=at;
        if(at==line.size() || !letter(line[at])) fail(TextErrorCode::malformedData,base+at,"Expected WBT identifier");
        ++at;while(at<line.size() && (letter(line[at]) || digit(line[at]))) ++at;
        return std::string(line.substr(start,at-start));
    }
    void finish() {spaces();if(at!=line.size()) fail(TextErrorCode::unsupportedSyntax,base+at,"Unsupported trailing WBT syntax");}
    WbtArgument argument() {
        spaces();
        if(at<line.size() && line[at]=='"') {
            const auto start=++at;while(at<line.size() && line[at]!='"') ++at;
            if(at==line.size()) fail(TextErrorCode::malformedData,base+start,"Unterminated WBT string");
            auto value=std::string(line.substr(start,at-start));++at;return {WbtArgumentKind::string,std::move(value),false};
        }
        expect('@');auto name=fold(identifier());
        if(name!="false" && name!="true") fail(TextErrorCode::unsupportedSyntax,base+at,"Unsupported WBT constant");
        return {WbtArgumentKind::boolean,{},name=="true"};
    }
};
}
TextResult<WbtScript> decodeWbt(const std::vector<std::uint8_t>& bytes,const TextLimits& limits) try {
    auto decoded=decodeText(bytes,limits);
    if(const auto* error=std::get_if<TextError>(&decoded)) return *error;
    WbtScript result;result.document=std::get<TextDocument>(std::move(decoded));
    std::uint64_t used=bytes.size()+result.document.lines.size()*sizeof(TextLine);
    auto budget=[&](std::uint64_t n,std::size_t at) {
        if(used>limits.decodedBytes || n>limits.decodedBytes-used) fail(TextErrorCode::limitExceeded,at,"WBT decoded storage limit exceeded");
        used+=n;
    };
    std::set<std::string> labels;
    for(const auto& line:result.document.lines) {
        Parser parser{std::string_view(reinterpret_cast<const char*>(bytes.data()+line.offset),line.length),line.offset,0};
        parser.spaces();if(parser.at==parser.line.size() || parser.line[parser.at]==';') continue;
        if(result.statements.size()>=limits.entries) fail(TextErrorCode::limitExceeded,line.offset,"WBT statement count exceeds limit");
        budget(sizeof(WbtStatement),line.offset);WbtStatement statement;statement.offset=line.offset;
        if(parser.line[parser.at]==':') {
            ++parser.at;auto name=parser.identifier();parser.finish();
            if(!labels.insert(fold(name)).second) fail(TextErrorCode::malformedData,line.offset,"Duplicate WBT label");
            statement.operation=WbtOperation::label;statement.arguments.push_back({WbtArgumentKind::identifier,std::move(name),false});
        } else {
            auto name=fold(parser.identifier());
            if(name=="goto") {
                if(parser.at==parser.line.size() || (parser.line[parser.at]!=' ' && parser.line[parser.at]!='\t'))
                    fail(TextErrorCode::malformedData,line.offset+parser.at,"Expected space after WBT Goto");
                statement.operation=WbtOperation::gotoLabel;
                statement.arguments.push_back({WbtArgumentKind::identifier,parser.identifier(),false});parser.finish();
            } else {
                unsigned count=0;
                if(name=="dirchange") {statement.operation=WbtOperation::dirChange;count=1;}
                else if(name=="filedelete") {statement.operation=WbtOperation::fileDelete;count=1;}
                else if(name=="runwait") {statement.operation=WbtOperation::runWait;count=2;}
                else if(name=="filecopy") {statement.operation=WbtOperation::fileCopy;count=3;}
                else fail(TextErrorCode::unsupportedSyntax,line.offset,"Unsupported WBT command");
                parser.expect('(');statement.arguments.reserve(count);
                for(unsigned i=0;i<count;++i) {
                    if(i) parser.expect(',');
                    auto argument=parser.argument();
                    const auto kind=statement.operation==WbtOperation::fileCopy && i==2 ? WbtArgumentKind::boolean : WbtArgumentKind::string;
                    if(argument.kind!=kind) fail(TextErrorCode::malformedData,line.offset+parser.at,"Incorrect WBT argument type");
                    statement.arguments.push_back(std::move(argument));
                }
                parser.expect(')');parser.finish();
            }
        }
        budget(statement.arguments.size()*sizeof(WbtArgument),line.offset);
        for(const auto& argument:statement.arguments) budget(argument.text.size(),line.offset);
        result.statements.push_back(std::move(statement));
    }
    for(const auto& statement:result.statements)
        if(statement.operation==WbtOperation::gotoLabel && !labels.count(fold(statement.arguments[0].text)))
            fail(TextErrorCode::malformedData,statement.offset,"Unknown WBT Goto label");
    return result;
} catch(const TextError& error) {return error;}
  catch(const std::bad_alloc&) {return TextError{TextErrorCode::limitExceeded,0,"WBT allocation failed",{}};}
  catch(const std::length_error&) {return TextError{TextErrorCode::limitExceeded,0,"WBT allocation too large",{}};}
TextResult<WbtScript> loadWbt(AssetFile& file,const TextLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return TextError{TextErrorCode::invalidArgument,0,"WBT input limit exceeds file API range",{}};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes))
        return TextError{error->code==ErrorCode::limitExceeded ? TextErrorCode::limitExceeded : TextErrorCode::assetInput,0,error->detail,*error};
    return decodeWbt(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}

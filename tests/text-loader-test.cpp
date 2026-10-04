#include "wbt.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
std::vector<std::uint8_t> bytes(const std::string& s) {return {s.begin(),s.end()};}
void require(bool value) {if(!value) throw std::runtime_error("Text/WBT assertion failed");}
template<class T> void rejected(const TextResult<T>& result,TextErrorCode code) {
    require(std::holds_alternative<TextError>(result));require(std::get<TextError>(result).code==code);
}
}
int main() try {
    auto source=bytes("\r\nA\rB\n\xff");auto doc=std::get<TextDocument>(decodeText(source));source.clear();
    require(doc.lines.size()==4 && doc.lineText(0).empty() && doc.lineText(3)=="\xff");
    require(doc.lines[0].ending==TextLineEnding::crlf && doc.lines[1].ending==TextLineEnding::cr);
    require(doc.lines[2].ending==TextLineEnding::lf && doc.lines[3].ending==TextLineEnding::none);
    require(std::get<TextDocument>(decodeText({})).lines.empty());
    require(std::get<TextDocument>(decodeText(bytes("a\n"))).lines.size()==1);
    rejected(decodeText({65,0,66}),TextErrorCode::malformedData);
    TextLimits limits;limits.lines=1;rejected(decodeText(bytes("a\nb"),limits),TextErrorCode::limitExceeded);
    limits={};limits.lineBytes=1;rejected(decodeText(bytes("ab"),limits),TextErrorCode::limitExceeded);
    limits={};limits.decodedBytes=1+sizeof(TextLine);require(std::holds_alternative<TextDocument>(decodeText(bytes("a"),limits)));
    --limits.decodedBytes;rejected(decodeText(bytes("a"),limits),TextErrorCode::limitExceeded);
    limits={};limits.inputBytes=0;rejected(decodeText(bytes("a"),limits),TextErrorCode::limitExceeded);
    auto scroll=bytes(";comment\r\n[Scroll2]\n~HS ~HE\n~BS first\r\nsecond~BE\n[Scroll1]\n~HS title ~HE\n~BS body~BE");
    auto book=std::get<ScrollText>(decodeScrollText(scroll));scroll.clear();
    require(book.entries.size()==2 && book.entries[0].id==2 && book.entries[0].heading==" ");
    require(book.entries[0].body==" first\r\nsecond" && book.entries[1].heading==" title ");
    for(auto invalid:{"[Scroll0]\n~HS x~HE\n~BS y~BE", "[Scroll1]\n~HS x", "[Scroll1]\n~HS x~HE\n~BS y~BE\n[Scroll1]"})
        rejected(decodeScrollText(bytes(invalid)),TextErrorCode::malformedData);
    rejected(decodeScrollText(bytes("[Scroll1]\n~HS ~XX~HE\n~BS x~BE")),TextErrorCode::unsupportedSyntax);
    limits={};limits.entries=0;rejected(decodeScrollText(bytes("[Scroll1]\n~HS ~HE\n~BS ~BE"),limits),TextErrorCode::limitExceeded);
    auto script=bytes(";commands are never executed\nGoto START\n:start\nDirChange(\"..\")\nFileDelete(\"x\")\nRunWait(\"a,b\\c\",\"/win/train\")\nFileCopy(\"x\",\"y\",@TRUE)\n");
    auto ast=std::get<WbtScript>(decodeWbt(script));script.clear();
    require(ast.statements.size()==6 && ast.statements[0].operation==WbtOperation::gotoLabel);
    require(ast.statements[4].arguments[0].text=="a,b\\c" && ast.statements[5].arguments[2].boolean);
    rejected(decodeWbt(bytes(":a\n:A")),TextErrorCode::malformedData);
    rejected(decodeWbt(bytes("Goto missing")),TextErrorCode::malformedData);
    rejected(decodeWbt(bytes("Unknown(\"x\")")),TextErrorCode::unsupportedSyntax);
    rejected(decodeWbt(bytes("DirChange(@FALSE)")),TextErrorCode::malformedData);
    rejected(decodeWbt(bytes("FileDelete(\"x)")),TextErrorCode::malformedData);
    rejected(decodeWbt(bytes("FileCopy(\"a\",\"b\",@MAYBE)")),TextErrorCode::unsupportedSyntax);
    limits={};limits.entries=0;rejected(decodeWbt(bytes(":start"),limits),TextErrorCode::limitExceeded);
    limits={};const auto label=bytes(":start");
    limits.decodedBytes=label.size()+sizeof(TextLine)+sizeof(WbtStatement)+sizeof(WbtArgument)+5;
    require(std::holds_alternative<WbtScript>(decodeWbt(label,limits)));
    --limits.decodedBytes;rejected(decodeWbt(label,limits),TextErrorCode::limitExceeded);
    std::cout<<"Owned text lines, raw bytes, scroll grammar and read-only WBT AST/limits passed\n";
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}

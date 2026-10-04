#include "wbt.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
QString hex(const std::string& text) {return QString::fromLatin1(QByteArray(text.data(),static_cast<qsizetype>(text.size())).toHex());}
QJsonObject document(const TextDocument& doc) {
    QJsonArray lines;
    const char* endings[]={"none","lf","crlf","cr"};
    for(std::size_t i=0;i<doc.lines.size();++i) {
        const auto& line=doc.lines[i];lines.append(QJsonObject{{"offset",qint64(line.offset)},{"length",qint64(line.length)},
            {"ending",endings[static_cast<unsigned>(line.ending)]},{"text_hex",hex(doc.lineText(i))}});
    }
    return {{"source_hex",QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(doc.source.data()),static_cast<qsizetype>(doc.source.size())).toHex())},
        {"source_bytes",qint64(doc.source.size())},{"lines",lines}};
}
template<class T> T take(TextResult<T> result) {
    if(const auto* error=std::get_if<TextError>(&result)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    return std::get<T>(std::move(result));
}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3 && argc!=4) throw std::runtime_error("Usage: mnm-text-inspect ROOT PATH [text|scrolls|wbt]");
    const std::string mode=argc==4 ? argv[3] : "text";
    if(mode!="text" && mode!="scrolls" && mode!="wbt") throw std::runtime_error("Unknown text mode");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));QJsonObject output;
    if(mode=="text") {auto text=take(loadText(*file));file.reset();output=document(text);}
    else if(mode=="scrolls") {
        auto text=take(loadScrollText(*file));file.reset();output=document(text.document);QJsonArray entries;
        for(const auto& entry:text.entries) entries.append(QJsonObject{{"id",qint64(entry.id)},{"heading_hex",hex(entry.heading)},{"body_hex",hex(entry.body)}});
        output["entries"]=entries;
    } else {
        auto script=take(loadWbt(*file));file.reset();output=document(script.document);QJsonArray statements;
        const char* operations[]={"label","dir_change","file_delete","run_wait","file_copy","goto"};
        const char* kinds[]={"string","boolean","identifier"};
        for(const auto& statement:script.statements) {
            QJsonArray arguments;
            for(const auto& argument:statement.arguments) arguments.append(QJsonObject{
                {"kind",kinds[static_cast<unsigned>(argument.kind)]},{"text_hex",hex(argument.text)},{"boolean",argument.boolean}});
            statements.append(QJsonObject{{"operation",operations[static_cast<unsigned>(statement.operation)]},
                {"offset",qint64(statement.offset)},{"arguments",arguments}});
        }
        output["statements"]=statements;
    }
    output["mode"]=QString::fromStdString(mode);std::cout<<QJsonDocument(output).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}

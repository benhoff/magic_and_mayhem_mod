#pragma once
#include "text.hpp"
namespace mnm::assets {
enum class WbtOperation {label,dirChange,fileDelete,runWait,fileCopy,gotoLabel};
enum class WbtArgumentKind {string,boolean,identifier};
struct WbtArgument {WbtArgumentKind kind=WbtArgumentKind::string;std::string text;bool boolean=false;};
struct WbtStatement {
    WbtOperation operation=WbtOperation::label;
    std::size_t offset=0;
    std::vector<WbtArgument> arguments;
};
struct WbtScript {TextDocument document;std::vector<WbtStatement> statements;};
// Read-only AST for the installed automation subset. Never runs commands.
TextResult<WbtScript> decodeWbt(const std::vector<std::uint8_t>&,const TextLimits& limits={});
TextResult<WbtScript> loadWbt(AssetFile&,const TextLimits& limits={});
}

#include "commands.hpp"
#include "command_state.hpp"
#include "capture.hpp"
#include <limits>
#include <map>
#include <stdexcept>

namespace mnm::render {
namespace {
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
void imageValid(const Image& image,unsigned width,unsigned height,unsigned bits){
    require(width && height && width<=2048 && height<=2048,"Invalid command image dimensions");
    require(image.width==int(width) && image.height==int(height) && image.pixels.size()==std::size_t(width)*height,
            "Command image dimensions disagree with pixels");
    const auto maximum=bits==32?UINT32_MAX:((1u<<bits)-1);
    for(auto value:image.pixels)require(value<=maximum,"Command pixel exceeds its format");
}
}
namespace detail {
void validateCommandFormat(PixelFormat f){
    require(f.bits==8 || f.bits==16 || f.bits==24 || f.bits==32,"Unsupported command pixel format");
    if(f.bits==8){require(f.masks==std::array<std::uint32_t,3>{},"Indexed masks must be zero");return;}
    const auto maximum=f.bits==32?UINT32_MAX:((1u<<f.bits)-1);
    for(auto mask:f.masks){const auto low=mask&(~mask+1),normal=low?mask/low:0;
        require(mask && mask<=maximum && normal<=255 && !(normal&(normal+1)),"Unsupported command RGB mask");}
    require(!(f.masks[0]&f.masks[1]) && !(f.masks[0]&f.masks[2]) && !(f.masks[1]&f.masks[2]),"Overlapping command masks");
}
const CommandDescription& CommandState::get(unsigned id) const{
    const auto at=live.find(id);require(at!=live.end(),"Unknown or destroyed surface ID");return at->second;
}
const PaletteDescription& CommandState::palette(unsigned id,unsigned generation) const{
    const auto at=palettes.find(id);require(at!=palettes.end() && at->second.generation==generation,"Unknown or stale palette identity");return at->second;
}
void CommandState::accept(const SurfaceCommand& c){
    require((c.version==1 || c.version==2) && (!version || version==c.version),"Changed or unknown command version");
    require(!ended && commands<(mode==CommandStreamMode::Streaming?UINT32_MAX:4096u),"Closed or oversized command session");
    require(c.sequence==commands+1,"Command sequence gap");
    const auto& w=c.words;std::size_t payload=0;
    switch(c.operation){
    case 1:{
        require(w[0] && (mode==CommandStreamMode::Streaming?w[0]>lastCreated:!used.count(w[0])) && live.size()<64,"Reused, zero or excessive surface ID");
        validateCommandFormat(c.format);
        require(c.format.bits==w[3] && c.format.masks==std::array<std::uint32_t,3>{w[4],w[5],w[6]},"Command format fields disagree");
        imageValid(c.image,w[1],w[2],c.format.bits);
        require(pixels+c.image.pixels.size()<=16777216,"Command surface pixel budget exceeded");
        payload=28+c.image.pixels.size()*(c.format.bits/8);break;}
    case 2:{
        const auto& s=get(w[0]);imageValid(c.image,w[3],w[4],s.format.bits);
        require(w[3]<=unsigned(s.width) && w[4]<=unsigned(s.height) && w[1]<=unsigned(s.width)-w[3] && w[2]<=unsigned(s.height)-w[4],"Update outside surface");
        payload=20+c.image.pixels.size()*(s.format.bits/8);break;}
    case 3:{
        const auto& s=get(w[0]);const auto& d=get(w[1]);
        require(w[0]!=w[1] && s.format.bits==d.format.bits && s.format.masks==d.format.masks,"Aliased or incompatible copy");
        require(w[2]<w[4] && w[3]<w[5] && w[4]<=unsigned(s.width) && w[5]<=unsigned(s.height) &&
                w[4]-w[2]<=unsigned(d.width) && w[5]-w[3]<=unsigned(d.height) &&
                w[6]<=unsigned(d.width)-(w[4]-w[2]) && w[7]<=unsigned(d.height)-(w[5]-w[3]),"Copy outside surface");
        require(w[8]<=1 && (w[8] || !w[9]) && (s.format.bits==32 || w[9]<(1u<<s.format.bits)),"Invalid copy key");
        payload=40;break;}
    case 4:{
        const auto& s=get(w[0]);require(c.version==1 && s.format.bits==8 && w[2] && w[1]<256 && w[2]<=256-w[1] && c.colors.size()==w[2],"Invalid palette command");
        payload=12+c.colors.size()*3;break;}
    case 5:case 10:{
        const auto& s=get(w[0]);const auto length=std::size_t(s.width)*s.height*(c.operation==5?s.format.bits/8:4);
        require(std::size_t(c.expected.size())==length,"Invalid check length");payload=4+length;break;}
    case 6:case 7:
        get(w[0]);if(c.operation==6 && c.version==2 && get(w[0]).format.bits==8)require(bindings.count(w[0]) && bindings.at(w[0]),"Presentation lacks palette binding");payload=4;break;
    case 8:require(live.empty() && palettes.empty() && presented,"Incomplete command session");break;
    case 9:throw std::runtime_error("Capture history contains a gap (reason "+std::to_string(w[0])+")");
    case 11:{
        const auto& a=get(w[0]);const auto& b=get(w[1]);
        require(w[0]!=w[1] && a.width==b.width && a.height==b.height && a.format.bits==b.format.bits && a.format.masks==b.format.masks,"Aliased or incompatible surface swap");payload=8;break;}
    case 12:
        require(c.version==2 && w[0] && w[0]>lastPaletteCreated && w[1] && palettes.size()<32 && c.colors.size()==256,"Reused, zero or excessive palette identity");payload=8+768;break;
    case 13:
        require(c.version==2,"Palette resources require stream v2");palette(w[0],w[1]);
        require(w[3] && w[2]<256 && w[3]<=256-w[2] && c.colors.size()==w[3],"Invalid palette resource range");payload=16+c.colors.size()*3;break;
    case 14:
        require(c.version==2 && get(w[0]).format.bits==8,"Palette binding requires indexed surface and v2");
        if(w[1])palette(w[1],w[2]);else require(!w[2],"Null palette generation");payload=12;break;
    case 15:
        require(c.version==2,"Palette resources require stream v2");palette(w[0],w[1]);
        for(const auto& binding:bindings){require(binding.second!=w[0],"Retiring attached palette");}
        payload=8;break;
    default:throw std::runtime_error("Unsupported command opcode");
    }
    if(mode==CommandStreamMode::Bounded)require(payload+12<=std::size_t(maxCommandBytes)-bytes,"Command byte budget exceeded");
    else require(payload+12<=std::size_t(maxStreamingRecordBytes),"Command record budget exceeded");
    version=c.version;
    if(c.operation==1){if(mode==CommandStreamMode::Bounded)used.insert(w[0]);else lastCreated=w[0];live.emplace(w[0],CommandDescription{c.image.width,c.image.height,c.format});pixels+=c.image.pixels.size();}
    else if(c.operation==7){const auto& s=get(w[0]);pixels-=std::size_t(s.width)*s.height;live.erase(w[0]);bindings.erase(w[0]);}
    else if(c.operation==12){lastPaletteCreated=w[0];palettes.emplace(w[0],PaletteDescription{w[1],c.colors});}
    else if(c.operation==13){auto& colors=palettes.at(w[0]).colors;std::copy(c.colors.begin(),c.colors.end(),colors.begin()+w[2]);}
    else if(c.operation==14)bindings[w[0]]=w[1];
    else if(c.operation==15)palettes.erase(w[0]);
    else if(c.operation==6)presented=true;
    else if(c.operation==8)ended=true;
    if(mode==CommandStreamMode::Bounded)bytes+=payload+12;
    ++commands;
}
}
struct CommandConsumer::Impl {
    GlBlitter& renderer;
    std::function<void(GpuFrame)> present;
    CommandConsumerOptions options;
    detail::CommandState admission;
    std::map<unsigned,SurfaceId> handles;
    std::map<unsigned,unsigned> bits;
    CommandResult output;
    CommandConsumerState state=CommandConsumerState::Active;
    bool executing=false;
    Impl(GlBlitter& renderer,std::function<void(GpuFrame)> present,CommandConsumerOptions options):
        renderer(renderer),present(std::move(present)),options(options),admission(options.stream){
        require(options.exportImages || bool(this->present),"GPU execution requires a presentation callback");
        require(!options.exportImages || !this->present,"Image export cannot use a GPU callback");
        require(options.diagnostics==CommandDiagnostics::Skip || options.diagnostics==CommandDiagnostics::Verify,"Invalid diagnostic mode");
        output.driver=renderer.driver();refresh();
    }
    void refresh(){output.stats=renderer.stats();output.liveSurfaces=handles.size();output.livePixels=admission.pixels;output.livePalettes=admission.palettes.size();}
    void cleanup(){for(const auto& pair:handles)renderer.destroy(pair.second);handles.clear();bits.clear();admission.discardSurfaces();refresh();}
    void active(){renderer.stats();require(state==CommandConsumerState::Active,"Command consumer is closed");require(!executing,"Recursive command execution is unsupported");}
    void execute(const SurfaceCommand& c){
        const auto& w=c.words;admission.accept(c);
        switch(c.operation){
        case 1:handles.emplace(w[0],renderer.create(c.image,c.format));bits.emplace(w[0],c.format.bits);break;
        case 2:renderer.update(handles.at(w[0]),int(w[1]),int(w[2]),c.image);break;
        case 3:renderer.copy(handles.at(w[0]),handles.at(w[1]),{int(w[2]),int(w[3]),int(w[4]),int(w[5])},int(w[6]),int(w[7]),
                             w[8]?std::optional<std::uint32_t>(w[9]):std::nullopt);break;
        case 4:renderer.setPalette(handles.at(w[0]),w[1],c.colors);break;
        case 5:
            if(options.diagnostics==CommandDiagnostics::Verify){require(encodeNative(renderer.read(handles.at(w[0])),bits.at(w[0]))==c.expected,"Original native pixels disagree with command replay");++output.checks;}
            else ++output.skippedChecks;
            break;
        case 6:
            if(options.exportImages){output.native=encodeNative(renderer.read(handles.at(w[0])),bits.at(w[0]));output.presentation=renderer.present(handles.at(w[0]));}
            else present(renderer.presentGpu(handles.at(w[0])));
            ++output.presents;break;
        case 7:renderer.destroy(handles.at(w[0]));handles.erase(w[0]);bits.erase(w[0]);break;
        case 8:state=CommandConsumerState::Ended;break;
        case 10:
            if(options.diagnostics==CommandDiagnostics::Verify){const auto image=renderer.present(handles.at(w[0]));
                require(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes())==c.expected,"Original RGBA colors disagree with command replay");++output.colorChecks;}
            else ++output.skippedColorChecks;
            break;
        case 12:case 15:break;
        case 13:
            for(const auto& binding:admission.bindings)if(binding.second==w[0])renderer.setPalette(handles.at(binding.first),w[2],c.colors);
            break;
        case 14:
            if(w[1])renderer.setPalette(handles.at(w[0]),0,admission.palette(w[1],w[2]).colors);
            break;
        case 11:renderer.swapContents(handles.at(w[0]),handles.at(w[1]));break;
        default:throw std::runtime_error("Unsupported replay command");
        }
        ++output.commands;refresh();
    }
};
CommandConsumer::CommandConsumer(GlBlitter& renderer,std::function<void(GpuFrame)> present,CommandConsumerOptions options):
    impl_(std::make_unique<Impl>(renderer,std::move(present),options)){}
CommandConsumer::~CommandConsumer(){try {abort();}catch(const std::exception&) {}}
CommandConsumerState CommandConsumer::state() const{return impl_->state;}
const CommandResult& CommandConsumer::result() const{return impl_->output;}
void CommandConsumer::submit(const SurfaceCommand& command){submit(&command,1);}
void CommandConsumer::submit(const SurfaceCommand* commands,std::size_t count){
    auto& p=*impl_;p.active();p.executing=true;
    try {
        require(!count || commands,"Null command batch");require(count<=4096,"Oversized command batch");
        for(std::size_t i=0;i<count;++i){require(p.state==CommandConsumerState::Active,"Commands after END");p.execute(commands[i]);}
        p.executing=false;
    }catch(...){p.executing=false;p.state=CommandConsumerState::Failed;p.cleanup();throw;}
}
void CommandConsumer::finish(){
    auto& p=*impl_;p.renderer.stats();require(!p.executing,"Recursive command completion is unsupported");
    if(p.state==CommandConsumerState::Ended)return;
    require(p.state==CommandConsumerState::Active,"Command consumer is closed");
    p.state=CommandConsumerState::Failed;p.cleanup();throw std::runtime_error("Missing command stream END");
}
void CommandConsumer::abort(){
    auto& p=*impl_;p.renderer.stats();require(!p.executing,"Recursive command abort is unsupported");
    if(p.state==CommandConsumerState::Active){p.state=CommandConsumerState::Aborted;p.cleanup();}
}
}

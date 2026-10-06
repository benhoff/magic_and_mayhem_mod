#include "commands.hpp"
#include "command_state.hpp"
#include "../protocols/include/mnm/render_stream_v3.h"
#include <QtEndian>
#include <stdexcept>

namespace mnm::render {
namespace {
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
Image pixels(const QByteArray& payload,qsizetype start,unsigned w,unsigned h,unsigned bits){
    require(w && h && w<=2048 && h<=2048,"Invalid command image dimensions");
    require(payload.size()-start==qsizetype(w)*h*(bits/8),"Invalid command pixel length");
    Image image{int(w),int(h),{}};image.pixels.reserve(std::size_t(w)*h);
    while(start<payload.size()){
        std::uint32_t value=0;
        for(unsigned byte=0;byte<bits/8;++byte)value|=std::uint32_t(static_cast<unsigned char>(payload[start++]))<<(byte*8);
        image.pixels.push_back(value);
    }
    return image;
}
}
namespace {
SurfaceCommand decodeRecord(const QByteArray& record,detail::CommandState& state,unsigned version){
    const auto word=[](const QByteArray& b,qsizetype at){return qFromLittleEndian<quint32>(b.constData()+at);};
        SurfaceCommand c;c.version=version;c.operation=word(record,0);c.sequence=word(record,4);const auto length=word(record,8);
        require(c.sequence==state.commands+1 && length==unsigned(record.size()-12),"Command sequence gap or truncated payload");
        const auto p=record.mid(12);
        const auto fields=[&](unsigned n){require(n<=c.words.size() && length>=n*4,"Short command fields");for(unsigned i=0;i<n;++i)c.words[i]=word(p,i*4);};
        if(c.operation==1){
            fields(7);c.format={c.words[3],{c.words[4],c.words[5],c.words[6]}};detail::validateCommandFormat(c.format);
            c.image=pixels(p,28,c.words[1],c.words[2],c.format.bits);
        }else if(c.operation==2){
            fields(5);const auto& s=state.get(c.words[0]);c.image=pixels(p,20,c.words[3],c.words[4],s.format.bits);
        }else if(c.operation==3){fields(10);require(length==40,"Invalid copy length");
        }else if(c.operation==11){fields(2);require(length==8,"Invalid swap length");
        }else if(c.operation==4){
            fields(3);const auto count=c.words[2];require(count && count<=256 && length==12+count*3,"Invalid palette command");
            for(unsigned i=0;i<count;++i)c.colors.push_back({static_cast<std::uint8_t>(p[12+i*3]),
                static_cast<std::uint8_t>(p[13+i*3]),static_cast<std::uint8_t>(p[14+i*3])});
        }else if(version>=2 && (c.operation==12 || c.operation==13)){
            const unsigned n=c.operation==12?2:4;fields(n);
            const unsigned count=c.operation==12?256:c.words[3];
            require(count && count<=256 && length==n*4+count*3,"Invalid palette resource length");
            for(unsigned i=0;i<count;++i)c.colors.push_back({static_cast<std::uint8_t>(p[n*4+i*3]),static_cast<std::uint8_t>(p[n*4+i*3+1]),static_cast<std::uint8_t>(p[n*4+i*3+2])});
        }else if(version>=2 && (c.operation==14 || c.operation==15)){
            const unsigned n=c.operation==14?3:2;fields(n);require(length==n*4,"Invalid palette identity length");
        }else if(version==3 && c.operation==MNM_RENDER_STREAM_V3_OPERATION_CLIPPER_SET){
            fields(3);require(c.words[2]<=MNM_RENDER_STREAM_V3_CLIP_REGION_CAPACITY && length==12+c.words[2]*16,"Invalid clip region length");
            for(unsigned i=0;i<c.words[2];++i){const auto at=12+i*16;
                c.regions.push_back({detail::signedCommandCoordinate(word(p,at)),detail::signedCommandCoordinate(word(p,at+4)),
                    detail::signedCommandCoordinate(word(p,at+8)),detail::signedCommandCoordinate(word(p,at+12))});}
        }else if(version==3 && c.operation==MNM_RENDER_STREAM_V3_OPERATION_SURFACE_COPY){
            fields(MNM_RENDER_STREAM_V3_SURFACE_COPY_WORDS);require(length==52,"Invalid surface copy length");
        }else if(version==3 && c.operation==MNM_RENDER_STREAM_V3_OPERATION_SURFACE_RESULT_CHECK){
            fields(2);require(length==8,"Invalid result check length");
        }else if(c.operation==5 || c.operation==10){fields(1);c.expected=p.mid(4);
        }else if(c.operation==6 || c.operation==7 || c.operation==9){fields(1);require(length==4,"Invalid surface command length");
        }else if(c.operation==8){require(!length,"Invalid END length");
        }else throw std::runtime_error("Unsupported command opcode");
    state.accept(c);return c;
}
}
struct CommandDecoder::Impl {
    detail::CommandState state;
    CommandStreamMode mode;
    explicit Impl(CommandStreamMode mode):state(mode),mode(mode){}
    QByteArray pending;
    qint64 total=0;
    unsigned version=0;
    bool header=false,failed=false,finished=false;
};
CommandDecoder::CommandDecoder(CommandStreamMode mode):impl_(std::make_unique<Impl>(mode)){}
CommandDecoder::~CommandDecoder()=default;
std::vector<SurfaceCommand> CommandDecoder::append(const QByteArray& bytes){
    auto& p=*impl_;
    require(!p.failed && !p.finished,"Closed command decoder");
    try {
        if(p.mode==CommandStreamMode::Bounded)require(bytes.size()<=maxCommandBytes-p.total,"Oversized command stream");
        else {
            require(bytes.size()<=maxStreamingAppendBytes,"Oversized streaming fragment");
            require(bytes.size()<=UINT32_MAX-p.total,"Streaming byte lifetime exhausted");
            require(p.pending.size()+bytes.size()<=maxStreamingRecordBytes+maxStreamingAppendBytes+16,"Streaming retained-byte budget exceeded");
        }
        p.total+=bytes.size();p.pending.append(bytes);
        std::vector<SurfaceCommand> out;
        if(!p.header){
            if(p.pending.size()<16)return out;
            p.version=qFromLittleEndian<quint32>(p.pending.constData()+8);
            require(((p.pending.first(8)=="MNMCMD01" && p.version==1) || (p.pending.first(8)=="MNMCMD02" && p.version==2) ||
                    (p.pending.first(8)==MNM_RENDER_STREAM_V3_MAGIC && p.version==MNM_RENDER_STREAM_V3_VERSION)) &&
                    qFromLittleEndian<quint32>(p.pending.constData()+12)==16,"Invalid command stream header");
            p.pending.remove(0,16);p.header=true;
        }
        qsizetype consumed=0;
        while(consumed<p.pending.size()){
            require(!p.state.ended,"Trailing command stream");
            if(p.pending.size()-consumed<12)break;
            const auto length=qFromLittleEndian<quint32>(p.pending.constData()+consumed+8);
            require(length<=(p.mode==CommandStreamMode::Streaming?maxStreamingRecordBytes-12:maxCommandBytes-28),"Oversized command payload");
            if(p.pending.size()-consumed-12<length)break;
            out.push_back(decodeRecord(p.pending.mid(consumed,12+length),p.state,p.version));consumed+=12+length;
        }
        p.pending.remove(0,consumed);return out;
    }catch(...){p.failed=true;p.pending.clear();throw;}
}
void CommandDecoder::abort(){impl_->failed=true;impl_->pending.clear();}
void CommandDecoder::finish(){
    auto& p=*impl_;
    require(!p.failed,"Failed command decoder");
    if(p.finished)return;
    if(!p.header || !p.pending.isEmpty() || !p.state.ended){p.failed=true;p.pending.clear();throw std::runtime_error("Truncated command stream or missing END");}
    p.finished=true;
}
std::vector<SurfaceCommand> decodeCommands(const QByteArray& data){
    CommandDecoder decoder;auto commands=decoder.append(data);decoder.finish();return commands;
}
CommandResult replayCommands(const std::vector<SurfaceCommand>& commands){
    GlBlitter gl;CommandConsumer consumer(gl,{}, {CommandDiagnostics::Verify,true});
    consumer.submit(commands.data(),commands.size());consumer.finish();return consumer.result();
}
CommandResult replayCommandsGpu(const std::vector<SurfaceCommand>& commands,GlBlitter& renderer,
                               const std::function<void(GpuFrame)>& present){
    CommandConsumer consumer(renderer,present,{CommandDiagnostics::Verify,false});
    consumer.submit(commands.data(),commands.size());consumer.finish();return consumer.result();
}
}

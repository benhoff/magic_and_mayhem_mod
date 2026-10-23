#include "live_command_renderer.hpp"
#include "gl_viewport.hpp"
#include "../../renderer/command_copy_diagnostics.hpp"
#include "../../protocols/include/mnm/render_stream_v5.h"
#include <stdexcept>
#include "cpu_reply_file.hpp"
#include <QtEndian>
#include <cstring>
#include "../../protocols/include/mnm/native_cpu_read_v1.h"
#include <algorithm>
LiveCommandRenderer::LiveCommandRenderer(GlViewport& viewport,bool verify):LiveCommandRenderer(viewportRenderTarget(viewport),verify){}
LiveCommandRenderer::LiveCommandRenderer(NativeRenderTarget target,bool verify):target_(std::move(target)),verify_(verify){}
LiveCommandRenderer::~LiveCommandRenderer(){abort();}
bool LiveCommandRenderer::create(const QString& path,quint32 session,quint32 version){if(!channel_.create(path,session,version)){error_=channel_.error();closed_=true;return false;}cpuReplyPath_=path+".cpu-reply";cpuReplySession_=session;decoder_=std::make_unique<mnm::render::CommandDecoder>(channel_.version()>=2?mnm::render::CommandStreamMode::Streaming:mnm::render::CommandStreamMode::Bounded);return true;}
bool LiveCommandRenderer::open(const QString& path){if(!channel_.open(path)){error_=channel_.error();closed_=true;return false;}decoder_=std::make_unique<mnm::render::CommandDecoder>(channel_.version()>=2?mnm::render::CommandStreamMode::Streaming:mnm::render::CommandStreamMode::Bounded);return true;}
void LiveCommandRenderer::fail(const QString& reason){error_=reason;abort();target_.present({});}
void LiveCommandRenderer::abort(){
    if(cpuReadPending)cpuReadPending(0);
    channel_.cancel();if(decoder_)decoder_->abort();pending_.clear();cursor_=0;
    if(consumer_ && consumer_->state()==mnm::render::CommandConsumerState::Active)consumer_->abort();
    deferredPresentation_={};closed_=true;
}
void LiveCommandRenderer::setPresentationSuspended(bool suspended){
    if(presentationSuspended_==suspended)return;
    presentationSuspended_=suspended;
    if(!suspended && deferredPresentation_.valid()){
        target_.present(std::move(deferredPresentation_));deferredPresentation_={};
        if(framePresented)framePresented();
    }
}
bool LiveCommandRenderer::poll(quint32 budget){
    if(closed_)return false;
    try {
        if(!target_.ready())return true;
        if(!budget || budget>MNM_RENDER_COMMANDS_V1_POLL_BYTES)throw std::runtime_error("Invalid command poll budget");
        if(!consumer_){
            renderer_=std::make_unique<mnm::render::GlBlitter>(target_.context(),target_.offscreen);
            std::shared_ptr<mnm::render::CommandCopyDiagnostics> copyDiagnostics;
            const auto diagnosticPath=qEnvironmentVariable("MNM_NATIVE_COPY_DIAGNOSTICS");
            if(!diagnosticPath.isEmpty())copyDiagnostics=std::make_shared<mnm::render::CommandCopyDiagnostics>(diagnosticPath);
            consumer_=std::make_unique<mnm::render::CommandConsumer>(*renderer_,[this](auto frame){
                if(presentationSuspended_){deferredPresentation_=std::move(frame);return;}
                // setGpuFrame queues an update. Qt coalesces historical frames
                // and composes the latest lease after this bounded poll yields.
                target_.present(std::move(frame));
                if(framePresented)framePresented();
            },mnm::render::CommandConsumerOptions{verify_?mnm::render::CommandDiagnostics::Verify:mnm::render::CommandDiagnostics::Skip,false,channel_.version()>=2?mnm::render::CommandStreamMode::Streaming:mnm::render::CommandStreamMode::Bounded,residentWorld,prepareResidentFrames,reuseSealedUi,[this](const auto& command,const auto& image){publishCpuRead(command,image);},cpuReplySession_,copyDiagnostics});
        }
        // Retain the finite byte budget and the32 ordinary-command budget.
        // A split UPDATE of <=16,384 pixels costs one eighth of an ordinary
        // command, allowing at most256 small uploads in a streaming GUI poll.
        // Copies, CREATE, larger updates and all v1 commands retain full cost.
        const auto importsBefore=consumer_->result().residentWorldImports;
        // Resident World import shares this event loop with command/UI work.
        // Yield between smaller chunks; preserve the same byte cap, FIFO and
        // full command/PRESENT execution. General streaming retains256 work.
        const unsigned workLimit=residentWorld?32u:256u;
        unsigned work=0;quint32 remaining=budget;waitingWorld_=false;
        while(work<workLimit){
            if(cursor_==pending_.size()){
                if(!remaining)break;
                const auto bytes=channel_.poll(remaining);
                remaining-=quint32(bytes.size());
                pending_=decoder_->append(bytes);cursor_=0;
                if(cpuReadPending)for(const auto& command:pending_)if(command.operation==MNM_RENDER_STREAM_V5_OPERATION_CPU_READ){
                    cpuReadPending(std::uint64_t(command.words[1])|(std::uint64_t(command.words[2])<<32));break;
                }
                if(bytes.isEmpty())break;
            }
            const auto first=cursor_;
            while(cursor_<pending_.size()){
                const auto& command=pending_[cursor_];
                if(command.operation==MNM_RENDER_STREAM_V4_OPERATION_WORLD && residentReady){
                    const auto& w=command.words;
                    const auto generation=std::uint64_t(w[1])|(std::uint64_t(w[2])<<32),nonce=std::uint64_t(w[3])|(std::uint64_t(w[4])<<32);
                    if(!residentReady(generation,nonce)){waitingWorld_=true;break;}
                }
                const unsigned cost=channel_.version()>=2 && command.operation==2 &&
                    command.image.pixels.size()<=16384?1:8;
                if(cost>workLimit-work)break;
                work+=cost;++cursor_;
            }
            if(cursor_!=first)consumer_->submit(pending_.data()+first,cursor_-first);
            if(cursor_==pending_.size()){pending_.clear();cursor_=0;}
            else break; // The next command exceeds this poll's remaining work.
        }
        // Publish the newly imported native World through the last compatible
        // sealed UI after leaving all command GL batches. No duplicate draw.
        if(consumer_->state()==mnm::render::CommandConsumerState::Active && consumer_->result().residentWorldImports!=importsBefore)
            consumer_->presentImportedSealed();
        // Diagnostic callbacks may paint explicitly. Normal execution never
        // waits for a display repaint in the command reader.
        if(!target_.error().isEmpty())throw std::runtime_error(target_.error().toStdString());
        if(channel_.state()==MNM_RENDER_COMMANDS_V1_STATE_ENDED && channel_.drained() && pending_.empty()){
            // poll() can expose terminal state before all bounded bytes drain.
            decoder_->finish();consumer_->finish();ended_=true;closed_=true;
        }
        return true;
    }catch(const std::exception& e){fail(e.what());return false;}
}
bool LiveCommandRenderer::finishProducer(){
    if(ended_)return true;
    if(closed_)return false;
    // Drain bounded published bytes after producer exit; never call this while it runs.
    while(!ended_){
        const auto before=channel_.consumed();const auto commandsBefore=consumer_?consumer_->result().commands:0;
        if(!poll())return false;
        if(!ended_ && channel_.consumed()==before && (!consumer_ || consumer_->result().commands==commandsBefore)){fail("Producer exited without complete channel END");return false;}
    }
    return true;
}
std::uint64_t LiveCommandRenderer::preparedGeneration() const{return consumer_ && !closed_ && !ended_?consumer_->preparedGeneration():0;}
bool LiveCommandRenderer::presentPrepared(const mnm::render::GpuFrame& frame,std::uint64_t generation){
    try{if(!consumer_ || closed_ || ended_)return false;consumer_->presentPrepared(frame,generation);return true;}
    catch(const std::exception& e){fail(e.what());return false;}
}
unsigned LiveCommandRenderer::presentations() const{return consumer_?consumer_->result().presents:0;}
const mnm::render::CommandResult* LiveCommandRenderer::result() const{return consumer_?&consumer_->result():nullptr;}

std::uint64_t LiveCommandRenderer::sealedGeneration() const{return consumer_ && !closed_ && !ended_?consumer_->sealedGeneration():0;}
bool LiveCommandRenderer::presentSealed(const mnm::render::GpuFrame& frame,std::uint64_t generation){
    try{if(!consumer_ || closed_ || ended_)return false;consumer_->presentSealed(frame,generation);return true;}
    catch(const std::exception& e){fail(e.what());return false;}
}

mnm::render::CommandDisplayState LiveCommandRenderer::displayState() const{return consumer_ && !closed_ && !ended_?consumer_->displayState():mnm::render::CommandDisplayState{};}

void LiveCommandRenderer::publishCpuRead(const mnm::render::SurfaceCommand& c,const mnm::render::Image& image){
    const auto& w=c.words;
    if(cpuReplyPath_.isEmpty() || !cpuReplySession_ || w[3]!=cpuReplySession_ ||
       image.width!=int(w[4]) || image.height!=int(w[5]) || image.pixels.size()>MNM_NATIVE_CPU_READ_V1_MAX_PIXELS)
        throw std::runtime_error("Explicit CPU reply session/extent mismatch");
    QByteArray reply(MNM_NATIVE_CPU_READ_V1_HEADER+qsizetype(image.pixels.size())*2,0);
    std::memcpy(reply.data(),MNM_NATIVE_CPU_READ_V1_MAGIC,8);
    const quint32 header[]={1,64,w[3],w[1],w[2],w[0],w[4],w[5],quint32(image.pixels.size()*2),w[6],w[7],w[8],w[9],1};
    for(unsigned i=0;i<14;++i)qToLittleEndian(header[i],reply.data()+8+i*4);
    for(std::size_t i=0;i<image.pixels.size();++i){
        if(image.pixels[i]>65535)throw std::runtime_error("Non-WORD explicit CPU reply");
        qToLittleEndian(quint16(image.pixels[i]),reply.data()+64+i*2);
    }
    if(!publishCpuReplyFile(cpuReplyPath_,reply))
        throw std::runtime_error("Explicit CPU reply publication failed");
    if(cpuReadPending)cpuReadPending(0);
}

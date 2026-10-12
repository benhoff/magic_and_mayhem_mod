#include "live_command_renderer.hpp"
#include "gl_viewport.hpp"
#include "../../protocols/include/mnm/render_stream_v4.h"
#include <stdexcept>
#include <algorithm>
LiveCommandRenderer::LiveCommandRenderer(GlViewport& viewport,bool verify):LiveCommandRenderer(viewportRenderTarget(viewport),verify){}
LiveCommandRenderer::LiveCommandRenderer(NativeRenderTarget target,bool verify):target_(std::move(target)),verify_(verify){}
LiveCommandRenderer::~LiveCommandRenderer(){abort();}
bool LiveCommandRenderer::create(const QString& path,quint32 session,quint32 version){if(!channel_.create(path,session,version)){error_=channel_.error();closed_=true;return false;}decoder_=std::make_unique<mnm::render::CommandDecoder>(channel_.version()==2?mnm::render::CommandStreamMode::Streaming:mnm::render::CommandStreamMode::Bounded);return true;}
bool LiveCommandRenderer::open(const QString& path){if(!channel_.open(path)){error_=channel_.error();closed_=true;return false;}decoder_=std::make_unique<mnm::render::CommandDecoder>(channel_.version()==2?mnm::render::CommandStreamMode::Streaming:mnm::render::CommandStreamMode::Bounded);return true;}
void LiveCommandRenderer::fail(const QString& reason){error_=reason;abort();target_.present({});}
void LiveCommandRenderer::abort(){
    channel_.cancel();if(decoder_)decoder_->abort();pending_.clear();cursor_=0;
    if(consumer_ && consumer_->state()==mnm::render::CommandConsumerState::Active)consumer_->abort();
    closed_=true;
}
bool LiveCommandRenderer::poll(quint32 budget){
    if(closed_)return false;
    try {
        if(!target_.ready())return true;
        if(!budget || budget>MNM_RENDER_COMMANDS_V1_POLL_BYTES)throw std::runtime_error("Invalid command poll budget");
        if(!consumer_){
            renderer_=std::make_unique<mnm::render::GlBlitter>(target_.context(),target_.offscreen);
            consumer_=std::make_unique<mnm::render::CommandConsumer>(*renderer_,[this](auto frame){
                // setGpuFrame queues an update. Qt coalesces historical frames
                // and composes the latest lease after this bounded poll yields.
                target_.present(std::move(frame));
                if(framePresented)framePresented();
            },mnm::render::CommandConsumerOptions{verify_?mnm::render::CommandDiagnostics::Verify:mnm::render::CommandDiagnostics::Skip,false,channel_.version()==2?mnm::render::CommandStreamMode::Streaming:mnm::render::CommandStreamMode::Bounded,residentWorld,prepareResidentFrames});
        }
        // Retain the finite byte budget and the32 ordinary-command budget.
        // A split UPDATE of <=16,384 pixels costs one eighth of an ordinary
        // command, allowing at most256 small uploads in a streaming GUI poll.
        // Copies, CREATE, larger updates and all v1 commands retain full cost.
        unsigned work=0;quint32 remaining=budget;waitingWorld_=false;
        while(work<256){
            if(cursor_==pending_.size()){
                if(!remaining)break;
                const auto bytes=channel_.poll(remaining);
                remaining-=quint32(bytes.size());
                pending_=decoder_->append(bytes);cursor_=0;
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
                const unsigned cost=channel_.version()==2 && command.operation==2 &&
                    command.image.pixels.size()<=16384?1:8;
                if(cost>256-work)break;
                work+=cost;++cursor_;
            }
            if(cursor_!=first)consumer_->submit(pending_.data()+first,cursor_-first);
            if(cursor_==pending_.size()){pending_.clear();cursor_=0;}
            else break; // The next command exceeds this poll's remaining work.
        }
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

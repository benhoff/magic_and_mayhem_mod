#include "voice_bridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
class FakeOutput:public QtOutput {
public:
    explicit FakeOutput(Device& device):QtOutput(device){}
    bool admitted=true,active=false;unsigned starts=0;
    bool start(const QAudioDevice&) override{stop();++starts;active=admitted;return active;}
    void stop() override{active=false;QtOutput::stop();}
    bool running() const override{return active;}
    QString lastError() const override{return "Synthetic unavailable output";}
    void lose(){stop();if(failed)failed(lastError());}
};
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
    Device device(1024,8,2000);auto endpoint=std::make_unique<FakeOutput>(device);auto* fake=endpoint.get();
    VoiceOutput output(std::move(endpoint),[]{return QAudioDevice{};},false);
    check(output.start() && output.available() && output.attempts()==1,"initial admission");
    BufferId id=0;check(device.createStatic(0xea,{1,1,1000,2000,2,16,0},20,id)==Error::ok,"create");
    auto& clock=output.timeline();clock.changed(id);check(clock.play(id,1)==Error::ok,"play");
    check(device.setVolume(id,-1200)==Error::ok && device.setPan(id,3333)==Error::ok,"preferences");
    std::vector<std::int16_t> pcm;check(clock.render(10,pcm)==Error::ok,"render");clock.consume(3);
    const auto cursor=clock.cursor(id);fake->admitted=false;fake->lose();
    check(!output.available() && output.losses()==1 && clock.cursor(id)==cursor && device.generatedCursor(id)==cursor,"loss restores consumed position");
    VoiceCommands commands(device,&clock,&output);std::uint32_t request[16]{};request[6]=std::uint32_t(id);request[5]=MNM_AUDIO_STATUS;
    check(commands.execute(request,{}).value==7,"logical playing loop plus buffer-lost status");
    request[5]=MNM_AUDIO_PLAY;check(commands.execute(request,{}).status==0x88780096u,"lost Play refuses without mutation");
    request[5]=MNM_AUDIO_RESTORE;check(commands.execute(request,{}).status==0x88780096u && output.losses()==1,"failed Restore is truthful and preserves outage identity");
    fake->admitted=true;check(commands.execute(request,{}).status==0 && output.available() && output.recoveries()==1,"Restore recovers owned output");
    check(clock.cursor(id)==cursor && device.voice(id)->volume==-1200 && device.voice(id)->pan==3333 && device.voice(id)->status()==5,"restore retains identity, consumed cursor and preferences");
    check(clock.render(2,pcm)==Error::ok,"render after restore");clock.consume(1);
    check(clock.cursor(id)==*cursor+2,"fractional phase retained across loss");
    const auto attempts=output.attempts();check(commands.execute(request,{}).status==0 && output.attempts()==attempts,"available Restore does not restart");
    request[6]=99;check(commands.execute(request,{}).status==0x80070057u,"stale Restore rejected");
    fake->admitted=false;output.rescan();check(!output.available() && output.losses()==2,"changed output retires old endpoint");
    fake->admitted=true;check(output.recover() && output.recoveries()==2,"retry selected output");
    fake->lose();fake->admitted=true;QElapsedTimer elapsed;elapsed.start();
    while(!output.available() && elapsed.elapsed()<1500){app.processEvents();QThread::msleep(1);}
    check(output.available() && output.losses()==3 && output.recoveries()==3,"automatic timed recovery");
    output.stop();check(!output.recover() && !output.available(),"shutdown cannot revive");
    fake->admitted=false;check(!output.start() && !output.recover(),"initial admission failure does not publish recovery-ready output");
    std::cout<<"Native voice output recovery passed: loss/retry/default rescan, consumed phase, sample identity/preferences, buffer-lost status, truthful Restore, stale IDs and shutdown; fake endpoint only\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}

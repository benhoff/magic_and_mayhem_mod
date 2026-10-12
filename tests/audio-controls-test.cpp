#include "../audio/buffers.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void ok(Error result){require(result==Error::ok,"unexpected control failure");}
int main(){
    Device device(1024,16,2000);BufferId id=0;
    const PcmFormat format{1,1,1000,2000,2,16,0};ok(device.createStatic(0xea,format,8,id));
    WriteLock lock;ok(device.lock(id,0,8,0,lock));const unsigned char pcm[]={0,0,0xe8,3,0xd0,7,0xb8,0xb};
    for(unsigned i=0;i<8;++i){lock.first.data[i]=pcm[i];}ok(device.unlock(lock,8,0));
    require(device.frequency(id)==1000&&device.generatedCursor(id)==0,"initial source clock/cursor");
    ok(device.play(id));std::vector<std::int16_t> output;ok(device.mixStereo(1,output));require(output==std::vector<std::int16_t>({0,0}),"initial ramp");
    ok(device.setFrequency(id,2000));ok(device.mixStereo(4,output));
    require(output==std::vector<std::int16_t>({500,500,1500,1500,2500,2500,3000,3000}),"pitch retains fractional phase");
    require(device.voice(id)->playback==Playback::completed&&device.generatedCursor(id)==0,"completed static cursor wraps");
    require(device.info(id)->format->rate==1000&&device.frequency(id)==2000,"source format remains unchanged");
    ok(device.setPosition(id,2));require(device.generatedCursor(id)==2&&device.voice(id)->playback==Playback::stopped,"seek completed voice");
    require(device.setPosition(id,3)==Error::invalid&&device.setPosition(id,8)==Error::invalid&&device.generatedCursor(id)==2,"bad seeks preserve cursor");
    require(device.setFrequency(id,99)==Error::invalid&&device.setFrequency(id,200001)==Error::invalid&&device.frequency(id)==2000,"pitch bounds preserve controls");
    BufferId child=0;ok(device.duplicate(id,child));require(device.frequency(child)==2000&&device.generatedCursor(child)==0,"duplicate controls independent cursor");
    ok(device.setFrequency(child,0));require(device.frequency(child)==1000&&device.frequency(id)==2000,"restore source clock independently");
    ok(device.play(id));ok(device.mixStereo(3,output));require(output==std::vector<std::int16_t>({1000,1000,2000,2000,3000,3000}),"seek playback ramp");
    ok(device.setFrequency(child,100));ok(device.setFrequency(child,200000));
    ok(device.play(child,1));ok(device.mixStereo(2,output));require(device.voice(child)->playback==Playback::playing,"high pitch looping bounds");
    ok(device.setPosition(child,6));ok(device.setFrequency(child,0));ok(device.mixStereo(2,output));
    require(output==std::vector<std::int16_t>({3000,3000,1500,1500})&&device.voice(child)->playback==Playback::playing,"active seek resets phase and retains looping playback");
    BufferId primary=0;ok(device.createPrimary(0x81,primary));require(device.setPosition(primary,0)==Error::unsupported&&device.setFrequency(primary,1000)==Error::unsupported&&!device.generatedCursor(primary),"primary ring excluded");
    ok(device.release(id));require(!device.frequency(id)&&!device.generatedCursor(id)&&device.setPosition(id,0)==Error::invalid,"released controls refuse");
    std::cout<<"Native pitch/seek/generated-cursor controls pass\n";
}

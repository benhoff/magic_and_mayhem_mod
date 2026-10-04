#include "dsound_setup.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
namespace rec=mnm::reconstruction::audio;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void ok(Error e){check(e==Error::ok,"unexpected mixer error");}
Wave wave(std::vector<std::int32_t> samples,std::uint32_t rate,unsigned channels=1,unsigned bits=16){
    const auto align=std::uint16_t(channels*(bits/8));
    Wave result{{1,std::uint16_t(channels),rate,rate*align,align,std::uint16_t(bits),0},{}};
    for(auto value:samples){
        if(bits==8)result.samples.push_back(std::uint8_t(value/256+128));
        else{const auto encoded=std::uint16_t(value);result.samples.push_back(std::uint8_t(encoded));result.samples.push_back(std::uint8_t(encoded>>8));}
    }
    return result;
}
BufferId upload(Device& device,const Wave& data){auto r=rec::uploadStatic(device,data);ok(r.error);return r.buffer;}
std::vector<std::int16_t> mix(Device& d,std::size_t n){std::vector<std::int16_t> out;ok(d.mixStereo(n,out));check(out.size()==2*n,"stereo output length");return out;}
void exactFixtures(){
    Device d(1024,16,22050);std::vector<std::int16_t> out{77};ok(d.mixStereo(2,out));check(out==std::vector<std::int16_t>(4,0),"empty mixer silence");
    auto mono=upload(d,wave({-32768,-1,0,32767},22050));ok(d.play(mono));
    check(mix(d,5)==std::vector<std::int16_t>({-32768,-32768,-1,-1,0,0,32767,32767,0,0}),"signed16 decode and mono duplication");
    check(d.voice(mono)->playback==Playback::completed,"render completes one-shot");
    auto stereo=upload(d,wave({-256,256,512,-512},22050,2,8));ok(d.play(stereo));
    check(mix(d,3)==std::vector<std::int16_t>({-256,256,512,-512,0,0}),"unsigned8 stereo conversion");
    Device up(1024,16,4);auto ramp=upload(up,wave({0,1000,2000},2));ok(up.play(ramp));
    check(mix(up,7)==std::vector<std::int16_t>({0,0,500,500,1000,1000,1500,1500,2000,2000,2000,2000,0,0}),"upsampling and clamped last frame");
    ok(up.play(ramp,1));check(mix(up,7)==std::vector<std::int16_t>({0,0,500,500,1000,1000,1500,1500,2000,2000,1000,1000,0,0}),"loop interpolation wraps to first sample");
    Device down(1024,16,2);auto fast=upload(down,wave({0,1000,2000,3000},4));ok(down.play(fast));
    check(mix(down,3)==std::vector<std::int16_t>({0,0,2000,2000,0,0}),"downsampling advances whole skipped frames");
}
void controlsAndClipping(){
    Device d(1024,16,22050);auto a=upload(d,wave({10000},22050));ok(d.play(a,1));
    ok(d.setVolume(a,-2000));check(mix(d,1)==std::vector<std::int16_t>({1000,1000}),"volume uses decibel amplitude");
    ok(d.setPan(a,2000));check(mix(d,1)==std::vector<std::int16_t>({100,1000}),"positive pan attenuates left");
    ok(d.setPan(a,-2000));check(mix(d,1)==std::vector<std::int16_t>({1000,100}),"negative pan attenuates right");
    ok(d.setPan(a,10000));check(mix(d,1)==std::vector<std::int16_t>({0,1000}),"pan endpoint hard mute policy");
    ok(d.setVolume(a,-10000));check(mix(d,1)==std::vector<std::int16_t>({0,0}),"volume endpoint mute");
    ok(d.setVolume(a,0));ok(d.setPan(a,0));BufferId b=0;ok(d.duplicate(a,b));ok(d.play(b,1));
    check(mix(d,1)==std::vector<std::int16_t>({20000,20000}),"independent overlapping voices sum");
    auto positive=upload(d,wave({32767},22050));ok(d.play(positive,1));
    check(mix(d,1)==std::vector<std::int16_t>({32767,32767}),"positive clipping");
    auto negative=upload(d,wave({-32768},22050));ok(d.play(negative,1));
    check(mix(d,1)==std::vector<std::int16_t>({19999,19999}),"clip after all voices sum, not per voice");
    ok(d.stop(a));ok(d.stop(b));ok(d.stop(positive));BufferId n2=0;ok(d.duplicate(negative,n2));ok(d.play(n2,1));
    check(mix(d,1)==std::vector<std::int16_t>({-32768,-32768}),"negative clipping");
}
// Independent reference derives each position from absolute output time and
// interpolates an integer weighted numerator, not the implementation's phase loop.
void resamplingOracle(){
    const std::vector<std::int32_t> mono={-32768,-4096,0,8192,16384,-16384,4096,32512};
    for(unsigned channels:{1u,2u})for(unsigned bits:{8u,16u})
    for(auto sourceRate:{8000u,11025u,22050u,44100u,48000u})
    for(auto outputRate:{8000u,22050u,48000u})for(bool loop:{false,true}){
        auto data=mono;if(channels==2){data.clear();for(auto value:mono){data.push_back(value);data.push_back(-value/2);}}
        // 8-bit source must be representable in that format.
        if(bits==8)for(auto& value:data)value=(value/256)*256;
        auto source=wave(data,sourceRate,channels,bits);Device full(1024,16,outputRate),split(1024,16,outputRate);
        auto a=upload(full,source),b=upload(split,source);ok(full.play(a,loop?1:0));ok(split.play(b,loop?1:0));
        auto actual=mix(full,300);std::vector<std::int16_t> pieces;
        for(auto block:{1u,0u,7u,23u,269u}){auto part=mix(split,block);pieces.insert(pieces.end(),part.begin(),part.end());}
        check(actual==pieces,"resampling must be bit-identical across block splits");
        check(full.voice(a)->frame==split.voice(b)->frame && full.voice(a)->status()==split.voice(b)->status(),"split final state");
        for(std::uint64_t frame=0;frame<300;++frame)for(unsigned channel=0;channel<2;++channel){
            const auto numerator=frame*sourceRate;auto at=numerator/outputRate;
            std::int64_t expected=0;
            if(loop || at<mono.size()){
                at%=mono.size();auto next=at+1;
                if(next==mono.size())next=loop?0:at;
                const auto col=channels==1?0:channel;
                const auto remainder=std::int64_t(numerator%outputRate);
                const auto weighted=std::int64_t(data[at*channels+col])*(outputRate-remainder)+
                                    std::int64_t(data[next*channels+col])*remainder;
                expected=std::llround(double(weighted)/outputRate);
            }
            check(std::abs(std::int64_t(actual[frame*2+channel])-expected)<=1,"independent rational PCM reference");
        }
    }
}
void lifecycleAndPhase(){
    Device d(1024,16,4);auto a=upload(d,wave({0,1000,2000},2));ok(d.play(a,1));mix(d,1);
    ok(d.stop(a));check(mix(d,2)==std::vector<std::int16_t>(4,0),"stopped voices contribute silence");
    ok(d.play(a,1));check(mix(d,1)==std::vector<std::int16_t>({500,500}),"fraction retained across stop resume");
    ok(d.resetPosition(a));check(mix(d,1)==std::vector<std::int16_t>({0,0}),"reset clears fraction");
    BufferId child=0;ok(d.duplicate(a,child));ok(d.stop(a));ok(d.play(child,1));
    check(mix(d,1)==std::vector<std::int16_t>({0,0}),"duplicate begins with zero fraction");
    ok(d.setPan(child,10000));check(mix(d,1)==std::vector<std::int16_t>({0,500}),"control change affects next block");
    ok(d.stop(child));ok(d.resetPosition(child));ok(d.setPan(child,0));ok(d.play(child,1));
    WriteLock lock;ok(d.lock(a,0,2,0,lock));lock.first.data[0]=0xe8;lock.first.data[1]=3;
    check(mix(d,1)==std::vector<std::int16_t>({0,0}),"mix reads committed storage only");
    ok(d.unlock(lock,2,0));ok(d.resetPosition(child));ok(d.release(a));
    check(mix(d,1)==std::vector<std::int16_t>({1000,1000}),"surviving duplicate sees committed update");
    auto before=d.voice(child);std::vector<std::int16_t> output{77,88};
    check(d.mixStereo(Device::maxMixFrames+1,output)==Error::limit && output==std::vector<std::int16_t>({77,88}),"limit preserves caller output");
    check(d.voice(child)->frame==before->frame,"limit preserves cursor");
    ok(d.release(child));check(mix(d,2)==std::vector<std::int16_t>(4,0),"released voices disappear");
    bool rejected=false;try{Device invalid(1024,16,0);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"zero output rate rejected");
}
}
int main(){try{exactFixtures();controlsAndClipping();resamplingOracle();lifecycleAndPhase();
    std::cout<<"Stereo mixer passed; 120 resampling scenarios with independent PCM reference; no audio device\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

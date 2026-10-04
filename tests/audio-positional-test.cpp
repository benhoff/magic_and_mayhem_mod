#include "positional_audio.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace r=mnm::reconstruction::audio;
using namespace mnm::audio;
void check(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
struct Backend:r::PositionalBackend {
    std::string calls;int volumeFault=0,stopFault=0,resetFault=0,queryFault=0;bool playing=true;
    r::Status getStatus(unsigned,unsigned& f)override{calls+='Q';f=playing?1:0;return queryFault;}
    r::Status stop(unsigned)override{calls+='S';return stopFault;}
    r::Status position(unsigned,unsigned)override{calls+='Z';return resetFault;}
    r::Status volume(unsigned,int)override{calls+='V';return volumeFault;}
    r::Status pan(unsigned,int)override{calls+='P';return 17;}
    r::Status play(unsigned,unsigned,unsigned,unsigned)override{return 0;}
    void volumeRecord(r::VoiceContract&,int)override{}
    void clearVoiceSchedule(unsigned)override{calls+='C';}
    void releaseBuffer(unsigned)override{}
    void freeWrapper(unsigned)override{}
    void positionalVolumeRecord(unsigned,int)override{calls+='N';}
};
void geometry(){
    for(int extent=7;extent<=21;++extent)for(int delta=-255;delta<=255;++delta){
        int expected=delta;while(expected<0)expected+=extent;while(expected>=extent)expected-=extent;if(expected>extent/2)expected-=extent;
        check(r::wrappedDifference(delta,0,extent)==expected,"wrapped coordinate oracle and positive half tie");
    }
    const int bx[]={-6,6,6,-6},by[]={-6,-6,6,6};
    for(unsigned rotation=0;rotation<4;++rotation)for(int x=-8;x<=8;++x)for(int y=-8;y<=8;++y)for(int range:{7,31}){
        r::PositionalInput in;in.sourceX=100+bx[rotation]+x;in.sourceY=100+by[rotation]+y;in.listenerX=100;in.listenerY=100;in.mapWidth=256;in.mapHeight=256;in.range=range;in.panWidth=5;in.orientation=rotation;
        const auto out=r::positionalControls(in);const int ax=x<0?-x:x,ay=y<0?-y:y;
        const int distance=(ax>ay?ax+ay/2:ay+ax/2);
        const int volume=distance>=range?-5000:-((distance*5000+range-1)/range);
        const int lateral=rotation==0?(x-y)/2:rotation==1?(x+y)/2:rotation==2?-((x-y)/2):-((x+y)/2);
        int pan=lateral*3333/5;if(lateral>=5)pan=3333;if(lateral<=-5)pan=-3333;
        check(out.volume==volume && out.panWritten==(volume>-5000),"independent integer distance attenuation oracle");
        if(out.panWritten)check(out.pan==pan,"rotation, signed half truncation and saturation");
    }
    r::PositionalInput in;in.listenerX=6;in.listenerY=6;in.mapWidth=128;in.mapHeight=128;in.range=100;in.panWidth=5;
    check(r::positionalControls(in).volume==0,"biased listener center");
    for(int byte:{-127,-64,-1,0,64}){in.mapByte=std::int8_t(byte);auto out=r::positionalControls(in);
        if(byte>=0)check(out.volume==0,"map byte at/above threshold bypasses adjustment");
        else check(out.volume>=-5000 && out.volume<0 && out.panWritten==(byte!=-127),"signed byte attenuation and silence boundary");}
    in.mapByte=std::int8_t(-127);check(r::positionalControls(in).volume==-5000,"map byte minus127 fully attenuates");
    in.mapByte=std::int8_t(-128);in.byteThreshold=-127;bool rejected=false;try{r::positionalControls(in);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"zero divisor outside recovered valid domain");
    in.mapByte.reset();in.orientation=4;rejected=false;try{r::positionalControls(in);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"invalid orientation not stale-global emulation");
}
void updateMatrix(){
    for(bool world:{false,true})for(bool enabled:{false,true})for(bool audible:{false,true})for(bool cached:{false,true})
    for(int volume:{0,5,-1})for(int stop:{0,7,-1})for(int reset:{0,9,-1}){
        Backend b;b.volumeFault=volume;b.stopFault=stop;b.resetFault=reset;r::VoiceWrapper v;v.buffer=11;v.identity=1;v.cachedVolume=cached?-1000:99;auto* slot=&v;
        r::PositionalControls c{audible?-1000:-5000,123,audible};const auto result=r::updatePositionalVoice(b,slot,c,world,enabled);
        std::string expected;int wanted=0;
        if(world && enabled){if(audible){if(!cached){expected+='V';if(!volume)expected+='N';}expected+='P';}
            else{expected="QSC";wanted=stop;if(!stop){expected+='Z';wanted=reset;}}}
        check(b.calls==expected && result==wanted,"world/manager gates, scheduler notifications and error returns");
        check((slot==nullptr)==(world && !audible),"inaudible branch clears slot even on disabled/error path");
        if(world && audible)check(v.cachedVolume==-1000,"cache writes precede rejected native volume");
    }
    for(bool playing:{false,true})for(int query:{0,5}){Backend b;b.playing=playing;b.queryFault=query;r::VoiceWrapper v;v.buffer=11;auto* slot=&v;
        r::updatePositionalVoice(b,slot,{},true,true);check(b.calls==(playing || query?"QSCZ":"Q") && !slot,"idle/error query ordering");}
    Backend b;r::VoiceWrapper v;auto* slot=&v;r::updatePositionalVoice(b,slot,{},true,true);check(!slot && b.calls.empty(),"absent buffer clears slot without controls");
}
void nativePcm(){
    struct Native:Backend {
        Device& d;explicit Native(Device& device):d(device){}
        r::Status getStatus(unsigned id,unsigned& f)override{f=d.voice(id)->status();return 0;}
        r::Status stop(unsigned id)override{return d.stop(id)==Error::ok?0:-1;}
        r::Status position(unsigned id,unsigned)override{return d.resetPosition(id)==Error::ok?0:-1;}
        r::Status volume(unsigned id,int v)override{return d.setVolume(id,v)==Error::ok?0:-1;}
        r::Status pan(unsigned id,int v)override{return d.setPan(id,v)==Error::ok?0:-1;}
    };
    Device d;BufferId id=0;PcmFormat format{1,1,48000,96000,2,16,0};d.createStatic(0xea,format,2,id);WriteLock write;d.lock(id,0,2,0,write);write.first.data[0]=0x10;write.first.data[1]=0x27;d.unlock(write,2,0);d.play(id,1);
    r::VoiceWrapper v;v.buffer=unsigned(id);v.identity=1;auto* slot=&v;Native b(d);
    r::PositionalInput in;in.listenerX=6;in.listenerY=6;in.sourceX=2;in.mapWidth=128;in.mapHeight=128;in.range=10;in.panWidth=1;
    auto c=r::positionalControls(in);check(c.volume==-1000 && c.pan==3333,"known geometry to native controls");r::updatePositionalVoice(b,slot,c,true,true);
    std::vector<std::int16_t> pcm;d.mixStereo(1,pcm);check(pcm==std::vector<std::int16_t>({68,3162}),"distance and pan produce independently expected stereo PCM");
    in.sourceX=11;c=r::positionalControls(in);r::updatePositionalVoice(b,slot,c,true,true);d.mixStereo(1,pcm);
    check(!slot && pcm[0]==0 && !d.voice(id)->status() && d.count()==1,"out of range stops/reset clears slot without releasing sample");
}
int main(){try{geometry();updateMatrix();nativePcm();std::cout<<"Positional audio passed: 2312 geometry and 432 update cases; wrap, signed map bytes and native stereo PCM\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

#include "buffers.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
using namespace mnm::audio;
// Fail each allocation in turn, including the map node/rehash after storage.
static long allocationCountdown=-1;
void* operator new(std::size_t n){
    if(allocationCountdown==0)throw std::bad_alloc();
    if(allocationCountdown>0)--allocationCountdown;
    if(void* p=std::malloc(n?n:1))return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
void operator delete[](void* p,std::size_t) noexcept{std::free(p);}
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void ok(Error e){check(e==Error::ok,"unexpected Device error");}
PcmFormat format(unsigned channels=1,unsigned bits=16,std::uint32_t rate=48000){
    auto alignment=std::uint16_t(channels*(bits/8));
    return {1,std::uint16_t(channels),rate,rate*alignment,alignment,std::uint16_t(bits),0};
}
void word(std::vector<std::uint8_t>& b,std::uint32_t v,unsigned n){for(unsigned i=0;i<n;++i)b.push_back(std::uint8_t(v>>(8*i)));}
void chunk(std::vector<std::uint8_t>& b,const char* name,const std::vector<std::uint8_t>& data){
    b.insert(b.end(),name,name+4);word(b,std::uint32_t(data.size()),4);b.insert(b.end(),data.begin(),data.end());if(data.size()%2)b.push_back(0xff);
}
std::vector<std::uint8_t> riff(const PcmFormat& f,int fmtSize=16,bool duplicateFmt=false,bool duplicateData=false,bool omitFmt=false,bool omitData=false){
    std::vector<std::uint8_t> b={'R','I','F','F',0,0,0,0,'W','A','V','E'},fmt;
    word(fmt,f.tag,2);word(fmt,f.channels,2);word(fmt,f.rate,4);word(fmt,f.bytesPerSecond,4);word(fmt,f.alignment,2);word(fmt,f.bits,2);
    if(fmtSize>=18)word(fmt,f.extra,2);
    fmt.resize(std::size_t(fmtSize),0);
    chunk(b,"JUNK",{3});if(!omitData)chunk(b,"data",std::vector<std::uint8_t>(f.alignment,0x80));
    if(!omitFmt)chunk(b,"fmt ",fmt);
    if(duplicateFmt)chunk(b,"fmt ",fmt);
    if(duplicateData)chunk(b,"data",{0,0});
    const auto size=b.size()-8;for(unsigned i=0;i<4;++i)b[4+i]=std::uint8_t(size>>(8*i));return b;
}
void rejects(const std::vector<std::uint8_t>& b){bool rejected=false;try{(void)readWave(b);}catch(const std::runtime_error&){rejected=true;}check(rejected,"invalid WAV accepted");}
void formats(){
    for(unsigned channels:{1u,2u})for(unsigned bits:{8u,16u})for(auto rate:{1u,8000u,11025u,22050u,44100u,48000u}){
        auto f=format(channels,bits,rate);check(validPcm(f),"supported format rejected");
        for(auto size:{16,18,20}){auto file=riff(f,size);auto w=readWave(file);check(w.samples==std::vector<std::uint8_t>(f.alignment,0x80),"PCM/padding boundary");file.insert(file.end(),{9,8,7});check(readWave(file).samples==w.samples,"bytes beyond RIFF boundary");}
        Device d(16,2,rate);BufferId id=0;ok(d.createStatic(0xea,f,2*f.alignment,id));
        check(d.samples(id)==std::vector<std::uint8_t>(2*f.alignment,bits==8?128:0),"new PCM must be silence");ok(d.play(id));
        std::vector<std::int16_t> pcm;ok(d.mixStereo(3,pcm));check(pcm==std::vector<std::int16_t>(6,0),"unwritten PCM audible");
        check(d.voice(id)->playback==Playback::completed,"silence completion");
    }
    auto f=format();
    for(unsigned field=0;field<8;++field){auto bad=f;switch(field){case 0:bad.tag=3;break;case 1:bad.channels=3;break;case 2:bad.bits=24;break;case 3:bad.rate=0;break;case 4:bad.alignment=4;break;case 5:++bad.bytesPerSecond;break;case 6:bad.extra=1;break;default:bad.rate=std::numeric_limits<std::uint32_t>::max();break;}
        check(!validPcm(bad),"invalid format accepted");rejects(riff(bad,18));Device d;BufferId out=77;
        check(d.createStatic(0xea,bad,4,out)==Error::badFormat && out==77 && !d.count(),"format failure published buffer");
    }
    rejects(riff(f,15));rejects(riff(f,17));rejects(riff(f,16,true));rejects(riff(f,16,false,true));rejects(riff(f,16,false,false,true));rejects(riff(f,16,false,false,false,true));
    auto odd=riff(format(1,8));odd.pop_back();rejects(odd); // shortened container
    auto file=riff(f);for(std::size_t n=0;n<file.size();++n)rejects({file.begin(),file.begin()+n});
}
void lifetime(){
    Device d;BufferId source=0,child=0;ok(d.createStatic(0xea,format(1,8),4,source));ok(d.duplicate(source,child));
    WriteLock lock;ok(d.lock(source,3,3,0,lock));lock.first.data[0]=255;lock.second.data[0]=0;lock.second.data[1]=129;
    ok(d.play(child,1));std::vector<std::int16_t> pcm;ok(d.mixStereo(4,pcm));check(pcm==std::vector<std::int16_t>(8,0),"pending bytes mixed");
    auto forged=lock;forged.second.data=nullptr;check(d.unlock(forged,1,2)==Error::invalid,"forged region accepted");
    ok(d.unlock(lock,1,2));ok(d.mixStereo(4,pcm));check(pcm==std::vector<std::int16_t>({-32768,-32768,256,256,0,0,32512,32512}),"committed split PCM");
    ok(d.lock(source,0,4,0,lock));std::fill_n(lock.first.data,4,128);ok(d.release(child));BufferId survivor=0;ok(d.duplicate(source,survivor));
    WriteLock sentinel{999,777,{nullptr,5},{nullptr,7}};
    check(d.lock(survivor,0,4,0,sentinel)==Error::busy && sentinel.owner==999,"nonowner release discarded lock");
    ok(d.release(source));check(d.unlock(lock,4,0)==Error::invalid && d.samples(survivor)[0]==0,"released writer committed");
    ok(d.lock(survivor,0,4,0,lock));const auto old=lock;ok(d.unlock(lock,0,0));check(d.info(survivor)->revision==1,"empty commit changes revision");
    ok(d.lock(survivor,0,4,0,lock));check(d.unlock(old,4,0)==Error::invalid,"stale ticket accepted");ok(d.unlock(lock,0,0));
    ok(d.release(survivor));check(!d.count() && d.release(survivor)==Error::invalid,"double release");
    {Device scoped;BufferId id=0;ok(scoped.createStatic(0xea,format(),4,id));ok(scoped.play(id,1));ok(scoped.lock(id,0,4,0,lock));} // sanitizer checks locked, playing destruction
}
void failures(){
    Device d(4,1);BufferId id=0,out=777;auto f=format();ok(d.createStatic(0xea,f,4,id));
    check(d.createStatic(0xea,f,4,out)==Error::limit && d.duplicate(id,out)==Error::limit && out==777 && d.count()==1,"capacity failure mutation");
    ok(d.play(id,1));auto before=d.voice(id);std::vector<std::int16_t> pcm{55};
    check(d.mixStereo(Device::maxMixFrames+1,pcm)==Error::limit && pcm==std::vector<std::int16_t>{55} && d.voice(id)->frame==before->frame,"mix limit mutation");
    check(d.play(id,2)==Error::unsupported && d.setVolume(id,1)==Error::invalid && d.setPan(id,10001)==Error::invalid && d.voice(id)->status()==5,"control failure mutation");
    bool rejected=false;try{Device invalid(4,1,0);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"zero clock accepted");
}
void allocationFailures(){
    for(unsigned operation=0;operation<5;++operation){bool succeeded=false;unsigned failures=0;
        for(long fail=0;fail<32 && !succeeded;++fail){Device d;BufferId id=0,out=777;auto f=format();
            if(operation>=2)ok(d.createStatic(0xea,f,4,id));
            if(operation==4)ok(d.play(id,1));
            const auto count=d.count();WriteLock lock{999,777,{nullptr,5},{nullptr,7}};std::vector<std::int16_t> pcm{55};
            allocationCountdown=fail;
            try{switch(operation){case 0:ok(d.createStatic(0xea,f,4,out));break;case 1:ok(d.createPrimary(0x81,out));break;case 2:ok(d.duplicate(id,out));break;case 3:ok(d.lock(id,0,4,0,lock));break;default:ok(d.mixStereo(2,pcm));break;}succeeded=true;}
            catch(const std::bad_alloc&){allocationCountdown=-1;++failures;check(d.count()==count && out==777,"allocation failure published identity");
                if(operation>=2)check(d.info(id)->revision==0 && d.voice(id)->frame==0,"allocation failure changed source");
                if(operation==3){check(lock.owner==999 && lock.ticket==777,"allocation failure published lock");ok(d.lock(id,0,4,0,lock));ok(d.unlock(lock,0,0));}
                if(operation==4)check(pcm==std::vector<std::int16_t>{55} && d.voice(id)->status()==5,"allocation failure advanced mixer");
                if(operation<=1){ok(d.createPrimary(0x81,out));check(out==1,"failed creation consumed identity");}
            }
            allocationCountdown=-1;
        }
        check(succeeded && failures>0,"allocation fault sweep incomplete");
    }
}
}
int main(){try{formats();lifetime();failures();allocationFailures();std::cout<<"Native audio boundaries passed: 24 PCM combinations, RIFF, lifetime, failure and five allocation sweeps\n";return 0;}
catch(const std::exception& e){allocationCountdown=-1;std::cerr<<e.what()<<'\n';return 1;}}

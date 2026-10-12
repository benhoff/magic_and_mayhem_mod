#include "../compat/legacy/canvas_producers.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
using Bytes=std::vector<std::uint8_t>;
void put(Bytes& b,unsigned at,unsigned v){for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(v>>(8*i));}
void require(bool v){if(!v)throw std::runtime_error("Whole-update assertion");}
template<class F>void refuses(F f){try{f();}catch(const std::exception&){return;}throw std::runtime_error("Whole-update refusal missing");}
Bytes wire(unsigned queues){
 Bytes b(64);std::memcpy(b.data(),"MNMPRO04",8);put(b,8,4);put(b,12,64);put(b,16,0x40209ca7);put(b,20,1048576);put(b,24,queues);put(b,28,1);put(b,32,536870912);unsigned sequence=0;
 auto row=[&](std::array<unsigned,24> r){r[0]=96;r[1]=++sequence;auto at=b.size();b.resize(at+96);for(unsigned i=0;i<24;++i)put(b,unsigned(at)+4*i,r[i]);};
 std::array<unsigned,24> r{};r[2]=1;r[3]=1;r[5]=r[6]=r[7]=2;row(r);
 for(unsigned q=1;q<=queues;++q){r={};r[2]=11;r[14]=q;row(r);r[2]=12;row(r);}
 r={};r[2]=5;r[3]=1;r[12]=r[13]=2;r[14]=0x07e0;row(r);
 r={};r[2]=27;r[14]=queues;r[15]=r[16]=queues;r[17]=queues+1;r[19]=300;r[21]=7;r[22]=3;row(r);return b;
}
int main(){try{
 for(unsigned n:{1u,16u,120u}){auto b=wire(n);auto stream=mnm::legacy::decodeCanvasProducers(b);require(stream.version==4&&stream.queues==n);mnm::legacy::CanvasProducerReplay replay([](const auto&)->Bytes{throw std::runtime_error("Unexpected asset");});for(const auto& c:stream.operations)replay.apply(c);auto image=replay.read(1);require(image.width==2&&image.height==2&&std::all_of(image.pixels.begin(),image.pixels.end(),[](auto p){return p==0x07e0;}));auto partial=b;partial.resize(partial.size()-96);mnm::legacy::decodeCanvasProducers(partial,false);refuses([&]{mnm::legacy::decodeCanvasProducers(partial);});}
 auto original=wire(1);for(auto change:std::vector<std::pair<unsigned,unsigned>>{{20,262144},{28,0},{32,134217728},{24,121},{unsigned(original.size()-96+68),8},{unsigned(original.size()-96+84),0},{unsigned(original.size()-96+88),1}}){auto b=original;put(b,change.first,change.second);refuses([&]{mnm::legacy::decodeCanvasProducers(b);});}
 auto b=original;b.insert(b.end(),original.begin()+64,original.begin()+160);refuses([&]{mnm::legacy::decodeCanvasProducers(b);});b=original;std::memcpy(b.data(),"MNMPRO03",8);put(b,8,3);refuses([&]{mnm::legacy::decodeCanvasProducers(b);});std::cout<<"Whole-update decoder/replay:3postlude cases,12refusals\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

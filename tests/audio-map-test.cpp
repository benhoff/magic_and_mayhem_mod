#include "attenuation_map.hpp"
#include "camera_projection.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
template<class F> void rejects(F f){bool rejected=false;try{f();}catch(const std::out_of_range&){rejected=true;}check(rejected,"invalid snapshot lookup must reject");}
struct Probe:r::PositionalByteSource {
    mutable unsigned calls=0;mutable int x=-1,y=-1;mutable unsigned z=0;
    std::optional<std::int8_t> read(int a,int b,std::uint32_t c) const override {
        ++calls;x=a;y=b;z=c;return -64;
    }
};
int main(){
    // Deliberately padded tables: no assumptions about contiguous row/layer strides.
    std::vector<std::uint8_t> bytes(300,0);bytes[15]=192;bytes[45]=129;
    std::vector<std::uint32_t> rows={10,20},layers={0,30};
    r::AttenuationMap map(8,rows,layers,bytes);
    bytes[15]=0;rows[0]=100;layers[0]=100;
    check(map.read(5,0,0)==-64 && map.read(5,0,1)==-64,"owned snapshot and paired Z layers");
    check(map.read(5,0,2)==-127 && map.read(5,0,3)==-127,"second layer signed load");
    auto copy=map;map.clear();check(copy.read(5,0,0)==-64,"snapshot copy survives source clear");
    rejects([&]{map.read(5,0,0);});map.enabled=false;
    check(!map.read(-1,-1,65535),"disabled map does not access tables");
    for(unsigned value=0;value<256;++value){
        r::AttenuationMap one(1,{0},{0},{std::uint8_t(value)});
        check(int(*one.read(0,0,0))==(value<128?int(value):int(value)-256),"all signed byte values");
    }
    rejects([&]{copy.read(-1,0,0);});rejects([&]{copy.read(8,0,0);});
    rejects([&]{copy.read(0,-1,0);});rejects([&]{copy.read(0,2,0);});
    rejects([&]{copy.read(0,0,4);});rejects([&]{copy.read(0,0,65535);});rejects([&]{copy.read(0,0,0xffffffffu);});
    r::AttenuationMap overflow(1,{std::numeric_limits<std::uint32_t>::max()},{1},{0});
    rejects([&]{overflow.read(0,0,0);});
    r::PositionalInput in;in.mapWidth=100;in.mapHeight=100;in.range=10;in.panWidth=10;
    in.listenerX=6;in.listenerY=6;in.sourceX=11;in.mapByte=0;
    Probe probe;
    check(r::positionalControls(in,probe,3).volume==-5000 && probe.calls==0,"out of range skips lookup");
    in.sourceX=10;
    check(r::positionalControls(in,probe,3).volume==-5000 && probe.calls==1,"exact range still reads map");
    in.sourceX=99;in.sourceY=0;
    auto audible=r::positionalControls(in,probe,3);
    check(probe.x==99 && probe.y==0 && probe.z==3,"lookup uses source coordinates, not wrapped deltas");
    in.mapByte=-64;auto explicitByte=r::positionalControls(in);
    check(audible.volume==explicitByte.volume && audible.pan==explicitByte.pan && audible.panWritten,"provider attenuation agrees with decoded byte");
    in.sourceX=5;in.mapByte.reset();
    auto actual=r::positionalControls(in,copy,0);in.mapByte=-64;
    check(actual.volume==r::positionalControls(in).volume,"snapshot feeds positional attenuation");
    copy.enabled=false;in.mapByte=-127;
    actual=r::positionalControls(in,copy,0);in.mapByte.reset();
    check(actual.volume==r::positionalControls(in).volume && actual.panWritten,"disabled source replaces stale caller byte");
    r::CameraState camera;camera.mapWidth=100;camera.mapHeight=100;
    const auto listener=r::audioListener(camera,false);
    in.listenerX=listener.x;in.listenerY=listener.y;
    in.sourceX=(listener.x+94)%100;in.sourceY=(listener.y+94)%100;
    copy.enabled=true;
    // Use a larger snapshot for the camera's normalized world coordinates.
    std::vector<std::uint32_t> cameraRows;for(unsigned y=0;y<100;++y)cameraRows.push_back(y*100);
    r::AttenuationMap cameraMap(100,cameraRows,{0},std::vector<std::uint8_t>(10000,192));
    actual=r::cameraPositionalControls(camera,false,in,cameraMap,1);in.mapByte=-64;
    auto expected=r::cameraPositionalControls(camera,false,in);
    check(actual.volume==expected.volume && actual.pan==expected.pan && actual.panWritten,"camera to map lookup to attenuation integration");
    std::cout<<"Audio map: signed bytes, padded tables, gates, ownership and camera integration passed\n";
}

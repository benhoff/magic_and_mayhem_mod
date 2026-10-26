#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "../tests/sprite-binary-reference.cpp"
#undef main
#endif
#include "../protocols/include/mnm/terrain_light_v1.h"
#include "../reconstruction/rendering/terrain_lighting_cycle.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
using namespace mnm::reconstruction;using U=std::uint32_t;using Bytes=std::vector<unsigned char>;
int main(int argc,char** argv)try{
 if(argc!=
#ifdef ORIGINAL_REFERENCE
  4
#else
  3
#endif
 )throw std::runtime_error("Expected input reply [original PE]");std::ifstream f(argv[1],std::ios::binary);Bytes b(std::istreambuf_iterator<char>(f),{});
 auto u=[&](std::size_t at){if(at+4>b.size())throw std::runtime_error("Input extent");U v;std::memcpy(&v,b.data()+at,4);return v;};
 if(b.size()<300 || b.size()>MNM_LIGHT_V1_MAX_BYTES || std::memcmp(b.data(),MNM_LIGHT_V1_MAGIC,8) || u(8)!=1 || u(12)!=128 || u(16)!=b.size() || u(112)>1 || u(116) || u(124) || u(120)!=mnm_light_checksum(b.data(),U(b.size())))throw std::runtime_error("Light input identity");
 const U w=u(32),h=u(36),layers=u(40),count=u(44),cc=u(76),cs=u(80),oc=u(84),os=u(88);
 if(w<1 || w>128 || h<1 || h>128 || layers<1 || layers>32 || count!=w*h*((layers+1)/2) || cc>MNM_LIGHT_V1_MAX_SOURCES || cs>MNM_LIGHT_V1_MAX_SOURCES || oc>MNM_LIGHT_V1_MAX_SOURCES || os>oc || u(68)>1 || u(72)>1 || b.size()!=300ull+cc*28ull+oc*32ull+count*5ull)throw std::runtime_error("Light input bounds");
 TerrainLightingCycle cycle(w,h,layers,{std::int32_t(u(48)),std::int32_t(u(52)),std::int32_t(u(56))});TerrainLightingSnapshot input;
 input.player=std::int32_t(u(60));input.changed=std::int32_t(u(64));input.creaturesEnabled=u(68);input.objectsEnabled=u(72);input.creatureScan=cs;input.objectScan=os;input.view={u(92),u(96),u(100)};input.spectator=TerrainSpectatorLight{u(92),u(96)};
 for(U a=0;a<8;++a)for(U z=0;z<8;++z){input.relations.first[a][z]=b[128+a*16+z];input.relations.second[a][z]=b[128+a*16+8+z];}
 std::size_t at=300;
 for(U i=0;i<cc;++i,at+=28){if(u(at)>1 || u(at+4)>1 || u(at+24))throw std::runtime_error("Creature flags/reserved");input.creatures.push_back({u(at)!=0,u(at+4)!=0,std::int32_t(u(at+8)),u(at+12),u(at+16),u(at+20)});}
 for(U i=0;i<oc;++i,at+=32){if(u(at)>1 || u(at+28))throw std::runtime_error("Object flags/reserved");input.objects.push_back({u(at)!=0,u(at+4),u(at+8),u(at+12),u(at+16),u(at+20),u(at+24)});}
 std::array<std::vector<std::int8_t>,5> buffers;
 for(auto& dst:buffers){dst.resize(count);std::memcpy(dst.data(),b.data()+at,count);at+=count;}
 if(u(272)>1 || u(292)>1 || u(296)>1)throw std::runtime_error("Publication flag domain");
 cycle.restore(buffers,{u(256),u(260),u(264),u(268),u(272)!=0},{u(276),u(280),u(284),u(288),u(292)!=0,u(296)!=0});cycle.step(input);
 Bytes reply(32+48+5*count,0);std::memcpy(reply.data(),MNM_LIGHT_V1_REPLY,8);
 auto put=[&](std::size_t o,U v){std::memcpy(reply.data()+o,&v,4);};put(8,1);put(12,U(reply.size()));put(16,u(20));put(20,u(24));put(24,u(28));put(28,u(120));
 const auto& c=cycle.creatures();const auto& o=cycle.objects();std::array<U,12> state{0,c.phase,c.remaining,c.quota,c.cursor,c.published,o.phase,o.remaining,o.quota,o.cursor,o.active,o.requested};std::memcpy(reply.data()+32,state.data(),48);at=80;
 for(const auto& src:cycle.field().buffers()){std::memcpy(reply.data()+at,src.data(),count);at+=count;}
#ifdef ORIGINAL_REFERENCE
 // Rebuild original kernels, then restore the captured producer inputs and
 // execute the unchanged original outer updater. The emitted result below is
 // read from original memory, independently of the native result above.
 map_image(read(argv[3]));constexpr U map=0x6c5490;Bytes object(0x66c0,0),creatures(cc*0xe4b,0),objects(oc*0x22e,0);
 std::memcpy(reinterpret_cast<void*>(map),object.data(),object.size());
 const auto set=[](U p,U value){std::memcpy(reinterpret_cast<void*>(p),&value,4);};
 set(map+4,w);set(map+8,h);set(map+12,layers);set(map+16,w*h);set(map+0x7e0,count);set(map+0x808,u(68));set(map+0x80c,u(72));
 for(U y=0;y<h;++y)set(map+0x64b2+y*4,y*w);for(U z=0;z<(layers+1)/2;++z)set(map+0x6432+z*4,z*w*h);
 set(0x5e18e8,u(48));set(0x5e18ec,u(52));
 using Call=void(__attribute__((thiscall))*)(void*);reinterpret_cast<Call>(0x4f1890)(reinterpret_cast<void*>(map));*reinterpret_cast<unsigned char*>(map+0x6380)=static_cast<unsigned char>(u(56));
 for(U i=0;i<5;++i)set(map+0x7cc+i*4,U(reinterpret_cast<std::uintptr_t>(buffers[i].data())));
 set(0x644520,u(60));set(0x5e41a8,u(64));set(0x5fbf28,0x643a80);
 for(U a=0;a<8;++a)for(U j=0;j<16;++j)*reinterpret_cast<unsigned char*>(0x643a80+a*0x154+0x18+j)=b[128+a*16+j];
 const U stateAddresses[11]={0x6e8d90,0x6e8d94,0x6e8d98,0x6e8d9c,map+0x642e,0x6e8da0,0x6e8da4,0x6e8da8,0x6e8dac,map+0x642a,map+0x6426};
 for(U i=0;i<11;++i)set(stateAddresses[i],u(256+i*4));
 set(0x6def58,U(reinterpret_cast<std::uintptr_t>(creatures.data())));set(0x6def5c,cc);set(0x6df180,cs);set(0x689498,U(reinterpret_cast<std::uintptr_t>(objects.data())));set(0x68949c,oc);set(0x6898dc,os);
 for(U i=0;i<34;++i)set(0x6894a4+i*12,i);
 for(U i=0;i<cc;++i){U p=U(reinterpret_cast<std::uintptr_t>(creatures.data()))+i*0xe4b;const auto& c=input.creatures[i];set(p+4,c.active);set(p+0xe4,c.lightEnabled);set(p+0x174,U(c.owner));set(p+8,c.column);set(p+12,c.row);set(p+16,c.layer);}
 for(U i=0;i<oc;++i){U p=U(reinterpret_cast<std::uintptr_t>(objects.data()))+i*0x22e;const auto& o=input.objects[i];set(p+4,o.active);set(p+0x28,o.lightIndex);set(p+8,o.column);set(p+12,o.row);set(p+16,o.layer);set(p+0x1c2,o.admissionColumn);set(p+0x1c6,o.admissionRow);}
 set(0x689930+0x39,u(92));set(0x689930+0x3d,u(96));set(0x689930+0x59,u(100));
 reinterpret_cast<Call>(0x4f27a0)(reinterpret_cast<void*>(map));
 const auto get=[](U p){U value;std::memcpy(&value,reinterpret_cast<void*>(p),4);return value;};put(32,get(0x5e41a8));for(U i=0;i<11;++i)put(36+i*4,get(stateAddresses[i]));at=80;
 for(const auto& src:buffers){std::memcpy(reply.data()+at,src.data(),count);at+=count;}
#endif
 std::ofstream out(argv[2],std::ios::binary);out.write(reinterpret_cast<const char*>(reply.data()),reply.size());if(!out)throw std::runtime_error("Reply write");
 std::cout<<"{\"success\":true,\"sequence\":"<<u(20)<<",\"creature_capacity\":"<<cc<<",\"object_capacity\":"<<oc<<",\"buffer_bytes\":"<<5*count<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

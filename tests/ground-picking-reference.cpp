// Independent original/native processes; no game, Win32 imports or PE file patch.
#include "ground_picking.hpp"
#include "../assets/sprite_loader.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <stdexcept>
#include <sys/mman.h>
using namespace mnm::reconstruction;
using Bytes=std::vector<std::uint8_t>;
using U=std::uint32_t;using I=std::int32_t;
static void require(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static Bytes read(const char* p){std::ifstream f(p,std::ios::binary);require(bool(f),"Input open");return Bytes(std::istreambuf_iterator<char>(f),{});}
static U get(const void* p){U v;std::memcpy(&v,p,4);return v;}
static U word(const Bytes& b,std::size_t p){require(p<=b.size()&&b.size()-p>=4,"Input extent");return get(b.data()+p);}
static void put(void* p,std::size_t n,U v){std::memcpy(static_cast<std::uint8_t*>(p)+n,&v,4);}
static void half(void* p,std::size_t n,std::uint16_t v){std::memcpy(static_cast<std::uint8_t*>(p)+n,&v,2);}
static void global(U p,U v){put(reinterpret_cast<void*>(p),0,v);}
static U at(U p){return get(reinterpret_cast<void*>(p));}
static U address(const void* p){return U(reinterpret_cast<std::uintptr_t>(p));}
static I bits(U v){I n;std::memcpy(&n,&v,4);return n;}
static std::size_t mapImage(const Bytes& b,bool original){
    const auto pe=word(b,60),opt=pe+24,length=word(b,opt+56);
    require(word(b,opt+28)==0x400000&&length<32*1024*1024,"PE image");
    void* p=mmap(reinterpret_cast<void*>(0x400000),length,PROT_READ|PROT_WRITE,
                 MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    require(p!=MAP_FAILED,"Private PE mapping");
    const auto sections=std::uint16_t(b.at(pe+6))|(std::uint16_t(b.at(pe+7))<<8);
    const auto size=std::uint16_t(b.at(pe+20))|(std::uint16_t(b.at(pe+21))<<8);
    for(unsigned k=0;k<unsigned(sections);++k){const auto s=opt+size+k*40,rva=word(b,s+12),n=word(b,s+16),raw=word(b,s+20);
        require(std::uint64_t(raw)+n<=b.size()&&std::uint64_t(rva)+n<=length,"PE section");
        std::memcpy(static_cast<std::uint8_t*>(p)+rva,b.data()+raw,n);
    }
    const std::uint8_t leaf[]={0x8b,0x41,0x10,0x53,0x56};
    const std::uint8_t cell[]={0x55,0x8b,0xec,0x83,0xe4,0xf8};
    require(!std::memcmp(reinterpret_cast<void*>(0x57dcf0),leaf,sizeof(leaf)),"Point leaf guard");
    require(!std::memcmp(reinterpret_cast<void*>(0x4f8960),cell,sizeof(cell)),"Terrain leaf guard");
    if(original){
        for(unsigned k=0;k<unsigned(sections);++k){const auto s=opt+size+k*40;
            if(word(b,s+36)&0x20000000u){const U rva=word(b,s+12),n=std::max(word(b,s+8),word(b,s+16));
                require(!mprotect(static_cast<std::uint8_t*>(p)+(rva&~4095u),((rva&4095u)+n+4095u)&~4095u,PROT_READ|PROT_EXEC),"Original code RX");
            }
        }
    }else require(!mprotect(p,length,PROT_NONE),"Native PE poison");
    return length;
}
extern "C" void rz_classify_block(void*,void*);
struct Trace {
    std::ofstream file;
    explicit Trace(const char* p):file(p,std::ios::binary){require(bool(file),"Trace output");file.write("MNMRZ001",8);}
    void u(U v){file.write(reinterpret_cast<const char*>(&v),4);require(bool(file),"Trace write");}
    void flag(bool b){file.put(char(b));require(bool(file),"Trace byte");}
};
static std::uint64_t pointCases=0,pointHits=0;
static U worlds=0,misses=0,groundHits=0,refusals=0,classCases=0;
static U aliasZ=0,nonMaximumXYZ=0,hiddenChildHits=0;
static std::array<U,4> orientations{};
static std::array<U,32> zHits{};
struct Art {
    Bytes bytes;mnm::assets::Sprite decoded;std::vector<PickingMask> masks;
    U count,table,base;
    explicit Art(Bytes b):bytes(std::move(b)){
        auto d=mnm::assets::decodeSprite(bytes);
        if(auto* error=std::get_if<mnm::assets::SpriteError>(&d))throw std::runtime_error("SPR decode: "+error->detail);
        decoded=std::get<mnm::assets::Sprite>(std::move(d));
        require(decoded.version==4,"SPR v4 required");
        count=word(bytes,12);table=24+768*word(bytes,16);base=table+4*count;
        for(const auto& f:decoded.frames)masks.emplace_back(f.width,f.height,f.originX,f.originY,f.opaqueMask);
    }
    void* frame(U n){if(n>=count)n=0;return bytes.data()+base+word(bytes,table+4*n);}
};
static Bytes synthetic(){
    const std::vector<std::array<U,2>> sizes{{0,0},{96,96},{96,96},{96,96},{511,3},{5,3},{511,3}};
    Bytes b(24+4*sizes.size());put(b.data(),0,0x00525053);put(b.data(),8,4);put(b.data(),12,sizes.size());
    const U base=U(b.size());
    for(U id=0;id<sizes.size();++id){
        const auto [w,h]=sizes[id];Bytes f(40+8*h);put(f.data(),4,w);put(f.data(),8,h);
        put(f.data(),12,id<4?48:U(-7));put(f.data(),16,id<4?48:1);put(f.data(),28,~0u);
        std::vector<Bytes> rows;std::vector<U> colours;
        for(U y=0;y<h;++y){
            Bytes runs;U x=0,ones=0;bool on=false;
            const auto opaque=[&](U k){return id==1 || id==6 || (id==2&&((k+y)%3==0)) || (id==4&&k>=255&&k<510) || (id==5&&((k^y)&1));};
            while(x<w){U n=0;while(x+n<w && opaque(x+n)==on)++n;
                U left=n;while(left>255){runs.push_back(255);runs.push_back(0);left-=255;}
                runs.push_back(std::uint8_t(left));if(on)ones+=n;x+=n;on=!on;
            }
            rows.push_back(runs);colours.push_back(ones);
        }
        for(U y=0;y<h;++y){put(f.data(),40+8*y,f.size());f.insert(f.end(),rows[y].begin(),rows[y].end());}
        for(U y=0;y<h;++y){put(f.data(),44+8*y,f.size());f.resize(f.size()+2*colours[y]);} // opaque word zero.
        put(f.data(),0,f.size());put(b.data(),24+4*id,b.size()-base);b.insert(b.end(),f.begin(),f.end());
    }
    put(b.data(),4,b.size());return b;
}
static void points(Art& art,bool original,Trace& out,bool exhaustive){
    using Hit=U(__attribute__((fastcall)) *)(void*,I,I,const I*);
    const auto call=reinterpret_cast<Hit>(0x57dcf0);
    for(U id=0;id<art.count;++id){const auto& f=art.decoded.frames[id];
        const std::array<std::array<I,2>,5> anchors{{{320,240},{-100,-80},{std::numeric_limits<I>::max(),0},{std::numeric_limits<I>::min(),-1},{0,std::numeric_limits<I>::max()}}};
        for(U a=0;a<anchors.size();++a){const auto ax=anchors[a][0],ay=anchors[a][1];
            const auto test=[&](I px,I py){
                const I point[2]={px,py};const bool hit=original?call(art.frame(id),ax,ay,point)!=0:art.masks[id].hit(ax,ay,px,py);
                out.flag(hit);++pointCases;pointHits+=hit;
            };
            const auto left=U(ax)-U(f.originX),top=U(ay)-U(f.originY);
            if(exhaustive && a==0){
                for(I y=-1;y<=I(f.height);++y)for(I x=-1;x<=I(f.width);++x)test(bits(left+U(x)),bits(top+U(y)));
            }else{
                for(I y:{-1,0,I(f.height/2),I(f.height)-1,I(f.height)})
                    for(I x:{-1,0,I(f.width/2),I(f.width)-1,I(f.width)})test(bits(left+U(x)),bits(top+U(y)));
            }
        }
    }
}
struct Fixture {
    U width=8,height=9,layers=3,player=1;bool force=false;
    TerrainCamera camera;TerrainViewport viewport{0,0,640,480};
    std::vector<GroundTile> cells;
    std::optional<std::array<I,2>> point;
};
static void ground(Fixture f,const Bytes& definitionBytes,const std::vector<TerrainDefinition>& definitions,
                   Art& art,bool original,Trace& out,U id){
    std::vector<std::array<std::uint16_t,2>> initial;
    for(const auto& t:f.cells)initial.push_back({t.flags8,t.flags10});
    std::vector<TerrainVisit> visits;
    try{visits=traverseOwnedTerrain(f.width,f.height,f.layers,f.camera,f.viewport,initial);}
    catch(const std::out_of_range&){++refusals;out.u(0x534b4950);out.u(id);return;}
    I px=-65536,py=-65536;
    if(!visits.empty()){
        const TerrainVisit* selected=&visits[(std::uint64_t(id)*137)%visits.size()];
        if(id>=100000)for(const auto& v:visits)if(f.cells[v.cell].definition){selected=&v;break;}
        const auto& v=*selected;px=v.anchorX;py=v.anchorY;
        if(id%4==0){px+=7;py-=3;}if(id%11==0){px=-65536;py=-65536;}
        // Pick the first source-derived opaque position for the installed/small single-cell cases.
        if(id>=100000){const auto& d=definitions[f.cells[v.cell].definition];
            const U role=(id-100000)%3,n=role==0?d.body[f.camera.view]:role==1?d.first[f.camera.view]:d.second[f.camera.view];
            const auto& frame=art.decoded.frames[n<art.count?n:0];
            const auto it=std::find(frame.opaqueMask.begin(),frame.opaqueMask.end(),std::uint8_t(1));
            if(it!=frame.opaqueMask.end()){
                const auto index=std::size_t(it-frame.opaqueMask.begin());
                px=bits(U(v.anchorX)-U(frame.originX)+U(index%frame.width));
                py=bits(U(v.anchorY)-U(frame.originY)+U(index/frame.width));
            }
        }
    }
    if(f.point){px=(*f.point)[0];py=(*f.point)[1];}
    std::optional<PickedGround> picked;I depth;U row=0,count=0;std::vector<std::array<std::uint16_t,2>> flags;
    U maxPhysical=~0u;
    if(!original){
        const auto r=pickWorldGround(f.width,f.height,f.layers,f.camera,f.viewport,f.player,f.force,f.cells,definitions,art.masks,px,py);
        picked=r.ground;depth=r.terrainDepth;row=r.depthRow;count=r.submittedRows;flags=r.flags;
    }else{
        Bytes cells(f.cells.size()*12),map(0x6800),light(f.width*f.height+1);
        for(U k=0;k<f.cells.size();++k){put(cells.data(),12*k+2,~0u);half(cells.data(),12*k+6,0xffff);half(cells.data(),12*k,f.cells[k].definition);half(cells.data(),12*k+8,f.cells[k].flags8);half(cells.data(),12*k+10,f.cells[k].flags10);}
        put(map.data(),4,f.width);put(map.data(),8,f.height);put(map.data(),12,f.layers);put(map.data(),16,f.width*f.height);put(map.data(),0x4c,address(cells.data()));put(map.data(),0x7f0,f.camera.specialLayer);
        for(U z=0;z<f.layers;++z)put(map.data(),0x6432+4*z,z*f.width*f.height);
        for(U y=0;y<f.height;++y)put(map.data(),0x64b2+4*y,y*f.width);
        std::array<U,4> header{{0,0,0,art.count-1}}; // Explicit safe last-frame bound.
        const std::array<U,4> bag{{address(header.data()),0,address(art.bytes.data()+art.table),address(art.bytes.data()+art.base)}};
        std::array<std::uint8_t,0x260> scene{};
        put(scene.data(),0x21,address(map.data()));put(scene.data(),0x25,address(map.data()));put(scene.data(),0x2d,address(bag.data()));
        const auto& c=f.camera;
        for(const auto& v:std::vector<std::pair<U,U>>{{0x31,c.view},{0x23d,c.view},{0x11,U(c.f11)},{0x15,U(c.f15)},{0x39,U(c.column)},{0x3d,U(c.row)},{0x41,U(c.f41)},{0x45,U(c.f45)},{0x49,U(c.f49)},{0x4d,U(c.f4d)},{0x51,U(c.f51)},{0x55,U(c.f55)},{0x59,c.span},{0x5d,c.diagonal},{0x61,c.cutLevel},{0x65,f.player},{0x6d,c.mode},{0x91,U(px)},{0x95,U(py)},{0xa1,U(c.wrapColumnPriority)},{0xa5,U(c.wrapRowPriority)},{0x1ed,U(-I(f.width*64))}})put(scene.data(),v.first,v.second);
        std::vector<std::array<U,9>> queue(3*visits.size()+16);
        put(scene.data(),0x229,address(queue.data()));put(scene.data(),0x22d,address(queue.data()));put(scene.data(),0x235,queue.size());
        global(0x65660c,address(definitionBytes.data()));global(0x6c5c5c,address(light.data()));global(0x5e41ac,1);global(0x5e41b0,f.force);global(0x6a49cc,0);
        for(U y=0;y<f.height;++y)global(0x6cb942+4*y,y*f.width);
        for(U z=0;z<32;++z)global(0x6cb8c2+4*z,0);
        for(U z=0;z<500;++z)global(0x6e818c+4*z,82*z/100);
        for(U p:{0x6c5488u,0x6c5484u,0x6cbb64u})global(p,~0u);
        global(0x6c3750,0);global(0x6e8db0,0);
        std::array<I,4> viewport{{f.viewport.left,f.viewport.top,f.viewport.right,f.viewport.bottom}};
        using Run=void(__attribute__((thiscall)) *)(void*,void*);
        const std::array<U,4> entries{{0x4f83f0,0x4fbe00,0x4fc3f0,0x4fc930}};
        reinterpret_cast<Run>(entries[c.view])(scene.data(),viewport.data());
        if(at(0x6c5488)!=~0u)picked=PickedGround{at(0x6c5488),at(0x6c5484),at(0x6cbb64)};
        depth=bits(get(scene.data()+0x1ed));row=at(0x6c3750);count=get(scene.data()+0x231);
        if(count&&row<count&&depth> -I(f.width*64))maxPhysical=(queue[row][5]-address(cells.data()))/12;
        for(U k=0;k<f.cells.size();++k)flags.push_back({std::uint16_t(cells[k*12+8]|U(cells[k*12+9])<<8),std::uint16_t(cells[k*12+10]|U(cells[k*12+11])<<8)});
    }
    out.u(0x47524e44);out.u(id);out.u(picked?picked->column:~0u);out.u(picked?picked->row:~0u);out.u(picked?picked->layer:~0u);out.u(U(depth));out.u(row);out.u(count);
    U changed=0;for(U k=0;k<flags.size();++k)if(flags[k]!=initial[k])++changed;
    out.u(changed);for(U k=0;k<flags.size();++k)if(flags[k]!=initial[k]){out.u(k);out.u(flags[k][0]);out.u(flags[k][1]);}
    if(id==99999)require(picked&&picked->column==4&&picked->row==4&&picked->layer==0,"Physical/annotated Z carry");
    ++worlds;++orientations[f.camera.view];
    if(picked){++groundHits;require(picked->layer<32,"Ground layer domain");++zHits[picked->layer];
        if(original&&maxPhysical<f.cells.size()&&maxPhysical!=(picked->layer*f.height+picked->row)*f.width+picked->column)++nonMaximumXYZ;
        for(const auto& v:visits)if(v.column==picked->column&&v.row==picked->row&&v.layer==picked->layer&&v.cell/(f.width*f.height)!=v.layer){++aliasZ;break;}
        for(const auto& v:visits)if(v.column==picked->column&&v.row==picked->row&&v.layer==picked->layer&&(initial[v.cell][0]&0x8004)) {++hiddenChildHits;break;}
    }else ++misses;
}
static void worldsFor(Art& art,const std::vector<TerrainDefinition>& definitions,const Bytes& rawDefinitions,bool original,Trace& out,bool installed){
    std::mt19937 rng(0x525a3039);
    if(!installed){
        for(U n=0;n<2048;++n){Fixture f;f.width=8+rng()%13;f.height=8+rng()%13;f.layers=1+rng()%12;f.player=rng()%4;f.force=rng()%2;
            auto& c=f.camera;c.view=n%4;c.span=1+rng()%std::min({f.width,f.height,12u});c.diagonal=rng()%std::min(f.width,f.height);c.column=rng()%f.width;c.row=rng()%f.height;c.cutLevel=1+rng()%f.layers;c.mode=rng()%2;c.specialLayer=rng()%f.layers;
            c.f11=160+I(rng()%120);c.f15=80+I(rng()%160);c.f41=2*c.diagonal+I(rng()%12);c.f45=rng()%32;c.f49=rng()%32;c.f4d=rng()%16;c.f51=I(rng()%5)-2;c.f55=I(rng()%5)-2;c.wrapColumnPriority=I(rng()%4097)-2048;c.wrapRowPriority=I(rng()%4097)-2048;
            f.viewport=n%2?TerrainViewport{0,0,800,600}:TerrainViewport{0,0,640,480};
            for(U k=0;k<f.width*f.height*f.layers;++k){const U flags[]={0,4,0x8000,0x8004,0x80,8u<<c.view,3};const U other[]={0,8,0x40,0x4000,0x200u<<((f.player-1)&31)};f.cells.push_back({std::uint16_t(rng()%definitions.size()),std::uint16_t(flags[rng()%7]),std::uint16_t(other[rng()%5])});}
            ground(f,rawDefinitions,definitions,art,original,out,n);
        }
    }
    // Every declared terrain definition in every view; short cells isolate Z.
    for(U id=1;id<definitions.size();++id)for(U view=0;view<4;++view)for(U role=0;role<3;++role){
        Fixture f;auto& c=f.camera;c.view=view;c.span=1;c.diagonal=0;c.column=2;c.row=3;c.cutLevel=3;c.f11=320;c.f15=240;c.f41=0;c.specialLayer=~0u;
        f.cells.resize(f.width*f.height*f.layers);const U z=(id+view+role)%3;f.cells[(z*f.height+3)*f.width+2]={std::uint16_t(id),std::uint16_t(role?0x8004:0),0};
        const U caseId=100000+12*(id-1)+view*3+role;
        ground(f,rawDefinitions,definitions,art,original,out,caseId);
    }
}
int main(int argc,char** argv)try{
    require(argc==6,"Expected original/native, PE, SPR, TTD and trace");const bool original=std::string(argv[1])=="original";require(original||std::string(argv[1])=="native","Mode");
    mapImage(read(argv[2]),original);Trace out(argv[5]);
    Art syntheticArt(synthetic());points(syntheticArt,original,out,true);
    Art installed(read(argv[3]));points(installed,original,out,true);
    std::vector<TerrainDefinition> defs(9);Bytes defBytes(defs.size()*356);
    for(U id=1;id<defs.size();++id)for(U view=0;view<4;++view){defs[id].body[view]=id==8?99:id%6;defs[id].first[view]=(id+1)%6;defs[id].second[view]=(id+2)%6;put(defBytes.data(),id*356+0x84+4*view,defs[id].body[view]);put(defBytes.data(),id*356+0xd0+4*view,defs[id].first[view]);put(defBytes.data(),id*356+0xe0+4*view,defs[id].second[view]);}
    worldsFor(syntheticArt,defs,defBytes,original,out,false);
    {Fixture f;f.width=4;f.height=5;f.layers=3;auto& c=f.camera;c.view=2;c.column=c.row=0;c.span=3;c.diagonal=0;c.f41=0;c.f11=100;c.f15=64;c.cutLevel=1;
     f.cells.resize(60);f.cells[20]={1,0x8004,0};
     const auto visits=traverseTerrain(4,5,3,c,{},[](auto,auto,auto){return std::array<std::uint16_t,2>{0,0};});
     f.point=std::array<I,2>{visits[3].anchorX,visits[3].anchorY};
     auto aliasDefs=defs;aliasDefs[1].body.fill(1);aliasDefs[1].first.fill(0);aliasDefs[1].second.fill(0);auto aliasRaw=defBytes;
     for(U v=0;v<4;++v){put(aliasRaw.data(),356+0x84+4*v,1);put(aliasRaw.data(),356+0xd0+4*v,0);put(aliasRaw.data(),356+0xe0+4*v,0);}
     ground(f,aliasRaw,aliasDefs,syntheticArt,original,out,99999);
    }
    const auto ttd=read(argv[4]);const auto definitions=word(ttd,12);require(ttd.size()==16+std::uint64_t(definitions)*356,"TTD extent");
    defs.assign(definitions+1,{});defBytes.assign((definitions+1)*356,0);
    std::copy(ttd.begin()+16,ttd.end(),defBytes.begin()+356);
    for(U id=1;id<defs.size();++id){std::array<std::uint8_t,356> d{};std::copy_n(defBytes.data()+id*356,356,d.begin());defs[id]=decodeTerrainDefinition(d);}
    worldsFor(installed,defs,defBytes,original,out,true);
    void* block=nullptr;std::array<std::uint8_t,0x260> scene{};
    if(original){block=mmap(nullptr,4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);require(block!=MAP_FAILED,"Block mapping");std::memcpy(block,reinterpret_cast<void*>(0x4fd160),0x60);static_cast<std::uint8_t*>(block)[0x60]=0xc3;require(!mprotect(block,4096,PROT_READ|PROT_EXEC),"Block RX");}
    const std::array<I,5> ds{{std::numeric_limits<I>::min(),-1,0,1,std::numeric_limits<I>::max()}};
    for(I a:ds)for(I b:ds)for(I t:ds)for(U present=0;present<8;++present){
        U value;
        if(original){put(scene.data(),0x1e9,U(a));put(scene.data(),0x1f1,U(b));put(scene.data(),0x1ed,U(t));global(0x656600,present&1);global(0x6dbea4,present&2);global(0x6c5488,present&4?0:~0u);rz_classify_block(block,scene.data());value=at(0x6a5134);}
        else value=U(classifyWorldHit({a,b,t,bool(present&1),bool(present&2),bool(present&4)}));
        out.u(value);++classCases;
    }
    out.file.flush();require(bool(out.file),"Trace close");
    std::cout<<"{\"success\":true,\"point_cases\":"<<pointCases<<",\"point_hits\":"<<pointHits<<",\"world_cases\":"<<worlds<<",\"ground_hits\":"<<groundHits<<",\"ground_misses\":"<<misses<<",\"native_domain_refusals\":"<<refusals<<",\"class_cases\":"<<classCases<<",\"physical_layer_annotation_cases\":"<<aliasZ<<",\"hits_with_suppressed_draw_roles\":"<<hiddenChildHits<<",\"xyz_max_depth_disagree_cases\":"<<nonMaximumXYZ<<",\"installed_frames\":"<<installed.count<<",\"installed_definitions\":"<<definitions<<",\"native_pe_unreadable\":"<<(original?"false":"true")<<",\"orientations\":[";
    for(U v=0;v<4;++v){if(v)std::cout<<',';std::cout<<orientations[v];}std::cout<<"],\"z_hits\":[";
    for(U z=0;z<32;++z){if(z)std::cout<<',';std::cout<<zHits[z];}std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

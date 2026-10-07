#pragma once
#include "resource_manager.hpp"
#include <QBuffer>
#include <QImage>
#include <QTemporaryDir>
#include <fstream>
#include <stdexcept>

namespace resource_test {
using Bytes=std::vector<std::uint8_t>;
inline void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F> void rejected(F action){
    bool failed=false;try{action();}catch(const std::exception&){failed=true;}
    require(failed,"Invalid resource operation accepted");
}
inline void word(Bytes& b,std::size_t at,std::uint32_t value){for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(value>>(8*i));}
inline Bytes sprite(bool indexed=false,std::uint16_t colour=0xf800){
    const std::size_t frameBytes=indexed?53:55,base=36+(indexed?768:0);
    Bytes b(base+frameBytes*2+40,0);
    word(b,0,0x00525053);word(b,4,b.size());word(b,8,4);word(b,12,3);word(b,16,indexed?1:0);
    if(indexed){b[24+3]=255;b[24+7]=255;}
    for(unsigned i=0;i<3;++i){
        const auto at=base+(i<2?frameBytes*i:frameBytes*2);
        word(b,base-12+i*4,at-base);word(b,at,i==2?40:frameBytes);
        word(b,at+28,indexed?0:0xffffffff);
        if(i==2)continue;
        word(b,at+4,3);word(b,at+8,1);word(b,at+12,1);word(b,at+16,0xffffffff);
        word(b,at+40,48);word(b,at+44,51);b[at+48]=0;b[at+49]=2;b[at+50]=1;
        if(indexed){b[at+51]=0;b[at+52]=std::uint8_t(i+1);}
        else{b[at+53]=std::uint8_t(colour);b[at+54]=std::uint8_t(colour>>8);}
    }
    return b;
}
inline Bytes animation(std::uint32_t frame=0){
    Bytes b(52+88,0);word(b,0,0x00494e41);word(b,4,b.size());word(b,8,2);word(b,12,5);word(b,20,2);
    word(b,44,0);word(b,48,2);word(b,52,0);word(b,56,frame);word(b,96,6);return b;
}
inline Bytes terrain(){Bytes b(16+356,0);word(b,0,0x00445454);word(b,4,b.size());word(b,8,4);word(b,12,1);b[20]=17;return b;}
inline Bytes bmp(){
    Bytes b(62,0);b[0]='B';b[1]='M';word(b,2,b.size());word(b,10,54);word(b,14,40);
    word(b,18,2);word(b,22,0xffffffff);b[26]=1;b[28]=24;word(b,34,8);b[58]=255;return b;
}
inline Bytes pcx(){
    Bytes b(128,0);b[0]=10;b[1]=5;b[2]=1;b[3]=8;b[8]=2;b[65]=1;b[66]=4;
    b.insert(b.end(),{0,1,2,0,12});b.resize(b.size()+768,0);b[b.size()-768+3]=255;b[b.size()-768+7]=255;return b;
}
inline void write(const std::filesystem::path& root,const std::string& name,const Bytes& bytes){
    std::ofstream file(root/name,std::ios::binary|std::ios::trunc);
    file.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));
    require(bool(file),"Cannot write resource fixture");
}
inline mnm::assets::AssetStore store(const std::filesystem::path& root){
    auto result=mnm::assets::AssetStore::create(root);
    if(const auto* e=std::get_if<mnm::assets::Error>(&result))throw std::runtime_error(e->detail);
    return std::get<mnm::assets::AssetStore>(std::move(result));
}
inline void populate(const std::filesystem::path& root){
    write(root,"body.spr",sprite());write(root,"indexed.spr",sprite(true));write(root,"body.ani",animation());
    write(root,"tiles.ttd",terrain());write(root,"ui.bmp",bmp());write(root,"ui.pcx",pcx());
    QImage image(2,1,QImage::Format_RGB888);image.fill(Qt::black);QByteArray bytes;QBuffer buffer(&bytes);
    require(buffer.open(QIODevice::WriteOnly) && image.save(&buffer,"JPEG"),"JPEG fixture codec unavailable");
    write(root,"ui.jpg",Bytes(bytes.begin(),bytes.end()));
}
inline mnm::assets::ResourceRecipe spr(std::string path="body.spr"){
    return {mnm::assets::ResourceImageFormat::sprite,std::move(path),{},{},{}};
}
}

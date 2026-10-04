#pragma once
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <stdexcept>
namespace menu_sprite_fixture {
inline QByteArray sprites(int count,bool indexed=false) {
    const int paletteBytes=indexed?768:0,base=24+paletteBytes+count*4;
    // 6x4 frames: row tables, skip=1/run=5, and five opaque samples per row.
    const int extent=40+32+12+20*(indexed?1:2);QByteArray bytes(base+count*extent,0);
    auto put=[&](int pos,unsigned value){for(int i=0;i<4;++i)bytes[pos+i]=char(value>>(8*i));};
    put(0,0x00525053);put(4,bytes.size());put(8,4);put(12,count);put(16,indexed?1:0);
    if(indexed){bytes[27]=char(220);bytes[28]=char(100);bytes[29]=char(30);}
    for(int n=0;n<count;++n){
        put(24+paletteBytes+n*4,n*extent);const int at=base+n*extent;
        put(at,extent);put(at+4,6);put(at+8,4);put(at+12,unsigned(-1));put(at+16,unsigned(-2));put(at+28,indexed?0:0xffffffffU);
        for(int y=0;y<4;++y){put(at+40+y*8,72+y*3);put(at+44+y*8,84+y*5*(indexed?1:2));bytes[at+72+y*3]=1;bytes[at+73+y*3]=5;bytes[at+74+y*3]=0;
            for(int x=0;x<5;++x){const unsigned word=x==0?0:n%3==0?0xf800:n%3==1?0x07e0:0x001f;const int p=at+84+(y*5+x)*(indexed?1:2);bytes[p]=char(indexed?(x==0?0:1):word&255);if(!indexed)bytes[p+1]=char(word>>8);}
        }
    }
    return bytes;
}
inline void write(const QString& root,const QString& path,int count) {
    const auto full=root+"/"+path;QDir dir;const int slash=full.lastIndexOf('/');
    if(!dir.mkpath(full.left(slash)))throw std::runtime_error("sprite fixture folder");
    QFile file(full);const auto data=sprites(count);if(!file.open(QIODevice::WriteOnly)||file.write(data)!=data.size())throw std::runtime_error("sprite fixture write");
}
inline void shared(const QString& root) {
    write(root,"Sprites/Buttons.spr",21);
    write(root,"Interface/MultiplayerBattleSetup/MultiPBattle Screen buttons 800-600.spr",63);
}
}

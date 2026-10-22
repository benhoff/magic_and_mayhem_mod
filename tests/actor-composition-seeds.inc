// Callers own these inputs; original outputs are never used to initialize native.
for(U i=0;i<250;++i){w(0x5eb940+i*4,(i+1)%250);w(0x6574d0+i*4,seed*12345u+i*712397u+17u);}w(0x6574c8,seed%250);w(0x6574cc,(seed+103)%250);
w(0x5eb480,128);w(0x5eb484,96);w(0x5eb488,32);
w(0x5c73f0,0x545e60);w(0x5c7404,0x54aa80);
w(0x6cbb8a+0x1c00,0);w(0x6cbb8a+0x1c04,0);
w(0x5f2094,Other);w(0x5f1ec8,0);memory.set8(0x5f20e8,0);
w(Base+0x10008,0x1001f200);memory.set16(Base+0x1000e,0xcad0);
if(entry==0x50ab30||entry==0x508b00){
    U effects=p+0xbf3;w(effects+0xfc,seed%6);w(effects+0x80,999);
    for(U i=0;i<6;++i)w(effects+0x84+i*4,i%3);
}
if(entry==0x508b00||entry==0x50ab30||entry==0x50db60){w(p+0x64c,seed%4);w(p+0x6a8,(seed/4)%4);}
if(entry==0x5137d0){
    w(p+8,2);w(p+12,2);w(p+16,1);
    for(U i=0;i<120;++i)memory.set16(Base+0x16000+i*12+6,0xffff);
}
if(entry==0x4d9ac0){w(Array,0x1001f1c0);}
if(entry==0x5121c0||entry==0x512340){
    for(U i=0;i<4;++i){U secondary=Other+i*0x300;w(secondary,i);w(secondary+4,1);w(secondary+8,2);w(secondary+12,2);w(secondary+16,1);w(secondary+40,1);w(secondary+0x19c,0);w(secondary+0x5c,0);}
}
if(entry==0x464cb0||entry==0x50db60){
    for(U i=0;i<400;++i)w(Array+i*4,(i%8)*8);
    for(U i=0;i<64;++i){w(Buffer+i*44,6);w(Buffer+i*44+4,0);}
    for(U i=0;i<8;++i){U at=Buffer+i*352;w(at,1);w(at+4,seed%3);w(at+44,0);w(at+88,5);w(at+92,2);w(at+132,4);w(at+136,U(-2));w(at+176,6);}
    for(U offset:{0u,0xb0u,0x7d5u}){w(p+offset+24,Buffer);w(p+offset+28,Other+0x600);}
    if(entry==0x464cb0)args={seed%8,(seed/8)&1};
}
if(entry==0x4d4970){
    w(Array,0x1001f1c0);for(U off:{16u,20u,24u,28u,36u})w(Array+off,0x1001f140);
    w(0x6b1250,Base+0x6000);w(0x6b1254,128);for(U i=0;i<128;++i)w(Base+0x6000+i*32+12,5);
    if(U actor=r(p+0x18ec)){
        w(actor+0x178,3);
        for(U i=0;i<3;++i){U row=actor+0x17c+i*24;w(row,5+i);w(row+4,0);w(row+16,3);w(row+20,(seed+i)&1);}
    }
}
if(entry==0x564e90){
    constexpr U Records=Base+0x7800;w(p,Records);w(p+8,4);
    for(U i=0;i<4;++i){U row=Records+i*0x47c;for(U k=0;k<0x47c;++k)memory.set8(row+k,0);w(row,1);w(row+4,(seed+i)%10);w(row+8,0);w(row+12,(seed+i)%3-1);w(row+0x138,seed&1?U(-1):5);w(row+0x13c,seed%8);w(row+0x1a8,seed&2);w(row+0x1f4,seed&4);w(row+0x27c,(seed+i)&1);w(row+0x2cc,i);w(row+0x2d0,i+4);}
    w(0x6c4830,seed*5);memory.set8(0x5fc874,0);
}
if(entry==0x506290||entry==0x506d30||entry==0x51f5e0){
    p=Pool;for(U i=0;i<4*0xe4b;++i)memory.set8(Pool+i,0);
    U type=seed&16?0:17,definition=0x6a5f80+type*0x5c9;
    for(U i=0;i<0x5c9;++i)memory.set8(definition+i,0);
    w(definition,type);w(definition+8,1+(seed&1));w(definition+12,1+(seed&2?1:0));w(definition+24,100);w(definition+0x74,U(-1));w(definition+0x7c,3);w(definition+0x8c,4);
    w(p,0);w(p+4,1);w(p+8,2);w(p+12,2);w(p+16,1);w(p+0xa8,type);w(p+0xac,definition);w(p+0xe4,80);w(p+0x174,0);w(p+0x64c,U(-1));w(p+0x6a8,U(-1));w(p+0x5e8,U(-1));
    w(p+0x8f9,Manager);w(Manager,Nodes);w(Manager+4,Nodes);w(Manager+8,0);w(Nodes,Nodes);w(Nodes+4,Nodes);
    w(0x6def58,Pool);w(0x6def5c,4);w(0x6df180,1);w(0x644520,1);w(0x6f37fc,0);w(0x6e1f0c,0);w(0x689920,0);w(0x6a49c0,0);w(0x5fed7c,0);w(0x6898dc,0);
    constexpr U Map=Base+0x16000;w(0x6c5494,6);w(0x6c5498,5);w(0x6c54a0,30);w(0x6c54dc,Map);w(0x65660c,Other+0x1200);
    for(U i=0;i<5;++i)w(0x6cb942+i*4,i*6);
    w(0x6cb8be,U(-30));for(U i=0;i<4;++i)w(0x6cb8c2+i*4,i*30);
    for(U i=0;i<120;++i){U at=Map+i*12;memory.set16(at,0);for(U off:{2u,4u,6u})memory.set16(at+off,0xffff);w(at+8,0);}
    w(Other+0x1200+0x94,0);w(Other+0x1200+0xc8,0);
    for(U owner=0;owner<8;++owner){w(0x643a80+owner*0x154,owner==0);w(0x643a8c+owner*0x154,owner==0?0:U(-1));}
    w(0x643a90,0);w(0x65b290,100);
    if(entry==0x506290)args={type,0,1};else if(entry==0x51f5e0)args={(seed/4)&1};
}

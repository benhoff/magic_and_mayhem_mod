// Independent inputs for concrete supplier execution. Each root is reseeded.
constexpr U Definition=Other+0x1000,Table=Other+0x1d00;
w(p,0);w(p+28,Definition);w(Definition,Definition+32);w(Definition+52,9);w(Definition+4,Array);w(Definition+8,Buffer);
for(U i=0;i<16;++i)w(Array+i*4,i*8);
for(U i=0;i<128;++i){w(Buffer+i*44,0);w(Buffer+i*44+4,1);}
for(U i=0;i<16;++i){U at=Buffer+i*352;w(at,1);w(at+4,seed%4);w(at+44,5);w(at+48,7);w(at+88,0);w(at+132,6);}
w(p+4,Buffer+88);w(p+8,seed%5!=0);w(p+12,seed%4);w(p+16,(seed/4)%4);w(p+20,0);w(p+24,Buffer+(seed%4)*44);w(p+32,1);w(p+36,0);w(p+40,(seed/8)%3);w(p+44,3);
for(U i=0;i<250;++i){w(0x5eb940+i*4,(i+1)%250);w(0x6574d0+i*4,seed*12345u+i*712397u+17u);}w(0x6574c8,seed%250);w(0x6574cc,(seed+103)%250);
w(0x5eb480,128);w(0x5eb484,96);w(0x5eb488,32);w(0x6c4830,seed*5);
w(Table,0x1001f480);w(Table+20,0x1001f440);
for(U i=0;i<0x701;++i){U token=Base+0x6000+(i%8)*64;w(token,Table);w(token+40,(i%8)*300);w(token+44,0);w(Manager+i*4,token);}
w(Manager+0x1c00,seed&1?0x700:seed%8);w(Manager+0x1c04,seed&2?2200:0);
w(Other+0x400,Table);w(Other+0x428,seed&4?2000:500);
switch(entry){
case 0x464b20:p=Definition;break;
case 0x464b30:p=Definition;args={seed%16};break;
case 0x464c50:case 0x464d70:case 0x465000:case 0x464ec0:break;
case 0x4f7970:for(U i=0;i<4;++i)w(Output+i*4,seed*7+i*3);args={Output};break;
case 0x510480:w(p+0xe4,seed&1);w(p+0xf4,seed&2);w(p+0xf0,seed&4);break;
case 0x57dc20:p=Buffer;second=seed*17;args={seed*32};break;
case 0x57dc60:p=seed;second=seed+2;args={seed*13,seed*19};break;
case 0x4a5a90:memory.set8(Output,seed*7);args={Output,seed%7==6?0x1234u:1u<<(seed%7)};break;
case 0x545590:case 0x545e90:args={seed*17-256,seed*19-128,seed*3-32};break;
case 0x545e80:case 0x5456b0:case 0x54aa00:break;
case 0x545e60:case 0x545690:case 0x54aa10:args={seed%4};break;
case 0x54aa80:w(p+40,seed*31-300);w(p+44,seed%7-1);break;
case 0x545ae0:args={seed*9-64,seed*11-128,seed*7-32};break;
case 0x54e200:p=0x6574c8;break;
case 0x5457e0:p=Manager;args={Other+0x400};break;
case 0x466900:
    p=0x643a80;for(U i=0;i<8;++i){memory.set8(p+i*0x154+0x50,'a'+i);memory.set8(p+i*0x154+0x51,0);}memory.set8(Output,'a'+seed%10);memory.set8(Output+1,0);args={Output};break;
case 0x56ba50:case 0x56ba60:case 0x56ba70:case 0x56ba80:p=seed%7-3;args={(seed/7)%7-3};break;
case 0x56b790:{
    w(0x6cbb52,Other+0x800);w(0x6cbb56,2);w(0x6c5494,6);w(0x6c5498,5);w(p+8,seed%8-1);w(p+12,(seed/8)%7-1);w(p+16,seed%3);w(p+0xac,Catalog);w(Catalog+8,1+(seed/4)%2);w(Catalog+12,1+(seed/16)%2);
    for(U i=0;i<2;++i){U at=Other+0x800+i*72;w(at,seed&1?4:1);w(at+4,seed&2?3:1);w(at+8,1);w(at+12,seed&1?1:4);w(at+16,seed&2?1:3);w(at+20,2);memory.set8(at+24,'a'+i);memory.set8(at+25,0);}memory.set8(Output,'a'+seed%3);memory.set8(Output+1,0);args={Output,p};p=0;break;
}
case 0x565180:{
    U record=Other+0x800,event=1+seed%10;for(U i=0;i<0x47c;++i)memory.set8(record+i,0);
    w(record,seed%11!=0);w(record+4,event==4||event==1?1:event==10?2:4);w(record+8,seed%13==0);
    w(record+0x1a8,seed&1);w(record+0x1f4,seed&2);w(record+0x138,seed&4?U(-1):5);w(record+0x13c,seed%8);w(record+0x27c,seed&8);memory.set8(0x5fc874,0);
    if(event==3&&(seed&16)){memory.set8(record+0x88,'x');memory.set8(record+0x89,0);w(record+0xa8,2);memory.set8(record+0x2d0,'x');memory.set8(record+0x2d1,0);}
    constexpr U ObjectRow=Other+0xe00;w(0x6f37f8,ObjectRow);w(0x6f37fc,seed&8?0:1);w(ObjectRow,1);memory.set8(ObjectRow+4,'x');memory.set8(ObjectRow+5,0);w(ObjectRow+40,seed&16?1:0);memory.set8(record+0x2cc,'x');memory.set8(record+0x2cd,0);
    args={record,event,p};break;
}
case 0x564d60:case 0x566d80:{
    U record=Other+0x800;for(U i=0;i<0x47c;++i)memory.set8(record+i,0);w(record,1);w(record+4,seed%11);w(record+8,0);w(record+12,seed%5-1);w(record+16,seed&1);w(record+20,5);w(record+24,seed&2);w(record+28,7);w(record+48,seed&4);w(record+52,9);w(record+0x27c,seed&8);w(record+0x2cc,seed%65-2);w(record+0x2d0,(seed+7)%65-2);w(p,record);w(p+8,1);w(p+32,17);args={record,Other};break;
}
case 0x4d47f0:{
    w(p+0x82,seed&1);w(p+0xb1,seed&2?Other+0x600:0);w(p+0x31,Other);w(Other+32,11);w(Other+36,13);w(p+0x11,20);w(p+0x15,30);w(p+0x19,150);w(p+0x1d,90);w(0x6e1ee4,seed&4);break;
}
case 0x4d72d0:{
    constexpr U Vtable=Other+0x1c00;for(U off:{0u,16u,20u})w(Vtable+off,0x1001f140);w(p+0x1555,seed%3);w(p+0x1910,seed&1);w(p+0x1518,seed%5);w(p+0x151c,Vtable);w(p+0x1596,Vtable);w(p+0x1520,U(-1));w(p+0x159a,U(-1));for(U i=0;i<4;++i){U widget=p+0xfc+i*0xc6;w(widget,Vtable);w(widget+4,U(-1));w(widget+0x3d,i%3);}w(0x68991c,0);break;
}
default:throw std::runtime_error("Missing concrete provider seed");
}

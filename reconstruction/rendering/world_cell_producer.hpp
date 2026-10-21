#pragma once

namespace mnm::reconstruction {
// Addresses are PE32 values in the backend's owned address space. External
// gameplay/animation services are explicit calls; none may re-enter 4f8960.
// Queue publication and all producer decisions/stores are native.
template<class Backend> class WorldCellProducer {
    using U=unsigned;
    using I=int;
    Backend& b;
    static constexpr U unset=0xffff8ad0u;
    U r(U p){return b.r(p);}
    I i(U p){return I(r(p));}
    U h(U p){return b.r16(p);}
    U byte(U p){return b.r8(p);}
    void w(U p,U v){b.w(p,v);}
    void wh(U p,U v){b.w16(p,v);}
    template<class... A> U call(U e,U p,A... a){return b.call(e,p,U(a)...);}
    struct Visual {U sprite;I x,y;};
    struct Frame {
        U s,t,view;I light,dx{},dy{},extra{},wrap{};
        U x,y,z;bool admitted{true};
    };
    static I low(U n){return I(short(n));}
    static I absolute(I n){return n<0?I(0u-U(n)):n;}
    static I clamp(I n){return n< -126?-127:n>=127?127:n;}
    void project(U view,I x,I y,I& sx,I& sy){
        switch(view){
        case 0:sx=x-y;sy=(x+y)/2;break;
        case 1:sx=x+y;sy=(y-x)/2;break;
        case 2:sx=y-x;sy=-((x+y)/2);break;
        case 3:sx=-(x+y);sy=(x-y)/2;break;
        default:b.fault("Unclassified world view");
        }
    }
    U frame(U collection,U number){
        if(r(r(collection)+12)<number)number=0;
        return r(r(collection+8)+4*number)+r(collection+12);
    }
    Visual animation(U reference,U asset){
        if(!reference)return {0,0,0};
        return {frame(r(asset+0x110),r(reference+4)),i(reference+8),i(reference+12)};
    }
    Visual visual(U entry,U object){
        const U out=b.scratch();call(entry,object,out);
        return {r(out),i(out+4),i(out+8)};
    }
    void pair(U entry,U object,I& x,I& y){
        const U out=b.scratch()+16;w(out,U(x));w(out+4,U(y));
        call(entry,object,out,out+4);x=i(out);y=i(out+4);
    }
    bool hit(U sprite,I x,I y,U point,bool rectangle=false){
        return b.hit(rectangle?0x57ddb0u:0x57dcf0u,sprite,U(x),U(y),point)!=0;
    }
    U pool(U id,U manager,U stride){
        if(id>=r(manager+4))b.fault("World producer pool reference");
        return r(manager)+id*stride;
    }
    // Full 4ffd30 row publication. WORD lanes discard caller stack garbage.
    U draw(Frame& f,U sprite,U x,U y,U z,I sx,I sy,I priority,I shade,
           U palette,U kind,U first=unset,U owner=0,U prior=unset,U special=0){
        const U q=f.s+0x229;w(0x6e8db0,r(0x6e8db0)+1);
        if(r(q+8)<r(q+12)){
            const U row=r(q+4);w(row+4,sprite);
            const I quantum=i(0x5e41ac);I light=low(U(shade));
            if(quantum!=1 && light!=-127){
                if(!quantum)b.fault("Zero World shade quantum");
                light=(light/quantum)*quantum;
            }
            wh(row+0x10,U(light));wh(row+0x12,palette);w(row+0x18,kind);
            wh(row+0x1e,0);wh(row+0x1c,first);w(row+0x14,owner);wh(row+0x1e,prior);
            const I height=I(z)>=500?I(z*82u)/100:i(0x6e818c+z*4);
            U depth{};bool known=true;
            switch(r(q+20)){
            case 0:depth=x+y+U(height);break;
            case 1:depth=y-x+U(height);break;
            case 2:depth=U(height)-x-y;break;
            case 3:depth=x-y+U(height);break;
            default:known=false;break;
            }
            // The original high-height branches all use x+y in every view.
            if(known && I(z)>=500)depth=x+y+U(height);
            if(known)w(row,depth);
            w(row,r(row)+U(priority));
            if(kind==9)w(row+8,special);
            else {w(row+8,U(sx));w(row+12,U(sy));}
            w(q+4,row+36);w(q+8,r(q+8)+1);
        }
        return r(q+8)-1;
    }
    void pickedTile(Frame& f,U index){
        const I depth=i(r(f.s+0x229)+index*36);
        if(i(f.s+0x1ed)<depth){w(0x6c3750,index);w(f.s+0x1ed,U(depth));}
    }
    void terrain(Frame& f){
        const U s=f.s,t=f.t,v=f.view,level=r(s+0x85);
        const U flags=h(t+8),other=h(t+10);
        const bool ordinary=(!(flags&(8u<<(v&31))) || !(other&(0x200u<<((r(s+0x65)-1)&31))))
            && (!(other&0x40) || !(flags&4) || !(flags&0x8000) || r(0x5e41b0));
        if(!ordinary && r(s+0x61)-1>level)return;
        const U def=r(0x65660c)+h(t)*356;
        const U body=r(def+0x84+4*v),first=r(def+0xd0+4*v),second=r(def+0xe0+4*v);
        if(h(t+6)!=0xffff || h(t+4)!=0xffff || body){
            if(r(s+0x6d)==1 && r(s+0x65)<(byte(t+8)&3))f.admitted=false;
            if(body && f.admitted){
                const U bag=r(s+0x2d);const I ax=i(s+0x89),ay=i(s+0x8d);
                const bool picked=hit(frame(bag,body),ax,ay,s+0x91)
                    || (first && hit(frame(bag,first),ax,ay,s+0x91))
                    || (second && hit(frame(bag,second),ax,ay,s+0x91));
                if(picked){w(0x6c5488,r(s+0x7d));w(0x6c5484,r(s+0x81));w(0x6cbb64,level);}
                I shade=f.light-i(s+0xb1);if(shade< -127)shade=-127;
                const U row=draw(f,frame(bag,body),f.x,f.y,f.z,ax,ay,i(s+0x9d),shade,h(s+0xa9),(byte(t+10)&8)?31:33,unset,t);
                if(picked)pickedTile(f,row);
                if(!(byte(t+8)&4)){
                    if(!first){b.w8(t+8,byte(t+8)|4);wh(t+8,h(t+8)|0x8000);}
                    else draw(f,frame(bag,first),f.x,f.y,f.z,ax,ay,i(s+0x9d),shade,h(s+0xa9),33,unset,t);
                    if(!(h(t+8)&0x8000)){
                        if(!second)wh(t+8,h(t+8)|0x8000);
                        else draw(f,frame(bag,second),f.x,f.y,f.z,ax,ay,i(s+0x9d),shade,h(s+0xa9),33,unset,t);
                    }
                }
            }
        }
        if(r(s+0x75) && r(r(s+0x21)+0x7f0) && level==r(r(s+0x21)+0x7f0)){
            const bool water=byte(def+0xa8)&0x20;
            if(!water)w(s+0x1f5,r(s+0x1f5)+1);
            if((f.light> -70 || !r(0x5e1404)) && (!h(t) || !water))
                draw(f,frame(r(s+0x2d),r(r(0x65660c)+0x1e8)),f.x,f.y,f.z,i(s+0x89),i(s+0x8d),i(s+0x9d)-1,f.light,0,6);
        }
    }
    void effects(Frame& f){
        U id=h(f.t+2);bool first=true;
        while(id!=0xffff){
            const U p=pool(id,0x689498,0x22e),type=r(p+0x28);
            if(!r(p+4) || r(p+8)!=r(f.s+0x7d) || r(p+12)!=r(f.s+0x81) || r(p+16)!=r(f.s+0x85))
                call(0x49bd20,p,f.t);
            else {
                if(call(0x49a600,p,f.t))break;
                const bool visible=r(p+0x22a) && (i(0x5e18e8)<f.light+15 || (call(0x49aa30,p)&255));
                if(visible || (first && r(0x644520)==~0u)){
                    const U below=I(r(p+16))<i(0x6c5c80);
                    I dx{},dy{};project(f.view,i(p+20)-i(p+8)*32-16,i(p+24)-i(p+12)*32-16,dx,dy);
                    const Visual art=visual(0x487c40,p);
                    U counter=type==14 || type==15 || type==16?0x1fd:type==25 || type==26 || type==27?0x219:type==10?0x205:0;
                    if(counter)w(f.s+counter,r(f.s+counter)+1);
                    if(art.sprite){
                        f.x=r(p+20);f.y=r(p+24);f.z=r(p+28);
                        if(type==8){
                            U x=r(p+8),y=r(p+12);
                            if(!(r(p+20)&31) && (f.view==0 || f.view==3))++x;
                            if(!(r(p+24)&31) && f.view<2)++y;
                            f.x=x*32+16;f.y=y*32+16;f.z=r(p+16)<<4;
                        }
                        const I shade=call(0x49a5f0,p)?f.light:0;
                        draw(f,art.sprite,f.x,f.y,f.z,i(f.s+0x89)+art.x+dx,
                            i(f.s+0x8d)-I(r(p+28)&15)+art.y+dy,i(f.s+0x9d)+7,shade,below,r(p+0x21e));
                    }
                }
            }
            id=h(p+0x1a0);first=false;
        }
    }
    void artifacts(Frame& f){
        const U s=f.s;
        if(r(0x6a49cc) && call(0x5344e0,0x6a49c0,r(s+0x7d),r(s+0x81),r(s+0x85))){
            f.x=r(s+0x7d)*32+16;f.y=r(s+0x81)*32+16;f.z=r(s+0x85)<<4;
            w(s+0x205,r(s+0x205)+1);
            const U out=b.scratch();call(0x534640,0x6a49c0,r(s+0x7d),r(s+0x81),out,out+12);
            draw(f,r(out),f.x,f.y,f.z,i(s+0x89)+i(out+4),i(s+0x8d)+i(out+8),i(s+0x9d)+7,f.light,r(s+0x85)<r(0x6c5c80),r(out+12));
        }
        U id=h(f.t+6);
        if(id==0xffff || !f.admitted || !(i(0x5e18f8)<f.light || !r(0x5e1404) || r(0x644520)==~0u))return;
        bool first=true;
        while(id!=0xffff){
            const U p=pool(id,0x6b1248,0x7e);const Visual art=animation(r(p+0x32),r(p+0x4a));
            if(art.sprite){
                f.x=r(p+8)*32+16;f.y=r(p+12)*32+16;f.z=r(p+16)<<4;
                bool picked;
                if(first){
                    const I dx=absolute(i(s+0x91)-(I(r(art.sprite+4)>>1)-i(art.sprite+12)+art.x+i(s+0x89)));
                    const I dy=absolute(i(s+0x95)-(I(r(art.sprite+8)>>2)-i(art.sprite+16)+art.y+i(s+0x8d)));
                    picked=b.distance(dx,dy)<b.real(0x5c7140);
                }else picked=hit(art.sprite,i(s+0x89)+art.x,i(s+0x8d)+art.y,s+0x91);
                if(picked)w(0x6dbea4,p);
                const U index=draw(f,art.sprite,f.x,f.y,f.z,i(s+0x89)+art.x,i(s+0x8d)-I(r(p+28)&15)+art.y,
                    i(s+0x9d)+2,f.light+(picked?32:0),i(p+16)<i(0x6c5c80),r(p+0x5e));
                if(picked){const I depth=i(r(s+0x229)+index*36);if(i(s+0x1ed)<depth){w(0x6c3750,index);w(s+0x1f1,U(depth));}}
            }
            id=call(0x5437f0,p)&0xffff;first=false;
        }
    }
    void creatures(Frame& f);
    void creature(Frame& f,U actor);
    void creatureDepth(Frame& f,U actor);
public:
    explicit WorldCellProducer(Backend& backend):b(backend){}
    void produce(U scene){
        Frame f{};f.s=scene;f.t=r(scene+0x79);f.view=r(scene+0x31);
        if(f.view>3)b.fault("Unclassified world view");
        const U offset=r(0x6cb942+r(scene+0x81)*4)+r(0x6cb8c2+(r(scene+0x85)>>1)*4)+r(scene+0x7d);
        f.light=I(static_cast<signed char>(byte(r(0x6c5c5c)+offset)));
        f.x=r(scene+0x7d)*32+16;f.y=r(scene+0x81)*32+16;f.z=r(scene+0x85)<<4;
        terrain(f);effects(f);artifacts(f);creatures(f);
    }
};

template<class Backend>
void WorldCellProducer<Backend>::creatureDepth(Frame& f,U a){
    I x=i(a+8),y=i(a+12),z=i(a+16);f.dx=f.dy=0;
    if(r(a+0x618) && !r(a+0x610)){
        static constexpr I dx[]={0,-32,-32,-32,0,32,32,32};
        static constexpr I dy[]={32,32,0,-32,-32,-32,0,32};
        const U direction=r(a+0x608);
        if(direction<8){
            f.dx=dx[direction];f.dy=dy[direction];
            const I width=i(0x6c5494),height=i(0x6c5498);
            if(f.dx && x==(f.dx<0?width-1:0)){
                const I sign=(f.view==0 || f.view==3)?1:-1;
                // View1 deliberately uses the height register for both axes.
                // Preserve the original rectangular-map priority asymmetry.
                const I extent=f.view==1?height:width;
                f.wrap+=(f.dx<0?-extent:extent)*32*sign;
            }
            if(f.dy && y==(f.dy<0?height-1:0)){
                const I sign=f.view<2?1:-1;
                f.wrap+=(f.dy<0?-height:height)*32*sign;
            }
        }
        const U history=a+0x97f+r(a+0x977)*28;
        I hx=i(history),hy=i(history+4),hz=i(history+8);
        const I width=i(0x6c5494),height=i(0x6c5498);
        if(width/2<absolute(x-hx))hx+=hx<x?width:-width;
        if(height/2<absolute(y-hy))hy+=hy<y?height:-height;
        if(f.view==1 || f.view==2){if(hx<x)x=hx;}
        else if(x<hx)x=hx;
        if(f.view<2){if(y<hy)y=hy;}
        else if(hy<y)y=hy;
        if(z<hz)z=hz;
    }
    if(r(r(a+0xac)+8)==2){
        if(f.view==0 || f.view==3)++x;
        if(f.view<2)++y;
    }
    f.x=U(x)*32+16;f.y=U(y)*32+16;f.z=U(z)<<4;
    if(r(a+0x5fc)==10 || r(a+0x5fc)==20){
        const U out=b.scratch()+32;w(out,f.x);w(out+4,f.y);w(out+8,f.z);
        b.relocate(a,out,out+4,out+8);f.x=r(out);f.y=r(out+4);f.z=r(out+8);
    }
    if(r(a+0xa8)==17){
        const U facing=(r(a+0x608)-f.view*2)&7;
        if(facing==0 || facing==6){
            if(f.view==0 || f.view==3)f.x+=30;
            else if(f.view==1)f.y+=30;
            else f.x-=30;
        }else if(facing==7){
            switch(f.view){
            case 0:f.x+=32;f.y+=30;break;
            case 1:f.y+=32;f.x-=30;break;
            case 2:f.y-=32;f.x-=30;break;
            case 3:f.x+=32;f.y-=30;break;
            }
        }
    }
}

template<class Backend>
void WorldCellProducer<Backend>::creature(Frame& f,U a){
    const U s=f.s;creatureDepth(f,a);
    U below=f.t;I shadow=I(r(a+28)&15);
    if(r(a+0x108) || r(a+0x5ec)==16){
        const U map=r(s+0x21),floor=r(map+0x4c)+r(map+0x10)*12;
        while(below>=floor && !(byte(r(0x65660c)+h(below)*356+0xb0)&8)){
            shadow+=16;below-=r(0x6c54a0)*12;
        }
    }
    shadow+=2-i(r(0x65660c)+h(below)*356+0x94);
    I px{},py{};project(f.view,i(a+0xb7b)-i(a+8)*32+i(a+20)+f.dx,
        i(a+0xb7f)-i(a+12)*32+i(a+24)+f.dy,px,py);
    const Visual body=visual(r(a+0x855)?0x507330:0x507190,a);
    I x=i(s+0x89)+px,y=i(s+0x8d)+i(s+0x85)*16-i(a+28)+py;
    const U clipped=U(low(r(a+28)-r(0x6c5c80)*16));
    I shade=f.light;
    if(r(a+0x174)==r(0x644520) && shade<=i(0x5e18e8))shade=i(0x5e18e8);
    bool picked=false,eligible=r(a+0xe4)!=0;
    if(!eligible){
        const U player=r(0x644520);
        if(call(0x466870,0x643a80,player)){
            const U wizard=call(0x466870,0x643a80,r(0x644520));
            eligible=r(wizard+0x5e8)!=~0u && r(wizard+0x570+r(wizard+0x5e8)*4)==52;
        }
    }
    if(eligible){
        if(r(0x6903e8)==3 && hit(body.sprite,body.x+x,body.y+y,0x690438,true)){
            shade+=32;picked=true;w(a+0x74,1);
        }else if(hit(body.sprite,body.x+x,body.y+y,s+0x91)){
            picked=true;w(s+0xbd,0);
        }else if(r(s+0xbd)){
            const I dx=absolute(i(s+0x91)-(I(r(body.sprite+4)>>1)-i(body.sprite+12)+body.x+x));
            const I dy=absolute(i(s+0x95)-(I(r(body.sprite+8)>>2)-i(body.sprite+16)+body.y+y));
            if(b.distance(dx,dy)<b.distance(i(s+0xb5),i(s+0xb9))){w(s+0xb5,U(dx));w(s+0xb9,U(dy));picked=true;}
        }
    }
    // 51fa60: read all six overlapping words before shifting, then resolve the
    // potentially aliased descriptor after the two position stores.
    U history[6];for(U k=0;k<6;++k)history[k]=r(a+0x754+k*4);
    for(U k=0;k<6;++k)w(a+0x760+k*4,history[k]);
    w(a+0x754,U(body.x+x));w(a+0x758,U(body.y+y));w(a+0x75c,r(r(a+0xb4)+4));
    I adjusted=low(U(shade));
    if(i(a+0xddf)<0 && adjusted> -111){adjusted+=i(a+0xddf);if(adjusted< -110)adjusted=-111;}
    if(r(a+0x798) && r(a+0xa0)!=1 && call(0x51fef0,a)!=3){
        const U jitter=(call(0x54e200,0x6574c8)&0x7fffffff)%10;if(jitter<4)y-=I(jitter);
    }
    const U mode=r(a+0xbf3);U palette=r(a+0xcff);if(!palette)palette=unset;
    const U row=draw(f,body.sprite,f.x,f.y,f.z,body.x+x,body.y+y,i(s+0x9d)+f.wrap+6+f.extra,
        clamp(i(r(a+0xcf7)+600+mode*4)+adjusted),h(a+0xcfb),r(r(a+0xcf7)+mode*4),clipped,0,palette);
    if(r(a+0x821))adjusted=low(U(shade));
    for(U k=0;k<2;++k){
        const U data=r(a+0xcf7),kind=r(data+120+k*120+r(a+0xbf3)*4);
        if(kind!=~0u){
            U token=r(a+0xcff);if(!token)token=unset;
            draw(f,body.sprite,f.x,f.y,f.z,body.x+x,body.y+y,i(s+0x9d)+f.wrap+7+I(k)+f.extra,
                clamp(adjusted+i(data+600+r(a+0xbf3)*4)),h(a+0xcfb),kind,clipped,0,token);
        }
    }
    const U shadowKind=r(r(a+0xcf7)+0x168+r(a+0xbf3)*4);
    if(shadowKind!=~0u)draw(f,body.sprite,f.x,f.y,f.z,body.x+x,shadow+body.y+y,
        i(s+0x9d)+f.wrap+3+f.extra,shade,0,shadowKind,clipped);
    if(r(a+0x778) && !r(0x6899c9))for(U k=0;k<3;++k){
        const U out=b.scratch();call(0x51faa0,a,k,out);const Visual v{r(out),i(out+4),i(out+8)};
        if(v.x==-1)break;
        const U kind=r(a+0xa8)==13 || r(a+0x8c)?3:k==0?5:k==1?4:3;
        U token=r(a+0xcff);if(!token)token=unset;
        draw(f,v.sprite,f.x,f.y,f.z,v.x,v.y,i(s+0x9d)+I(k)+f.wrap+6+f.extra,
            clamp(i(r(a+0xcf7)+600+r(a+0xbf3)*4)+adjusted),h(a+0xcfb),kind,clipped,0,token);
    }
    if(r(0x6903e8)!=3 && picked){w(0x657d40,row);w(s+0x1e9,r(r(s+0x229)+row*36));w(0x656600,a);}
    I ax{},ay{};pair(0x507250,a,ax,ay);
    auto overlay=[&](U reference,U asset,I priority,U kind=31,bool offsets=true,I atx=0,I aty=0){
        const Visual v=animation(reference,asset);
        draw(f,v.sprite,f.x,f.y,f.z,x+atx+(offsets?v.x:0),y+aty+(offsets?v.y:0),
            i(s+0x9d)+priority,shade+32,0,kind);
    };
    if(r(a+0xe4) && r(a+0x124))overlay(r(a+0x130),r(a+0x148),7+f.wrap,31,false,ax,ay);
    if(r(a+0xe4)){
        if(call(0x51fef0,a)==1)overlay(r(a+0x7d9),r(a+0x7f1),8,31,true,ax,ay);
        if(r(a+0xe4)){
            if(r(a+0x889))overlay(r(a+0x85d),r(a+0x875),9+f.wrap,31,true,ax,ay);
            if(r(a+0xe4) && r(a+0x8d1))overlay(r(0x6df150),r(0x6df168),10+f.wrap,31,true,ax,ay);
        }
    }
    if(r(a+0x8bd))overlay(r(a+0x891),r(a+0x8a9),11+f.wrap,31,true,ax,ay);
    if(r(a+0xe4)){
        if(r(a+0x780))overlay(r(0x6df060),r(0x6df078),12+f.wrap,31,false,ax,ay);
        if(r(a+0xe4)){
            if(r(a+0x8d5))overlay(r(0x6df090),r(0x6df0a8),13+f.wrap,2,true,ax,ay);
            if(r(a+0xe4)){
                if(r(a+0x963))overlay(r(0x6df0f0),r(0x6df108),14+f.wrap,2,true,ax,ay);
                if(r(a+0xe4)){
                    if(byte(a+0x962))overlay(r(0x6df0c0),r(0x6df0d8),15+f.wrap,31,true,ax,ay);
                    if(r(a+0xe4)){
                        if(r(a+0x78c))overlay(r(0x6df120),r(0x6df138),16+f.wrap,31,true,ax,ay);
                        if(r(a+0xe4) && call(0x51fef0,a)==3){
                            const Visual v=animation(r(a+0x7d9),r(a+0x7f1));I tx=ax,ty=ay;
                            if(r(r(a+0xac)+8)==2)pair(0x5072c0,a,tx,ty);else ty+=I(r(v.sprite+8)>>1);
                            draw(f,v.sprite,f.x,f.y,f.z,v.x+tx+x,v.y+ty+y,i(s+0x9d)+5+f.wrap,shade+32,0,31);
                        }
                    }
                }
            }
        }
    }
    const bool ownerIcon=(!r(a+0x68) && !r(a+0x70) && !call(0x506220,a) && i(0x5e18f4)<=low(U(shade))) || r(0x6e1f0c);
    if(ownerIcon && r(a+0x5fc)!=3){
        const U owner=r(a+0x174),army=owner==~0u?0:0x643a80+owner*0x154;
        draw(f,frame(0x6593e0,r(army+0x150)+26),f.x,f.y,f.z,ax+x,ay+y,i(s+0x9d)+11+f.wrap,shade+32,0,31);
    }
    pair(0x5072c0,a,ax,ay);
    if(r(a+0xe4) && r(a+0x744)){
        const U special=r(a+0xa8)==15 || r(a+0xa8)==1?0x6df000:0x6df030;
        overlay(r(special),r(special+24),8+f.wrap,2,false,ax,ay);
    }
    if(r(a+0xe4) && byte(a+0x7cc)){
        const Visual v=visual(0x520540,a);
        draw(f,v.sprite,f.x,f.y,f.z,v.x+ax+x,v.y+ay+y,i(s+0x9d)+5+f.wrap,shade+32,0,31);
        draw(f,v.sprite,f.x,f.y,f.z,v.x+ax+x,shadow+v.y+ay+y,i(s+0x9d)+5+f.wrap,shade+32,0,1);
    }
    auto corner=[&](U type,U direction,U index,I& cx,I& cy){
        const U out=b.scratch()+16;call(0x505aa0,type,out,out+4,direction,index);cx=i(out);cy=i(out+4);
    };
    auto marker=[&](U number,I cx,I cy){
        draw(f,frame(0x6593e0,number),f.x,f.y,f.z,cx+ax+x,cy+ay+y,i(s+0x9d)+5000+f.wrap,0,0,31);
    };
    if(r(a+0x6e4)){
        const U direction=r(a+0x608),type=0x6a5f80+r(a+0xa8)*0x5c9;
        for(U k=0;k<4;++k){I cx{},cy{};corner(type,direction,k,cx,cy);marker(k+4,cx,cy);}
    }
    if(r(a+0x68) || r(a+0x70)){
        const U direction=r(a+0x608),task=r(a+0x8e9),player=r(0x644520),owner=r(a+0x174);
        U base=12;
        if(player!=~0u){
            if(owner==player)base=r(r(a+0xac)+8)==2?8:0;
            else {const U army=owner==~0u?0:0x643a80+owner*0x154;if(byte(army+player+24))base=34;}
        }
        const U type=0x6a5f80+r(a+0xa8)*0x5c9;I cx{},cy{};
        corner(type,direction,0,cx,cy);marker(base,cx,cy);
        corner(type,direction,1,cx,cy);marker(base+1,cx,cy);
        if(task!=~0u)marker(task+16,cx,cy);
        const U index=r(s+0x1e5);w(s+0xf5+index*4,a+0x6b4);
        call(0x58f7d0,r(s+0xf5+index*4),cx+ax+2+x,cy+ay+y);w(s+0x1e5,r(s+0x1e5)+1);
        corner(type,direction,2,cx,cy);marker(base+2,cx,cy);
        corner(type,direction,3,cx,cy);marker(base+3,cx,cy);
    }
}

template<class Backend>
void WorldCellProducer<Backend>::creatures(Frame& f){
    if(byte(f.t+11)&0x20)call(0x52b0b0,0x6def58);
    U id=h(f.t+4);
    if(id!=0xffff){
        const U a=pool(id,0x6def58,0xe4b);
        const U cell=r(0x6c54dc)+(r(0x6cb942+r(a+12)*4)+r(0x6cb8c2+r(a+16)*4)+r(a+8))*12;
        if(!(h(cell+10)&1) && r(a+0x5ec)==1)wh(cell+10,h(cell+10)|1);
        if(call(0x506220,a)){
            const U player=r(0x644520);
            if(player>=8 || !r(a+0x24+player*4))call(0x508ca0,a,1,player,call(0x466870,0x643a80,player));
        }
    }
    id=h(f.t+4);U a=0;
    if(id!=0xffff && (byte(f.t+10)&1)){
        a=pool(id,0x6def58,0xe4b);if(!f.admitted && i(a+0x108)<1)a=0;
    }
    bool first=true;
    for(;;){
        if(a){
            const U player=r(0x644520);
            if(!((player>=8 || !r(a+0x24+player*4)) && r(0x5e1404) && player!=~0u))creature(f,a);
        }
        if(!(byte(f.t+11)&0x20))return;
        a=call(0x52b0c0,0x6def58,r(f.s+0x7d),r(f.s+0x81),r(f.s+0x85));
        if(first){f.extra=-2;first=false;}else ++f.extra;
        if(!a || r(f.s+0x6d)!=1 || r(f.s+0x65)<(byte(f.t+8)&3))return;
    }
}
}

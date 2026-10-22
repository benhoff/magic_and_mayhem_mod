#pragma once
#include "actor_services.hpp"
namespace mnm::reconstruction::actor_services {
// Actor task/session services composed through M's explicit service dispatch.
// The caller owns nested animation data, actor pools and callback lifetimes.
inline U actionCallback(U action){
    switch(action){
    case 1:case 6:case 16:case 17:case 19:case 22:case 25:return 0x50f090;
    case 2:return 0x5104b0;case 3:return 0x50f360;case 8:return 0x50e6f0;
    case 9:return 0x50e670;case 10:return 0x50e7a0;case 13:return 0x50f3b0;
    case 14:return 0x50f590;case 15:return 0x50f7c0;case 18:return 0x50eba0;
    case 20:return 0x5170e0;case 21:return 0x521790;case 23:return 0x5217e0;
    case 24:return 0x521dd0;default:return 0;
    }
}
template<class M> class Controllers {
    M& m;
    U r(U p){return m.get32(p);} U w(U p,U v){m.set32(p,v);return v;}
    U call(U id,U self,std::initializer_list<U> args={}){return m.invoke(id,self,args);}
    U actor(U index){return index<r(0x6def5c)?r(0x6def58)+index*0xe4b:0;}
    bool leader(U p){U kind=r(r(p+0xac));return kind==0||kind==24||kind==25||kind==26;}
    void animation(U p,U offset,U reverse=0){w(p+0xe0,offset);call(0x464cb0,p+0xb0,{offset+r(p+0x60c),reverse});}
    void overlay(U p,bool melee=false){
        if(r(p+0x7cd)!=1)return;
        U source=r(0x6898d0)+(melee?0x230:0x240),controller=p+0x7d5;
        call(0x464ca0,controller,{r(0x6894a0)+r(source+4)*0x114});
        call(0x464cb0,controller,{r(source)+r(p+0x60c),0});
    }
public:
    explicit Controllers(M& memory):m(memory){}
    U run(U entry,U p,std::initializer_list<U> args={},U second=0){
        U a[5]{};unsigned n=0;for(U v:args){if(n==5)throw std::invalid_argument("controller argument extent");a[n++]=v;}
        switch(entry){
        case 0x463cb0:{
            U value=r(0x5fbf1c);
            if(!value){value=call(0x597850,0,{0x1cb});if(value){call(0x59c441,0,{value,0x97,3,0x464350,0x463270});w(value+0x1c5,0);m.set8(value+0x1c9,0);m.set8(value+0x1ca,0);}w(0x5fbf1c,value);}
            return value;
        }
        case 0x464cb0:{
            call(0x464c80,p);w(p,a[0]);U start=r(r(p+28)+8)+r(r(r(p+28)+4)+a[0]*4)*44;
            w(p+8,1);w(p+36,a[1]);w(p+24,start);
            if(!r(start))w(p+4,start);else call(0x465000,p);
            if(r(p+8)&&r(p+36)){
                w(p+40,0);w(p+36,0);U display=0,current=0,previous=0,count=0;
                do{display=r(p+24);current=r(p+4);U result=call(0x464ec0,p);previous=r(p+40);count=previous+1;w(p+40,count);if(!r(p+8)||result==2)break;}while(r(display)!=4||signed32(r(display+4))>=0);
                w(p+44,count);w(p+24,display);w(p+4,current);w(p+36,1);w(p+40,previous);w(p+8,1);
            }
            break;
        }
        case 0x472730:{
            call(0x4d6c60,0x6a2dd0);call(0x4d39b0,0x6c4e40);w(0x6903f4,0);
            if(r(p+0x100b3))call(0x4cb9b0,r(p+0x100b3));
            U count=r(0x6de6d9);if(!count)break;w(0x5d9e38,count);
            if(signed32(count)>0&&r(0x6903e8)!=3&&!m.get8(0x6c4859)){w(0x6de6d9,count-1);w(0x6c482c,(r(0x6de6d5)?0x5e1720:0x5e1700)+(count-1)*16);}
            call(0x4f7970,0x689930,{r(0x6c482c)});call(0x4d7a90,0x6a2dd0);w(0x5e16fc,1);break;
        }
        case 0x494a50:
            if(r(p+40)==52){U descriptor=r(r(p+36)+0x438),peer=actor(r(p+68));w(p+0x198,peer);call(0x464e20,p+0x160,{r(peer+0x60c)+r(descriptor+0x2d0)});}break;
        case 0x5057b0:
            if(p&&!(p>=24&&p<=26))return r((m.get8(0x6dbe9d)?0x5c71b4:0x5c7144)+p*4);
            if(r(0x689920)==5&&second==r(0x644520)&&!p){U owner=second==U(-1)?0:0x643a80+second*0x154;U profile=r(0x65b34c+r(owner+16)*0x93a);return r((m.get8(0x6dbe9d)?0x5c723c:0x5c7224)+profile*4);}
            return 0x6f9de0;
        case 0x509bd0:{
            if(r(p+0x174)==r(0x644520)){
                if(a[0]==U(-1)){
                    w(0x6903e8,r(0x6e984c)!=0);
                    if(leader(p)&&!r(p+0x7c)&&r(p+0xe4)){
                        call(0x4d3b20,0x6c4e40,{0});U owner=call(0x466870,0x643a80,{r(0x644520)}),index=r(owner+0x5e8);
                        call(0x4d6c80,0x6a2dd0,{index==U(-1)?U(-1):r(owner+0x570+index*4),0});
                    }
                }else{
                    U peer=r(0x6def58);
                    for(U i=0;i<r(0x6df180);++i,peer+=0xe4b)if(r(peer+4)){
                        if(!r(peer+0x68)){if(r(peer+0x70))w(peer+0x70,0);}
                        else{w(peer+0x68,0);w(0x6e984c,r(0x6e984c)-1);w(0x6903e8,r(0x6e984c)!=0);if(r(peer+0x6e8)){call(0x4943f0,r(peer+0x6e8));w(peer+0x6e8,0);}call(0x512340,peer);if(leader(peer)&&!r(peer+0x7c))call(0x4d3ea0,0x6c4e40);}
                    }
                    w(0x6df17c,0);w(0x6903e8,2);if(leader(p)&&!r(p+0x7c))call(0x4d3b20,0x6c4e40,{r(0x6b1270+a[0]*0x2d1)});
                }
            }
            U index=0;for(;index<r(p+0x56c);++index)if(r(p+0x570+index*4)==a[0]){w(p+0x5e8,index);return 0;}w(p+0x5e8,U(-1));break;
        }
        case 0x50ab30:{
            U mode=r(p+0xa0);if(mode){if(mode==2||mode==1)call(0x54dce0,p+0xbf3,{mode==2?1u:4u});w(p+0xa0,0);w(p+0x9c,0);}
            if(r(p+0x5f0)==8)break;
            if(r(p+0x784))w(p+0x784,0);
            if(r(p+0x10c)==3)w(p+0x108,0);
            w(p+0xb73,0);w(p+0x164,0);
            if(r(p+0x664)){
                if(r(p+0x5fc)==10){U peer=actor(r(p+0x64c));w(p+0x664,0);U count=r(peer+0x668)-1;w(peer+0x668,count);if(signed32(count)<0)w(peer+0x668,0);}
                else if(r(p+0x5fc)==20){U peer=actor(r(p+0x6a8));w(p+0x664,0);call(0x50eb60,peer,{0});}
            }
            w(p+0xbc7,r(p+12));m.set8(p+0x71c,0);w(p+0xde3,0);m.set8(p+0x962,0);w(p+0x963,0);m.set8(p+0x723,0);m.set8(p+0x722,0);
            for(U off:{0x932u,0x6a0u,0xb8bu})w(p+off,0);
            w(p+0xbc3,r(p+8));w(p+0xbcb,r(p+16));
            for(U off:{0x684u,0x6acu,0x698u})w(p+off,0);
            call(0x50aa30,p,{1});break;
        }
        case 0x50db60:{
            U action=a[0];if(r(p+0x618)&&action!=2)break;
            if(r(p+0x5fc)==2){w(p+0x16c,1);w(p+0x170,r(p+0xb63));}else if(r(p+0x5fc)!=15)w(p+0x16c,0);
            w(p+0x5fc,action);if(action!=2){w(p+0xb77,0);w(p+0xb7f,0);w(p+0xb7b,0);}
            switch(action){
            case 1:{U offset;if(!r(p+0x108)){U random=call(0x54e200,0x6e1ac8);if(r(0x6e1f30))call(0x485b10,0x657d78,{0x5e9c10,0x19d8});offset=0x40+((random&0x7fffffff)%6)*8;}else offset=r(r(p+0xac)+0x3c)?0xf8:0x120;animation(p,offset);overlay(p);break;}
            case 2:overlay(p);break;
            case 3:animation(p,r(p+0xf4)*8+0x128);break;
            case 6:animation(p,0x70);overlay(p);break;
            case 8:case 9:w(p+0x164,1);animation(p,0x100);overlay(p);break;
            case 10:case 20:{U peer=actor(r(p+(action==10?0x64c:0x6a8)));call(0x50e400,p,{peer});w(p+0x664,1);U count=r(peer+0x668)+1;w(peer+0x668,count);if(signed32(count)<1)w(peer+0x668,1);overlay(p,true);break;}
            case 13:animation(p,0x10);overlay(p);break;
            case 14:animation(p,0x90);overlay(p);break;
            case 15:animation(p,0x120);overlay(p);break;
            case 16:animation(p,0x108);overlay(p);break;
            case 17:animation(p,0x100);overlay(p);break;
            case 18:call(0x50e400,p,{0});overlay(p);break;
            case 19:animation(p,0xe8);overlay(p);break;
            case 21:animation(p,0x38);if(r(p+0x7cd)==1)call(0x513f50,p);
                break;
            case 22:animation(p,0xe8,1);break;
            case 23:case 24:case 25:
                if(action!=24||!call(0x510480,p))call(0x50e3d0,p,{action==25?0x100u:0x120u,0});
                if(r(p+0x7cd)==1)call(0x513f50,p);
                break;
            default:break;
            }
            w(p+0x600,actionCallback(action));w(p+0x604,0);break;
        }
        case 0x520e70:{
            w(p+0x798,0);m.set8(p+0x723,0);m.set8(p+0x722,0);
            if(!r(r(p+0xac)+0x3c)&&r(p+0x7cd)!=3)w(p+0x104,0);
            if(m.get8(p+0x7cc))call(0x465130,p+0x79c);
            if(!r(p+0xe4))break;
            if(!r(p+0x108)){U mode=r(p+0xa0);if(mode){w(p+0xa0,0);call(0x50d060,p,{r(p+0x9c),mode,1});break;}}
            else if(r(p+0xa8)!=27){
                w(p+0xa4,r(p+16));w(p+0xb8b,0);w(p+0xde3,0);m.set8(p+0x962,0);w(p+0x963,0);w(p+0x164,0);
                if(r(p+0x664)&&(r(p+0x5fc)==10||r(p+0x5fc)==20)){U peer=actor(r(p+(r(p+0x5fc)==10?0x64c:0x6a8)));w(p+0x664,0);U count=r(peer+0x668)-1;w(peer+0x668,count);if(signed32(count)<0)w(peer+0x668,0);}
                call(0x50aa30,p,{16});break;
            }
            call(0x50ab30,p);break;
        }
        case 0x522340:{
            U owner=call(0x466870,0x643a80,{r(p+0x174)});if(!owner)break;
            U peer=r(0x6def5c)?r(0x6def58):0,count=0;
            for(U n=r(0x6df180);signed32(n)>0;--n,peer+=0xe4b)if(r(peer+4)&&r(peer+0xe4)&&r(peer+0x174)==r(owner+0x174)&&r(peer)!=r(owner)&&r(peer+0xa8)!=27)++count;
            U before=r(owner+0x8f5);w(owner+0x8f5,count);
            if(leader(owner)&&!r(owner+0x7c)&&r(owner+0x174)==r(0x644520)){
                call(0x4dea00,0x6903f0,{count});U maximum=r(owner+0x8f1);
                if(before==maximum&&signed32(r(owner+0x8f5))<signed32(maximum))call(0x4d7a00,0x6a2dd0);
                else if(signed32(before)<signed32(maximum)&&r(owner+0x8f5)==maximum)call(0x4d78b0,0x6a2dd0);
            }break;
        }
        default:throw std::invalid_argument("Unrecovered actor controller service");
        }
        return 0;
    }
};
}

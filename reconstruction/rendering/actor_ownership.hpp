#pragma once
#include <cstdint>
#include <initializer_list>

namespace mnm::reconstruction::actor_lifecycle {
using U = std::uint32_t;
inline std::int32_t signedWord(U n) {
    return static_cast<std::int32_t>(n<=0x7fffffffu ? std::int64_t(n) : std::int64_t(n)-0x100000000ll);
}
constexpr U Stride=0xe4b;

// PE32 addresses are values in an explicitly owned memory service. None of
// these operations retains a host pointer or introduces a generation check.
template<class M> U firstFree(M& m,U manager) {
    U at=m.get32(manager),capacity=m.get32(manager+4);
    for(U index=0;index<capacity;++index,at+=Stride)
        if(!m.get32(at+4))return at;
    return 0;
}
template<class M> U nextActive(M& m,U manager,bool predicate) {
    while(m.get32(manager+8)<m.get32(manager+0x228)) {
        U at=m.get32(manager+12);
        if(m.get32(at+4) && (!predicate || m.invoke(m.get32(manager+16),at+m.get32(manager+20),{}))) {
            // Predicate callbacks may publish a new cursor/backing. The
            // original rereads these fields after the callback returns.
            at=m.get32(manager+12);
            m.set32(manager+8,m.get32(manager+8)+1);
            m.set32(manager+12,at+Stride);
            return at;
        }
        m.set32(manager+8,m.get32(manager+8)+1);
        m.set32(manager+12,m.get32(manager+12)+Stride);
    }
    return 0;
}

template<class M> void invalidate(M& m,U actor,U removed) {
    if(m.get32(actor+0x906)==removed)m.set32(actor+0x906,0xffffffffu);
    U target=m.get32(actor+0x64c);
    if(target==removed && m.get32(actor+0xe4)) {
        if(m.get32(actor+0x5fc)==10 && m.get32(actor+0x664)) {
            U other=target<m.get32(0x6def5c)?m.get32(0x6def58)+target*Stride:0;
            m.set32(actor+0x664,0);
            U count=m.get32(other+0x668)-1;
            m.set32(other+0x668,signedWord(count)<0?0:count);
        }
        bool special=m.get32(actor+0xa8)==5 && m.get32(actor+0x5fc)==10;
        m.set32(actor+0x64c,0xffffffffu);m.set32(actor+0x784,0);
        U task=m.get32(actor+0x5ec);
        if(task==3 || task==9 || task==10)m.invoke(0x50ab30,actor,{});
        if(m.get32(actor+0x5f0)==3)m.set32(actor+0x5f0,0);
        if(m.get32(actor+0x5fc)==10)m.invoke(0x50db60,actor,{15});
        if(special)m.invoke(0x50db60,actor,{25});
    }
    target=m.get32(actor+0x6a8);
    if(target==removed && m.get32(actor+0xe4)) {
        if(m.get32(actor+0x5fc)==20 && m.get32(actor+0x664)) {
            U other=target<m.get32(0x6def5c)?m.get32(0x6def58)+target*Stride:0;
            m.set32(actor+0x664,0);
            U count=m.get32(other+0x668)-1;
            m.set32(other+0x668,signedWord(count)<0?0:count);
        }
        m.set32(actor+0x6a8,0xffffffffu);
        if(m.get32(actor+0x5fc)==20)
            m.invoke(0x50db60,actor,{m.get32(actor+0xa8)==5?25u:15u});
    }
}

template<class M> U mapCell(M& m,U x,U y,U z) {
    return m.get32(0x6c54dc)+(m.get32(0x6cb942+y*4)+m.get32(0x6cb8c2+z*4)+x)*12;
}
template<class M> void attach(M& m,U actor,U x,U y,U z) {
    U base=mapCell(m,x,y,z),definition=m.get32(actor+0xac),height=m.get32(definition+12);
    m.set8(base+10,m.get8(base+10)|1);m.set16(base+4,m.get16(actor));
    U at=base;
    for(U i=1;i<height;++i) {
        at+=m.get32(0x6c54a0)*12;
        m.set8(at+10,m.get8(at+10)|2);m.set16(at+4,m.get16(actor));
    }
    if(m.get32(definition+8)==2) {
        const U width=m.get32(0x6c5494),plane=m.get32(0x6c54a0);
        U right=base+12-(width<=x+1?width*12:0);
        U below=base+width*12-(m.get32(0x6c5498)<=y+1?plane*12:0);
        U diagonal=base+(width+1)*12-(width<=x+1?width*12:0)-
            (m.get32(0x6c5498)<=y+1?plane*12:0);
        for(U start:{right,below,diagonal})for(U i=0;i<height;++i) {
            at=start+i*plane*12;
            m.set16(at+10,m.get16(at+10)|2);m.set16(at+4,m.get16(actor));
        }
    }
    m.set16(base+8,m.get16(base+8)&0xff7f);
}
template<class M> bool emptyColumn(M& m,U cell) {
    U table=m.get32(0x6a49c0);
    if(!table)return true;
    U linear=(cell-m.get32(0x6c54dc))/12,plane=m.get32(0x6c54a0);
    U at=table+(linear%plane)*6;
    auto lower=static_cast<std::int8_t>(m.get8(at+2));
    auto upper=static_cast<std::int8_t>(m.get8(at+3));
    return !m.get16(at) || (signedWord(linear/plane)!=lower && signedWord(linear/plane)!=upper);
}
template<class M> void detach(M& m,U actor,U x,U y,U z) {
    U base=mapCell(m,x,y,z),definition=m.get32(actor+0xac),height=m.get32(definition+12);
    auto conditional=[&](U start) {
        for(U i=0;i<height;++i) {
            U at=start+i*m.get32(0x6c54a0)*12;
            if(U(m.get16(at+4))==m.get32(actor) && (m.get16(at+10)&0x83)) {
                m.set16(at+10,m.get16(at+10)&0xfffc);m.set16(at+4,0xffff);
                U terrain=m.get16(at);
                if(terrain && m.get32(m.get32(0x65660c)+terrain*0x164+0x94)==16)
                    m.set8(at+10,m.get8(at+10)|1);
            }
        }
    };
    conditional(base);
    if(m.get32(definition+8)==2) {
        U width=m.get32(0x6c5494),plane=m.get32(0x6c54a0);
        conditional(base+12-(width<=x+1?width*12:0));
        U below=base+width*12-(m.get32(0x6c5498)<=y+1?plane*12:0);
        U diagonal=base+(width+1)*12-(width<=x+1?width*12:0)-
            (m.get32(0x6c5498)<=y+1?plane*12:0);
        // The last two columns do not check the prior occupant or restore
        // terrain flags. Preserve this original asymmetry.
        for(U start:{below,diagonal})for(U i=0;i<height;++i) {
            U at=start+i*m.get32(0x6c54a0)*12;
            m.set16(at+10,m.get16(at+10)&0xfffc);m.set16(at+4,0xffff);
        }
    }
    if(!m.get16(base)&&m.get16(base+2)==0xffff&&m.get16(base+4)==0xffff&&
       m.get16(base+6)==0xffff&&!(m.get8(base+11)&0x20)&&emptyColumn(m,base))
        m.set8(base+8,m.get8(base+8)|0x80);
}
template<class M> bool leader(M& m,U actor) {
    U type=m.get32(m.get32(actor+0xac));
    return type==0 || type==24 || type==25 || type==26;
}
template<class M> void cleanup(M& m,U actor,U alternate) {
    m.set32(actor+0x889,0);m.set32(actor+0x8bd,0);
    if(!alternate) {
        if(leader(m,actor) && !m.get32(actor+0x7c)) {
            U network=m.get32(0x689920),owner=m.get32(actor+0x174);
            if((network==1 || network==2) && m.get8(0x6dbe93)) {
                m.invoke(0x4da720,0x6a26c8,{0x658f10+owner*0x133,6});
                m.invoke(0x49d640,0x689498,{actor});
            } else {
                if(m.get8(0x6dbe93) && owner==m.get32(0x644520))
                    m.invoke(0x4da690,0x6a26c8,{5});
                m.invoke(0x49d640,0x689498,{actor});
            }
        }
        m.invoke(0x5137d0,actor,{});
    }
    if(m.get32(actor+0x664) && (m.get32(actor+0x5fc)==10 || m.get32(actor+0x5fc)==20)) {
        m.set32(actor+0x664,0);m.invoke(0x50eb60,actor,{0});
    }
    if(m.get32(actor+0x124)==2)m.invoke(0x52ae60,0x6def58,{m.get32(actor+0xa8),0});
    m.set32(actor+0xe4,0);
    U linked=m.get32(actor+0x967);
    if(linked) {
        if(!alternate)m.invoke(0x4637c0,linked,{});
        else m.invoke(0x464030,m.invoke(0x463cb0,0,{}),{actor});
        m.set32(actor+0x967,0);
    }
    for(U offset:{0x9cu,0x778u,0x780u,0x124u})m.set32(actor+offset,0);
    m.set32(actor+0x64c,0xffffffffu);
    if(m.get32(actor+0x798))m.invoke(0x520e70,actor,{});
    if(!m.get32(actor+0x68)) {
        if(m.get32(actor+0x70))m.set32(actor+0x70,0);
    } else {
        m.set32(actor+0x68,0);U count=m.get32(0x6e984c)-1;
        m.set32(0x6e984c,count);m.set32(0x6903e8,count!=0);
        if(m.get32(actor+0x6e8)) {m.invoke(0x4943f0,m.get32(actor+0x6e8),{});m.set32(actor+0x6e8,0);}
        m.invoke(0x512340,actor,{});
        if(leader(m,actor) && !m.get32(actor+0x7c))m.invoke(0x4d3ea0,0x6c4e40,{});
    }
    U index=m.get32(actor),entry=m.get32(0x689498);
    for(U i=0;i<m.get32(0x6898dc);++i,entry+=0x22e)
        if(m.get32(entry+4) && m.get32(entry+0x48)==index && m.get32(entry+0x4c)==50)
            m.invoke(0x4943f0,entry,{});
    index=m.get32(actor);entry=m.get32(0x6def58);
    for(U i=0;i<m.get32(0x6df180);++i,entry+=Stride)
        if(m.get32(entry+4) && m.get32(entry)!=index)invalidate(m,entry,index);
    m.invoke(0x4d9ac0,0x6de8a0,{m.get32(actor)});
    for(U offset:{0x678u,0x67cu})if(m.get32(actor+offset))m.invoke(0x407b90,m.get32(actor+offset),{});
    if(m.get32(actor+0x7cd)==2) {
        if(m.get32(actor+0x7d1))m.invoke(0x4943f0,m.get32(actor+0x7d1),{});
        m.set32(actor+0x7d1,0);
    } else if(m.get32(actor+0x7cd)==3) {
        m.set32(actor+0x7cd,0);m.set32(actor+0x809,0);m.invoke(0x464c80,actor+0x7d5,{});
        if(m.get32(actor+0x805)) {m.invoke(0x4de090,m.get32(actor+0x805),{0,1});m.set32(actor+0x805,0);}
    }
    for(U offset:{0x794u,0x805u,0x120u})if(m.get32(actor+offset)) {
        m.invoke(0x4de090,m.get32(actor+offset),{0,1});m.set32(actor+offset,0);
    }
    if(leader(m,actor) && !m.get32(actor+0x7c)) {
        if(m.get32(actor+0x174)==m.get32(0x644520)) {
            if(!alternate)m.invoke(0x474b10,0x6db967,{m.invoke(0x474b40,0,{})});
            m.invoke(0x509bd0,actor,{0xffffffffu});m.set32(actor+0x56c,0);
            m.invoke(0x472730,0x6cbb78,{});m.set32(0x6a46bc,0);
            m.invoke(0x4d4970,0x6a2dd0,{});m.invoke(0x4d3a30,0x6c4e40,{0});m.set32(0x6c502f,0);
        }
        U network=m.get32(0x689920);
        if(network==1 || network==2)m.invoke(0x49e170,0x689498,{m.get32(actor+0x174)});
    }
    m.set32(actor+0xf4,0);m.set32(actor+0xf0,0);
}

template<class M> void release(M& m,U actor) {
    U index=m.get32(actor),entry=m.get32(0x6def58);
    for(U i=0;i<m.get32(0x6df180);++i,entry+=Stride)
        if(m.get32(entry+4) && m.get32(entry)!=index)invalidate(m,entry,index);
    detach(m,actor,m.get32(actor+8),m.get32(actor+12),m.get32(actor+16));
    if(m.get32(actor+0x618)) {
        U saved=actor+0x97f+m.get32(actor+0x977)*28;
        detach(m,actor,m.get32(saved),m.get32(saved+4),m.get32(saved+8));
    }
    if(m.get32(actor+0x68)) {
        m.set32(actor+0x68,0);U count=m.get32(0x6e984c)-1;
        m.set32(0x6e984c,count);m.set32(0x6903e8,count!=0);m.invoke(0x5121c0,actor,{});
        if(leader(m,actor) && !m.get32(actor+0x7c))m.invoke(0x4d3ea0,0x6c4e40,{});
    }
    m.set32(actor+4,0);
    if(m.get32(actor)+1==m.get32(0x6df180)) {
        U bound=m.get32(0x6df180);m.set32(0x6df180,0);
        while(bound) {--bound;if(m.get32(m.get32(0x6def58)+bound*Stride+4)){m.set32(0x6df180,bound+1);break;}}
    }
    if(leader(m,actor) && !m.get32(actor+0x7c)) {
        if(m.get32(actor+0x174)==m.get32(0x644520)) {
            m.set32(0x6e9854,m.get32(actor+0x178));
            for(U i=0;i<252;++i)m.set32(0x6e9218+i*4,m.get32(actor+0x17c+i*4));
        }
        U owner=m.get32(actor+0x174);
        m.invoke(0x465490,0x643a80+owner*0x154,{0xffffffffu});m.invoke(0x556370,0x689b80,{owner});
        U survivors=0;m.set32(0x644524,0);
        for(U i=0;i<8;++i) {
            m.set32(0x644524,i+1);U candidate=0x643a80+i*0x154;
            if(m.get32(candidate) && m.get32(candidate+12)!=0xffffffffu)++survivors;
        }
        if(survivors<2 && m.get32(0x6e1f0c)) {
            m.set32(0x690618,1);m.set32(0x644568,1);m.set32(0x6e007c,1);
        }
        U type=m.get32(actor+0xa8),slot=type==0?0:type==24?1:type==25?2:3;
        m.set32(0x6e980c+slot*4,0);m.invoke(0x464ad0,0x6dbf00+type*0x114,{});
        U resources=0x6de0e8+type*36;
        if(m.get32(resources)) {
            m.invoke(0x5979b0,0,{m.get32(resources)});
            for(U i=0;i<4;++i)m.set32(resources+i*4,0);
        }
        for(U i=4;i<8;++i)if(m.get32(resources+i*4)) {
            m.invoke(0x57d0d0,0,{m.get32(resources+i*4)});m.set32(resources+i*4,0);
        }
        if(m.get32(resources+32)) {m.invoke(0x5979b0,0,{m.get32(resources+32)});m.set32(resources+32,0);}
        entry=m.get32(0x6def5c)?m.get32(0x6def58):0;
        for(U i=0;signedWord(i)<signedWord(m.get32(0x6df180));++i,entry+=Stride)
            if(entry!=actor && m.get32(entry+4) && m.get32(entry+0xa8)==m.get32(actor+0xa8)) {
                release(m,entry);cleanup(m,entry,0);
            }
    }
    if(m.get32(actor+0x114))m.set32(actor+0x114,0);
    if(m.get32(actor+0x120)) {m.invoke(0x4de090,m.get32(actor+0x120),{0,1});m.set32(actor+0x120,0);}
    m.set32(actor+0xe4,0);m.invoke(0x522340,actor,{});m.invoke(0x56c430,0x6f3a70,{m.get32(actor)});
}
template<class M> U wrapped(U coordinate,U extent) {
    (void)sizeof(M);
    if(signedWord(coordinate)<0) {
        do {coordinate+=extent;}while(signedWord(coordinate)<0);
    } else while(coordinate>=extent)coordinate-=extent;
    return coordinate;
}
template<class M> void baseline(M& m,U actor) {
    for(U offset:{0x618u,0xb43u,0xb3fu,0x97bu,0x977u,0x973u,0x96fu,0x96bu,
        0xb6bu,0xb67u,0xb63u,0xb5fu,0xb5bu,0xb57u,0xb53u,0xb4fu,0xb4bu,0xb47u,
        0xb73u,0xb6fu,0xb77u,0xb7fu,0xb7bu,0x68u})m.set32(actor+offset,0);
    U definition=m.get32(actor+0xac);
    m.set32(actor+0xe4,leader(m,actor)?m.invoke(0x5075f0,actor,{}):m.get32(definition+24));
    if(!m.invoke(0x5075f0,actor,{}))m.set32(actor+0x6c,0);
    else m.set32(actor+0x6c,(m.get32(actor+0xe4)*15)/m.invoke(0x5075f0,actor,{}));
    U bar=m.get32(actor+0x6c);m.set32(actor+0x6c,bar>13?14:bar);
    m.set32(actor+0x64c,0xffffffffu);m.set32(actor+0x108,0);m.set32(actor+0x10c,0);
    m.set32(actor+0x88,m.get32(definition+20));m.set32(actor+0x84,m.get32(definition+40));
    for(U offset:{0x967u,0x15cu,0x963u,0x8e1u,0x8d9u,0x8d5u,0x809u,0x8cdu,0x8c5u,
        0x8c9u,0x794u,0x120u,0x805u,0xde3u,0x889u,0x855u,0x819u,0x81du,0x821u,
        0x798u,0x780u,0x784u,0x790u,0x78cu,0x744u,0xddfu,0x110u,0xcffu,0x80u,
        0x164u,0x168u,0x16cu,0x170u,0x9cu,0x114u,0x660u,0x664u,0x668u,0x66cu,0x670u})m.set32(actor+offset,0);
    for(U offset:{0x962u,0x7ccu,0x71du})m.set8(actor+offset,0);
    m.set32(actor+0x906,0xffffffffu);m.set32(actor+0x8e9,0xffffffffu);
    m.set32(actor+0x160,m.get32(0x6c4830));m.set32(actor+0x118,m.get32(0x6c4830));
    m.set32(actor+0xf8,m.get32(definition+0x58));m.set32(actor+0x104,m.get32(definition+0x3c));
    for(U i=0;i<8;++i)m.set32(actor+0x48+i*4,0);
    for(U offset:{0x6e4u,0x6e8u,0x8cu,0x90u,0x94u,0x7cu,0x70u,0xfcu,0x100u,
        0x778u,0x77cu,0x8d1u,0x6acu,0x6b0u,0x8e5u,0x8edu,0x8f1u,0x8f5u,
        0x90au,0x90eu,0x912u,0x916u,0x91au,0x91eu,0x922u,0x926u,0x92au,0x92eu,
        0x932u,0x95au,0x680u,0x80du,0x684u,0x688u,0x690u,0x694u,0x698u,0x69cu,
        0x6a0u,0x6a4u,0xf4u,0xf0u,0x124u,0x128u})m.set32(actor+offset,0);
    for(U offset=0x936;offset<=0x956;offset+=4)m.set32(actor+offset,100);
    m.set32(actor+0x6a8,0xffffffffu);
    for(U offset:{0x71cu,0x722u,0x723u})m.set8(actor+offset,0);
    m.invoke(0x464c80,actor+0x12c,{});
    for(U offset:{0x7d1u,0x7cdu,0xd03u,0xdcfu})m.set32(actor+offset,0);
    m.set32(actor+0xd07,0xffffffffu);m.set32(actor+0xde7,0xffffffffu);
    m.invoke(0x464ca0,actor+0x825,{0x6dbf00+m.get32(actor+0xa8)*0x114});
    U list=m.get32(actor+0x8f9);
    if(m.get32(list+8)) {
        U sentinel=m.get32(list+4),node=m.get32(sentinel);
        while(node!=sentinel) {
            if(m.get32(node+8)==2 && m.get32(node+40)) {
                m.invoke(0x4943f0,m.get32(node+40),{});m.set32(node+40,0);
            }
            node=m.get32(node);
        }
        m.invoke(0x52d0c0,list,{});
    }
    m.set8(actor+0x901,0);m.set32(actor+0x902,0);
    if(m.get32(actor+0xa0)==2)m.invoke(0x54dba0,actor+0xbf3,{5});
    m.set32(actor+0xa0,0);
    for(U i=0;i<8;++i){m.set32(actor+0xdeb+i*4,0);m.set32(actor+0xe0b+i*4,0);}
    for(U i=0;i<9;++i)m.set32(actor+0x754+i*4,0xffffffffu);
    U group=leader(m,actor)?0:m.invoke(0x466870,0x643a80,{m.get32(actor+0x174)});
    if(group)m.invoke(0x508250,actor,{m.get32(group+8),m.get32(group+12)});
    else {
        m.set32(actor+0x608,3);U direction=(3-2*m.get32(0x6e9850))&7;
        m.set32(actor+0x60c,direction);
        // Earlier zero stores make these branches inactive with pure owned
        // animation providers; real provider mutation remains a separate input.
        if(m.get8(actor+0x722))m.set32(actor+0x60c,(direction-4)&7);
        if(m.get32(actor+0x7cd)==3) {
            U selected=m.get32(0x6898d0);
            m.invoke(0x464ca0,actor+0x7d5,{m.get32(0x6894a0)+m.get32(selected+0x164)*0x114});
            m.invoke(0x464cb0,actor+0x7d5,{m.get32(selected+0x160)+m.get32(actor+0x60c),0});
        } else if(m.get32(actor+0x7cd)==2 && m.get32(actor+0x7d1))
            m.invoke(0x494a50,m.get32(actor+0x7d1),{m.get32(actor+0x60c)});
        if(m.get32(actor+0x855))m.invoke(0x464e20,actor+0x825,{m.get32(actor+0x60c)+8});
    }
    m.invoke(0x51a880,actor,{});
    U x=wrapped<M>(m.get32(actor+8),m.get32(0x6c5494));
    U y=wrapped<M>(m.get32(actor+12),m.get32(0x6c5498));
    if(!m.invoke(0x414890,actor,{x,y,m.get32(actor+16)}))m.set32(actor+0x108,1);
    m.set32(actor+0x5f0,0);m.invoke(0x50ab30,actor,{});
    U gauge=actor+0x6b4;m.invoke(0x58f730,gauge,{});
    U type=m.get32(actor+0xa8),first=m.get32(0x6a5ffc+type*0x5c9),second=m.get32(0x6a600c+type*0x5c9);
    auto absolute=[](U value){return signedWord(value)<0?0u-value:value;};
    m.invoke(0x58f750,gauge,{4,absolute(first)+absolute(second)+1});
    m.invoke(0x58f7f0,gauge,{m.invoke(0x5075f0,actor,{})});
    m.invoke(0x58f830,gauge,{m.get32(actor+0xe4)});m.invoke(0x58f890,gauge,{});
}

template<class M> void activate(M& m,U actor,U type,U owner,U preloaded) {
    m.set32(actor+0x174,owner);U definition=0x6a5f80+type*0x5c9;
    if(!(preloaded&0xff)) {
        U selection=m.invokeFast(0x5057b0,type,owner);
        if(!m.invoke(0x5055a0,definition,{type,selection})) {
            m.invoke(0x5c520c,0,{0,0x5da204,0x5da224,0x42030});
            m.invoke(0x5c5208,0,{m.get32(0x656614)});
        }
    }
    m.set32(actor+0xa8,type);m.set32(actor+0xac,definition);
    m.invoke(0x464ca0,actor+0xb0,{0x6dbf00+type*0x114});m.set32(actor+0x98,0);
    m.set32(actor+0xbbf,m.get32(definition+8)==1?8:64);
    attach(m,actor,m.get32(actor+8),m.get32(actor+12),m.get32(actor+16));
    for(U i=0;i<42;++i)m.set32(actor+0x190+i*24,0);
    for(U offset:{0x178u,0x56cu})m.set32(actor+offset,0);
    m.set32(actor+0x5e8,0xffffffffu);m.set32(actor+4,1);
    if(m.get32(0x6df180)<m.get32(actor)+1)m.set32(0x6df180,m.get32(actor)+1);
    m.set32(actor+0xf8,m.get32(definition+0x58));m.set32(actor+0x74,0);
    m.set32(actor+0x78,0xffffffffu);m.set32(actor+0xec,0);m.set32(actor+0xe8,0);
    m.set32(actor+0x104,m.get32(definition+0x3c));m.set32(actor+0x64c,0xffffffffu);
    m.invoke(0x4e1260,actor+0xb97,{m.get32(definition+12),m.get32(definition+8)==2?1u:0u});
    U inherited=m.get32(definition+0x74);
    if(inherited!=0xffffffffu && inherited!=0 && inherited!=24 && inherited!=25 && inherited!=26 && m.get32(actor+0x56c)<30) {
        U count=m.get32(actor+0x56c);m.set32(actor+0x570+count*4,inherited);m.set32(actor+0x56c,count+1);
        m.set32(0x6b13ed+m.get32(actor+0x174)*36+inherited*0x2d1,1);
    }
    for(U offset:{0x678u,0x67cu,0x704u,0x44u,0x6f0u,0xb8bu,0xa0u,0x8bdu})m.set32(actor+offset,0);
    baseline(m,actor);
    m.set32(actor+0x96b,m.get32(actor+8));m.set32(actor+0x96f,m.get32(actor+12));m.set32(actor+0x973,m.get32(actor+16));
    m.invoke(0x520790,actor,{0x20000000});m.set32(actor+0x92e,1);m.invoke(0x5244f0,actor,{});
    m.invoke(0x416590,0x690600,{actor});
    if(leader(m,actor) && !(preloaded&0xff)) {
        m.invoke(0x416590,0x644550,{actor});m.set8(0x690618,0);m.set8(0x644568,0);
    }
    m.invoke(0x51a8f0,actor,{});m.invoke(0x464ca0,actor+0x12c,{0x657ea8});
    U presentation=actor+0xbf3;m.invoke(0x54ddf0,presentation,{});
    if(m.get32(actor+0xa8)==13) {
        m.invoke(0x54dad0,presentation,{3});m.invoke(0x54dad0,presentation,{14});m.set32(actor+0x8c,100);
    } else {m.invoke(0x54dad0,presentation,{0});m.invoke(0x54dad0,presentation,{5});}
    m.invoke(0x54dcb0,presentation,{0});
    // Positive map extents are an admitted precondition; original zero extents
    // spin in the wrapping loops instead of returning a safe failure.
    for(U offset=0xbcf;offset<=0xbef;offset+=4)m.set32(actor+offset,0);
    m.invoke(0x522340,actor,{});m.invoke(0x51aa30,actor,{});
}
} // namespace mnm::reconstruction::actor_lifecycle

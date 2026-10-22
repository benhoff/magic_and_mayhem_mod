#pragma once

namespace mnm::reconstruction {
// PE32 addresses are values supplied by the memory provider, never host pointers.
// The provider implements r/w/r8/w8, allocate/free/arrayFree, call and scratch.
// call dispatches the explicitly named external engine services; it must not
// dispatch any lifecycle entry back to the original body. Faults are terminal.
template<class Backend> class ActorLifecycle {
    using U = unsigned;
    using I = int;
    Backend& b;
    static constexpr U manager=0x6def58, stride=0xe4b, bound=0x6df180;
    U r(U p) { return b.r(p); }
    void w(U p,U v) { b.w(p,v); }
    template<class... A> U call(U entry,U receiver,A... args) {
        return b.call(entry,receiver,args...);
    }
    void words(U p,U count,U value=0) { for(U i=0;i<count;++i)w(p+4*i,value); }
    static bool wizard(U kind) { return kind==0 || kind==24 || kind==25 || kind==26; }
    U army(U id) { return id==~0u?0:0x643a80+id*0x154; }
    U wrap(U value,U divisor) {
        if(!divisor)b.fault("Zero coordinate divisor");
        if(I(value)<0) {do {value+=divisor;}while(I(value)<0);}
        else while(value>=divisor)value-=divisor;
        return value;
    }
    void selectionRemoved(U p,bool releasing) {
        if(!r(p+0x68)) return;
        w(p+0x68,0);
        const U count=r(0x6e984c)-1;
        w(0x6e984c,count);w(0x6903e8,count!=0);
        if(!releasing && r(p+0x6e8)) {
            call(0x4943f0,r(p+0x6e8));w(p+0x6e8,0);
        }
        call(releasing?0x5121c0:0x512340,p);
        if(wizard(r(r(p+0xac))) && !r(p+0x7c))call(0x4d3ea0,0x6c4e40);
    }
    void invalidatePeers(U p) {
        const U id=r(p),base=r(manager);
        for(U i=0;i<r(bound);++i) {
            const U peer=base+i*stride;
            if(r(peer+4) && r(peer)!=id)call(0x508b00,peer,id);
        }
    }
    void stop(U p,U offset) {
        if(r(p+offset)) { call(0x4de090,r(p+offset),0u,1u);w(p+offset,0); }
    }
    void erase(U list,U node) {
        const U next=r(node),previous=r(node+4);
        w(previous,next);w(next+4,previous);b.free(node);w(list+8,r(list+8)-1);
    }
    void clearList(U list) {
        const U head=r(list+4);
        for(U node=r(head);node!=head;) {
            const U next=r(node);erase(list,node);node=next;
        }
    }
    void destroyOwned(U p) {
        if(const U callback=r(p+0x708)) {
            // 52cbe0/52cc00: dynamic callback deleting destructor.
            call(r(r(callback)+4),callback,1u);w(p+0x708,0);
        }
        if(const U list=r(p+0x8f9)) {
            clearList(list);b.free(r(list+4));w(list+4,0);w(list+8,0);
            b.free(list);w(p+0x8f9,0);
        }
    }
    void resetManagerFields(U m) {
        words(m+0x22c,15);w(m+8,0);w(m+12,0);words(m+0x20,28);
        // Neither original prepare entry resets +228 (the high-water bound).
    }
public:
    explicit ActorLifecycle(Backend& backend):b(backend) {}

    // 505db0. Deliberately leave constructor-untouched allocator bytes intact.
    void construct(U p) {
        words(p,9);
        const U animations[]={0xb0,0x12c,0x79c,0x7d5,0x825,0x859,0x88d};
        for(U at:animations)words(p+at,12);
        words(p+0x17c,42*6);w(p+0x640,1);w(p+0x644,~0u);w(p+0x648,0);
        words(p+0x6d0,5);histogramInit(p+0x6b4);
        for(U i=0;i<16;++i) {
            const U at=p+0x97f+28*i;
            words(at,4);w(at+20,0);w(at+24,5); // +10 is untouched.
        }
        const U zero[]={0xb43u,0xb3fu,0x97bu,0x977u,0x973u,0x96fu,0x96bu,
                  0xb6bu,0xb67u,0xb63u,0xb5fu,0xb5bu,0xb57u,0xb53u,
                  0xb4fu,0xb4bu,0xb47u,0xb73u,0xb6fu,0xbb7u,0xbcfu,0xbd3u,0xbd7u};
        for(U at:zero)w(p+at,0);
        words(p+0xbf3,2);words(p+0xbfb,30,~0u);w(p+0xc73,0);
        words(p+0xc77,30,~0u);words(p+0xcef,4);
        w(p+0x708,0);w(p+0x8f9,0);
    }
    void histogramInit(U p) {
        w(p,5);w(p+4,40);words(p+8,3);w(p+20,100);
    }
    // 505f70. Does not release a logical actor or free its containing record.
    void destruct(U p) {
        destroyOwned(p);
        // 54dac0 is an empty destructor. 58f6f0 releases the histogram array.
        if(r(p+0x6d0)) {
            b.arrayFree(r(p+0x6d0));w(p+0x6d0,0);w(p+0x6d4,0);
        }
    }
    // 5060a0, including the original byte-sized list allocator tag.
    void reset(U p,U index) {
        destroyOwned(p);
        for(U i=0;i<stride;++i)b.w8(p+i,0);
        const U callback=b.allocate(16);
        if(callback) {
            w(callback+4,0x51e890);w(callback+8,0);w(callback+12,p);w(callback,0x5c7264);
        }
        w(p+0x708,callback);w(p+0xcf3,callback);
        const U list=b.allocate(12);b.w8(list,static_cast<unsigned char>(index));
        const U head=b.allocate(48);w(head,head);w(head+4,head);
        w(list+4,head);w(list+8,0);w(p+0x8f9,list);w(p,index);w(p+0x10c,5);
    }
    void resetAll(U m=manager) {
        const U base=r(m);
        for(U i=0;i<r(m+4);++i)reset(base+i*stride,i);
        if(r(m+0x98)) {
            const U list=m+0x90,head=r(list+4),end=r(r(head+4));
            for(U node=r(head);node!=end;) {
                const U next=r(node);erase(list,node);node=next;
            }
        }
    }
    // 473100. Allocation count prefix is used for reverse-order destruction.
    void prepare(U m=manager) {
        if(!r(m)) {
            const U count=r(m+4),allocation=b.allocate(count*stride+4);
            if(!allocation) { w(m,0);return; }
            w(allocation,count);
            for(U i=0;i<count;++i)construct(allocation+4+i*stride);
            w(m,allocation+4);
        }
        words(0x6e9848,3);resetAll(m);resetManagerFields(m);
    }
    // 4bfef0: list and eight animation handles precede array preparation.
    U constructPool(U m,U count) {
        b.w8(m+0x90,static_cast<unsigned char>(count));
        const U head=b.allocate(12);w(head,head);w(head+4,head);
        w(m+0x94,head);w(m+0x98,0);
        for(U i=0;i<8;++i)words(m+0xa4+i*48,12);
        w(m,0);w(m+4,count);prepare(m);return m;
    }
    // 4c00d0: array elements are destructed last-to-first, then manager list.
    void destroyPool(U m=manager) {
        if(U base=r(m)) {
            U count=r(base-4);
            while(count)destruct(base+(--count)*stride);
            b.free(base-4);w(m,0);
        }
        clearList(m+0x90);b.free(r(m+0x94));w(m+0x94,0);w(m+0x98,0);
    }
    U firstFree(U m=manager) {
        U p=r(m);
        for(U i=0;i<r(m+4);++i,p+=stride)if(!r(p+4))return p;
        return 0; // Finding a free slot does not reserve or activate it.
    }

    void baseline(U p);
    void activate(U p,U type,U owner,unsigned char loaded);
    void cleanup(U p,unsigned char quiet);
    void release(U p);
    void releasePresentation(U m=manager);
};

template<class B> void ActorLifecycle<B>::activate(U p,U type,U owner,unsigned char loaded) {
    w(p+0x174,owner);
    if(!loaded) {
        const U animation=b.fastcall(0x5057b0,type,owner);
        if(!call(0x5055a0,0x6a5f80+type*0x5c9,type,animation))b.loadError();
    }
    w(p+0xa8,type);w(p+0xac,0x6a5f80+type*0x5c9);
    call(0x464ca0,p+0xb0,0x6dbf00+r(p+0xa8)*0x114);
    w(p+0x98,0);w(p+0xbbf,r(r(p+0xac)+8)==1?8:64);
    call(0x509230,p,r(p+8),r(p+12),r(p+16));
    for(U i=0;i<42;++i)w(p+0x190+24*i,0);
    w(p+0x178,0);w(p+0x56c,0);w(p+0x5e8,~0u);w(p+4,1);
    if(I(r(p)+1)>I(r(bound)))w(bound,r(p)+1);
    w(p+0xf8,r(r(p+0xac)+0x58));w(p+0x74,0);w(p+0x78,~0u);
    w(p+0xec,0);w(p+0xe8,0);w(p+0x64c,~0u);w(p+0x104,r(r(p+0xac)+0x3c));
    call(0x4e1260,p+0xb97,r(r(p+0xac)+12),U(r(r(p+0xac)+8)==2));
    const U spell=r(r(p+0xac)+0x74);
    if(spell!=~0u && !wizard(spell) && r(p+0x56c)<30) {
        const U count=r(p+0x56c);w(p+0x570+count*4,spell);w(p+0x56c,count+1);
        w(0x6b13ed+r(p+0x174)*0x24+spell*0x2d1,1);
    }
    const U zero[]={0x678,0x67c,0x704,0x44,0x6f0,0xb8b,0xa0,0x8bd};
    for(U at:zero)w(p+at,0);
    baseline(p);
    w(p+0x96b,r(p+8));w(p+0x96f,r(p+12));w(p+0x973,r(p+16));
    call(0x520790,p,b.scratch());w(p+0x92e,1);call(0x5244f0,p);
    call(0x416590,0x690600,p);
    if(wizard(r(r(p+0xac))) && !loaded) {
        call(0x416590,0x644550,p);b.w8(0x690618,0);b.w8(0x644568,0);
    }
    call(0x51a8f0,p);call(0x464ca0,p+0x12c,0x657ea8);call(0x54ddf0,p+0xbf3);
    if(r(p+0xa8)==13) {
        call(0x54dad0,p+0xbf3,3u);call(0x54dad0,p+0xbf3,14u);w(p+0x8c,100);
    } else { call(0x54dad0,p+0xbf3,0u);call(0x54dad0,p+0xbf3,5u); }
    call(0x54dcb0,p+0xbf3,0u);
    // Zero map divisors spin forever in the original modulo loops. The native
    // provider rejects this invalid context instead of hanging a host thread.
    if(!r(0x6c5494) || !r(0x6c5498))b.fault("Zero activation map divisor");
    words(p+0xbcf,9);call(0x522340,p);call(0x51aa30,p);
}

template<class B> void ActorLifecycle<B>::release(U p) {
    invalidatePeers(p);
    call(0x5093e0,p,r(p+8),r(p+12),r(p+16));
    if(r(p+0x618)) {
        const U saved=p+0x97f+r(p+0x977)*28;
        call(0x5093e0,p,r(saved),r(saved+4),r(saved+8));
    }
    selectionRemoved(p,true);
    w(p+4,0);
    if(r(p)+1==r(bound)) {
        U i=r(bound),base=r(manager);w(bound,0);
        while(i) { --i;if(r(base+i*stride+4)) {w(bound,i+1);break;} }
    }
    if(wizard(r(r(p+0xac))) && !r(p+0x7c)) {
        if(r(p+0x174)==r(0x644520)) {
            w(0x6e9854,r(p+0x178));
            for(U i=0;i<252;++i)w(0x6e9218+4*i,r(p+0x17c+4*i));
        }
        call(0x465490,army(r(p+0x174)),~0u);call(0x556370,0x689b80,r(p+0x174));
        U active=0;w(0x644524,0);
        while(I(r(0x644524))<8) {
            const U id=r(0x644524),a=army(id);w(0x644524,id+1);
            if(r(a) && r(a+12)!=~0u)++active;
        }
        if(active<2 && r(0x6e1f0c)) {
            b.w8(0x690618,1);b.w8(0x644568,1);w(0x6e007c,1);
        }
        const U type=r(p+0xa8);
        // The original default case uses an uninitialized stack value. Only
        // the four classified wizard types have a defined ownership contract.
        if(!wizard(type))b.fault("Unclassified wizard resource index");
        w(0x6e980c+(type?type-23:0)*4,0);
        call(0x464ad0,0x6dbf00+type*0x114);
        const U resources=0x6de0e8+r(p+0xa8)*0x24;
        if(r(resources)) {b.arrayFree(r(resources));words(resources,4);}
        for(U i=0;i<4;++i) {
            const U at=resources+16+4*i;
            if(r(at)) {call(0x57d0d0,resources,r(at));w(at,0);}
        }
        if(r(resources+32)) {b.arrayFree(r(resources+32));w(resources+32,0);}
        const U base=r(manager+4)?r(manager):0;
        // Reload the bound after each recursive release and each cleanup.
        for(U i=0;I(i)<I(r(bound));++i) {
            const U peer=base+i*stride;
            if(peer!=p && r(peer+4) && r(peer+0xa8)==r(p+0xa8)) {
                release(peer);cleanup(peer,0);
            }
        }
    }
    if(r(p+0x114))w(p+0x114,0);
    stop(p,0x120);w(p+0xe4,0);call(0x522340,p);call(0x56c430,0x6f3a70,r(p));
}

template<class B> void ActorLifecycle<B>::cleanup(U p,unsigned char quiet) {
    w(p+0x889,0);w(p+0x8bd,0);
    const bool special=wizard(r(r(p+0xac))) && !r(p+0x7c);
    if(!quiet) {
        if(special) {
            const U mode=r(0x689920);
            if((mode==1 || mode==2) && b.r8(0x6dbe93)) {
                call(0x4da720,0x6a26c8,0x658f10+r(p+0x174)*0x133,6u);
            } else if(mode!=1 && mode!=2 && b.r8(0x6dbe93) && r(p+0x174)==r(0x644520)) {
                call(0x4da690,0x6a26c8,5u);
            }
            call(0x49d640,0x689498,p);
        }
        call(0x5137d0,p);
    }
    if(r(p+0x664) && (r(p+0x5fc)==10 || r(p+0x5fc)==20)) {
        const U index=r(p+(r(p+0x5fc)==10?0x64c:0x6a8));
        const U target=index<r(manager+4)?r(manager)+index*stride:0;
        w(p+0x664,0);call(0x50eb60,target,0u);
    }
    if(r(p+0x124)==2)call(0x52ae60,manager,r(p+0xa8),0u);
    w(p+0xe4,0);
    if(r(p+0x967)) {
        if(!quiet)call(0x4637c0,r(p+0x967));
        else {const U node=call(0x463cb0,0);call(0x464030,node,p);}
        w(p+0x967,0);
    }
    w(p+0x9c,0);w(p+0x778,0);w(p+0x64c,~0u);
    if(r(p+0x798))call(0x520e70,p);
    w(p+0x780,0);w(p+0x124,0);
    if(!r(p+0x68)) {if(r(p+0x70))w(p+0x70,0);}
    else selectionRemoved(p,false);
    const U id=r(p),secondary=r(0x689498);
    for(U i=0;i<r(0x6898dc);++i) {
        const U other=secondary+i*0x22e;
        if(r(other+4) && r(other+0x48)==id && r(other+0x4c)==50)call(0x4943f0,other);
    }
    invalidatePeers(p);call(0x4d9ac0,0x6de8a0,r(p));
    for(U i=0;i<2;++i)if(r(p+0x678+4*i))call(0x407b90,r(p+0x678+4*i));
    if(r(p+0x7cd)==2) {
        if(r(p+0x7d1))call(0x4943f0,r(p+0x7d1));
        w(p+0x7d1,0);
    } else if(r(p+0x7cd)==3) {
        w(p+0x7cd,0);w(p+0x809,0);call(0x464c80,p+0x7d5);stop(p,0x805);
    }
    stop(p,0x794);stop(p,0x805);stop(p,0x120);
    if(wizard(r(r(p+0xac))) && !r(p+0x7c)) {
        if(r(p+0x174)==r(0x644520)) {
            if(!quiet) {const U state=call(0x474b40,0u);call(0x474b10,0x6db967,state);}
            call(0x509bd0,p,~0u);w(p+0x56c,0);call(0x472730,0x6cbb78);
            w(0x6a46bc,0);call(0x4d4970,0x6a2dd0);call(0x4d3a30,0x6c4e40,0u);w(0x6c502f,0);
        }
        if(r(0x689920)==1 || r(0x689920)==2)call(0x49e170,0x689498,r(p+0x174));
    }
    w(p+0xf4,0);w(p+0xf0,0);
}

template<class B> void ActorLifecycle<B>::releasePresentation(U m) {
    const U base=r(m);
    for(U i=0;i<r(m+4);++i) {
        const U p=base+i*stride;
        const U offsets[]={0x794,0x805,0x120};
        for(U offset:offsets) {
            if(!r(p+offset))continue;
            if(offset==0x120)stop(p,offset);
            else {
                const U voice=r(p+offset),audio=r(0x6f437c);
                if(r(audio+0x1c) && r(audio+0x18)) {
                    bool dispose=false;
                    if(offset==0x794) {
                        if(r(voice)) {
                            w(b.scratch(),0);
                            dispose=b.stdcall(r(r(r(voice))+0x24),r(voice),b.scratch())!=0 || (r(b.scratch())&1);
                        }
                    } else dispose=call(0x49c530,voice,0u)!=0;
                    if(dispose) {
                        U result=b.stdcall(r(r(r(voice))+0x48),r(voice));
                        w(0x6fa798,result);
                        const U list=r(0x6f437c)+0x25c,iterator=call(0x49bcf0,list);
                        call(0x571d20,list,iterator,voice);
                        if(!r(0x6fa798)) {
                            result=b.stdcall(r(r(r(voice))+0x34),r(voice),0u);w(0x6fa798,result);
                        }
                    }
                }
                w(p+offset,0);
            }
        }
    }
}

template<class B> void ActorLifecycle<B>::baseline(U p) {
    const U early[]={0x618,0xb43,0xb3f,0x97b,0x977,0x973,0x96f,0x96b,
        0xb6b,0xb67,0xb63,0xb5f,0xb5b,0xb57,0xb53,0xb4f,0xb4b,0xb47,
        0xb73,0xb6f,0xb77,0xb7f,0xb7b,0x68};
    for(U at:early)w(p+at,0);
    w(p+0xe4,wizard(r(r(p+0xac)))?call(0x5075f0,p):r(r(p+0xac)+24));
    U health=0;
    if(call(0x5075f0,p))health=(r(p+0xe4)*15)/call(0x5075f0,p);
    w(p+0x6c,health>13?14:health);w(p+0x64c,~0u);
    w(p+0x108,0);w(p+0x10c,0);w(p+0x88,r(r(p+0xac)+20));w(p+0x84,r(r(p+0xac)+40));
    const U zero[]={0x967,0x15c,0x963,0x8e1,0x8d9,0x8d5,0x809,0x8cd,
        0x8c5,0x8c9,0x794,0x120,0x805,0xde3,0x889,0x855,0x819,0x81d,0x821,
        0x798,0x780,0x784,0x790,0x78c,0x744,0xddf,0x110,0xcff,0x80,
        0x164,0x168,0x16c,0x170,0x9c,0x114,0x660,0x664,0x668,0x66c,0x670,
        0x6e4,0x6e8,0x8c,0x90,0x94,0x7c,0x70,0xfc,0x100,0x778,0x77c,
        0x8d1,0x6ac,0x6b0,0x8e5,0x8ed,0x8f1,0x8f5,0x90a,0x90e,0x912,
        0x916,0x91a,0x91e,0x922,0x926,0x92a,0x92e,0x95a,0x680,0x80d,
        0x684,0x688,0x690,0x694,0x698,0x69c,0x6a0,0x6a4,0xf4,0xf0,0x124,0x128};
    for(U at:zero)w(p+at,0);
    const U bytes[]={0x962,0x7cc,0x71d,0x71c,0x722,0x723};
    for(U at:bytes)b.w8(p+at,0);
    w(p+0x906,~0u);w(p+0x8e9,~0u);w(p+0x6a8,~0u);
    w(p+0x160,r(0x6c4830));w(p+0x118,r(0x6c4830));
    w(p+0xf8,r(r(p+0xac)+0x58));w(p+0x104,r(r(p+0xac)+0x3c));words(p+0x48,8);
    // +932 is cleared too; +936..+956 are nine independent defaults.
    w(p+0x932,0);words(p+0x936,9,100);
    call(0x464c80,p+0x12c);
    w(p+0x7d1,0);w(p+0x7cd,0);w(p+0xd03,0);w(p+0xdcf,0);
    w(p+0xd07,~0u);w(p+0xde7,~0u);
    call(0x464ca0,p+0x825,0x6dbf00+r(p+0xa8)*0x114);
    if(r(r(p+0x8f9)+8)) {
        const U list=r(p+0x8f9);
        for(U node=r(r(list+4));node!=r(r(p+0x8f9)+4);node=r(node)) {
            if(r(node+8)==2 && r(node+40)) {call(0x4943f0,r(node+40));w(node+40,0);}
        }
        clearList(r(p+0x8f9));
    }
    b.w8(p+0x901,0);w(p+0x902,0);
    if(r(p+0xa0)==2)call(0x54dba0,p+0xbf3,5u);
    w(p+0xa0,0);words(p+0xdeb,8);words(p+0xe0b,8);words(p+0x754,9,~0u);
    bool facing=false;
    if(!wizard(r(r(p+0xac)))) {
        const U leader=call(0x466870,0x643a80,r(p+0x174));
        if(leader) {call(0x508250,p,r(leader+8),r(leader+12));facing=true;}
    }
    if(!facing) {
        w(p+0x608,3);w(p+0x60c,(3-2*r(0x6e9850))&7);
        if(b.r8(p+0x722))w(p+0x60c,(r(p+0x60c)-4)&7);
        if(r(p+0x7cd)==3) {
            const U definition=r(0x6898d0)+0x160;
            call(0x464ca0,p+0x7d5,r(0x6894a0)+r(definition+4)*0x114);
            call(0x464cb0,p+0x7d5,r(definition)+r(p+0x60c),0u);
        } else if(r(p+0x7cd)==2 && r(p+0x7d1))call(0x494a50,r(p+0x7d1),r(p+0x60c));
        if(r(p+0x855))call(0x464e20,p+0x825,r(p+0x60c)+8);
    }
    call(0x51a880,p);
    // 40e290/40e8a0 copy already-normalized tile coordinates into value
    // wrappers, applying the configured unsigned modulo for out-of-range input.
    U x=r(p+8),y=r(p+12);
    x=wrap(x,r(0x6c5494));y=wrap(y,r(0x6c5498));
    if(!call(0x414890,p,x,y,r(p+16)))w(p+0x108,1);
    w(p+0x5f0,0);call(0x50ab30,p);histogramInit(p+0x6b4);
    const U def=0x6a5f80+r(p+0xa8)*0x5c9;
    const U a=r(def+0x7c),as=U(I(a)>>31),c=r(def+0x8c),cs=U(I(c)>>31);
    call(0x58f750,p+0x6b4,4u,((a^as)-as)+1+((c^cs)-cs));
    const U maximum=call(0x5075f0,p);
    call(0x58f7f0,p+0x6b4,maximum);call(0x58f830,p+0x6b4,r(p+0xe4));call(0x58f890,p+0x6b4);
}
}

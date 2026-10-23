#pragma once
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
namespace mnm::reconstruction::combat {
// Pinned NoCD task-owned source snapshots. Storage uses checked PE32 values;
// actors/snapshots at +35 are borrowed. Normal bounded acyclic forests only.
template<class M> class OwnerTaskSources {
    using U=std::uint32_t;
    M& m;
    U r(U p){return m.get32(p);} void w(U p,U v){m.set32(p,v);}
    U allocate(U n){U p=m.allocate(n);if(!p)throw std::runtime_error("source nested allocation");return p;}
    static int signedWord(U v){return v<=0x7fffffffu?int(v):-1-int(~v);}
    void depth(unsigned n){if(n>=128)throw std::invalid_argument("source forest depth");}
    // 42a340, restricted to one nonaliased value appended at end. Growth is
    // size+max(size,1), independent of the previous reserved capacity.
    void append(U h,U value){
        U start=r(h+4),end=r(h+8),limit=r(h+12);
        if(end==limit){U size=start?(end-start)/4:0,cap=size+(size?size:1);
            U p=allocate(cap*4);for(U i=0;i<size;++i)w(p+i*4,r(start+i*4));
            w(p+size*4,value);m.free(start);w(h+4,p);w(h+8,p+(size+1)*4);w(h+12,p+cap*4);
        }else{w(end,value);w(h+8,end+4);}
    }
    U source(U task,U actor,bool actorFactory=false){
        U p=allocate(0x8d);w(p,~U(0));m.set8(p+4,0);
        for(U o:{0xdu,0x11u,0x15u,0x29u,0x2du,0x31u,0x39u,0x3du,5u,0x1du})w(p+o,0);
        // Anonymous factory's byte25 is undefined stack data. Native zero is
        // explicit representation; actor factory copies its argument's low byte.
        m.set8(p+0x25,actor&255);w(p+0x35,actor);
        w(p+0x41,0x01010101);w(p+0x45,0x01010101);
        if(actorFactory){w(p+0x89,3);w(p+9,~U(0));}
        append(task+0x37,p);return p;
    }
    U begin(U p,unsigned d){
        depth(d);U slot=0;while(slot<8&&!m.get8(p+0x41+slot))++slot;
        if(slot==8){slot=r(p+0x3d);if(slot>=8)throw std::invalid_argument("source cursor");w(p+0x3d,(slot+1)&7);}
        U start=r(p+0x29);w(p+0x49+slot*4,start);
        w(p+0x69+slot*4,start&&start!=r(p+0x2d)?begin(r(start),d+1):0);
        m.set8(p+0x41+slot,0);return slot;
    }
    U next(U p,U slot,int mode,unsigned d){
        depth(d);if(slot>=8)throw std::invalid_argument("source cursor");
        if(U actor=r(p+0x35)){
            if(signedWord(r(p+0x89))>=mode&&!r(p+0x69+slot*4)){w(p+0x69+slot*4,1);return actor;}
        }else{
            while(r(p+0x49+slot*4)!=r(p+0x2d)){
                U at=r(p+0x49+slot*4);U actor=next(r(at),r(p+0x69+slot*4),mode,d+1);if(actor)return actor;
                at+=4;w(p+0x49+slot*4,at);if(at!=r(p+0x2d))w(p+0x69+slot*4,begin(r(at),d+1));
            }
        }
        m.set8(p+0x41+slot,1);return 0;
    }
    U copy(U task,U p,unsigned d){
        depth(d);U slot=begin(p,0),out;
        if(r(p+0x35))out=source(task,next(p,slot,signedWord(r(0x5d854c)),0),true);
        else{
            out=source(task,0);
            for(;;){U at=r(p+0x49+slot*4),child=0;
                while(at!=r(p+0x2d)){
                    child=r(at);at+=4;w(p+0x49+slot*4,at);
                    if(!child||!r(child+0x35)||signedWord(r(child+0x89))>=signedWord(r(0x5d854c)))break;
                    child=0;
                }
                if(!child){m.set8(p+0x41+slot,1);break;}
                attach(out,copy(task,child,d+1));
            }
        }
        for(U o:{9u,0xdu,0x11u,0x15u,0x19u,0x1du,0x21u})w(out+o,r(p+o));
        return out;
    }
public:
    explicit OwnerTaskSources(M& memory):m(memory){}
    U beginTraversal(U root){return begin(root,0);}
    U nextActor(U root,U cursor){return next(root,cursor,signedWord(r(0x5d854c)),0);}
    U drain(U root,U mode){
        U saved=r(0x5d854c);w(0x5d854c,mode);U cursor=begin(root,0),first=nextActor(root,cursor);
        while(nextActor(root,cursor)){}w(0x5d854c,saved);return first;
    }
    U anonymous(U task){return source(task,0);}
    U actor(U task,U borrowed){if(!borrowed)throw std::invalid_argument("borrowed actor");return source(task,borrowed,true);}
    U clone(U task,U root){return copy(task,root,0);}
    void attach(U parent,U child){
        if(r(parent+0x35)||r(child+0x39))return;
        U before=r(parent+0x29);append(parent+0x25,child);U shift=r(parent+0x29)-before;
        for(U i=0;i<8;++i)if(!m.get8(parent+0x41+i))w(parent+0x49+i*4,r(parent+0x49+i*4)+shift);
        w(child+0x39,parent);
    }
    // 407650 initializes traversal, detaches immediate eligible children and
    // the first parent occurrence, restores mode, then releases child backing.
    // The outer 418050 caller frees the source object separately.
    void destroy(U p){
        U mode=r(0x5d854c);w(0x5d854c,0);U slot=begin(p,0);
        for(;;){U at=r(p+0x49+slot*4),child=0;
            while(at!=r(p+0x2d)){
                child=r(at);at+=4;w(p+0x49+slot*4,at);
                if(!child||!r(child+0x35)||signedWord(r(child+0x89))>=0)break;
                child=0;
            }
            if(!child){m.set8(p+0x41+slot,1);break;}w(child+0x39,0);
        }
        if(U parent=r(p+0x39)){
            if(!r(parent+0x35)){
                U end=r(parent+0x2d),at=r(parent+0x29);while(at!=end&&r(at)!=p)at+=4;
                if(at!=end){for(U q=at+4;q!=end;q+=4)w(q-4,r(q));w(parent+0x2d,end-4);
                    for(U i=0;i<8;++i)if(!m.get8(parent+0x41+i)&&at<r(parent+0x49+i*4))w(parent+0x49+i*4,r(parent+0x49+i*4)-4);
                    w(p+0x39,0);
                }
            }
        }
        w(0x5d854c,mode);m.free(r(p+0x29));w(p+0x29,0);w(p+0x2d,0);w(p+0x31,0);
    }
};
}

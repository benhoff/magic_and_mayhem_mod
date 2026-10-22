#include "../reconstruction/rendering/actor_services.hpp"
#include <vector>
#include <cassert>
using U=mnm::reconstruction::actor_services::U;
struct Failure{unsigned index;};
struct Backend {
    unsigned failAt{},attempts{};std::vector<U> constructed,destroyed;
    U invoke(U id,U self,std::initializer_list<U>){
        if(id==1){if(attempts++==failAt)throw Failure{failAt};constructed.push_back(self);}
        else if(id==2)destroyed.push_back(self);else assert(false);
        return 0;
    }
    U get32(U){assert(false);return 0;}U get16(U){assert(false);return 0;}U get8(U){assert(false);return 0;}
    void set32(U,U){assert(false);}void set16(U,U){assert(false);}void set8(U,U){assert(false);}
    U fastcall(U,U,U,std::initializer_list<U>){assert(false);return 0;}
    U temporary(U){assert(false);return 0;}U com(U,U,std::initializer_list<U>){assert(false);return 0;}
};
int main(){
    for(unsigned count=0;count<18;++count)for(unsigned fail=0;fail<=count;++fail){
        Backend b;b.failAt=fail;bool thrown=false;
        try{mnm::reconstruction::actor_services::Services<Backend>(b).run(0x59c441,0,{0x1000,151,count,1,2});}
        catch(const Failure& f){assert(f.index==fail);thrown=true;}
        assert(thrown==(fail<count));assert(b.constructed.size()==(thrown?fail:count));
        if(thrown){assert(b.destroyed.size()==fail);for(unsigned i=0;i<fail;++i)assert(b.destroyed[i]==b.constructed[fail-i-1]);}
        else assert(b.destroyed.empty());
    }
}

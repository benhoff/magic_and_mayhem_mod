#include "placement.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main()try{
    mnm::assets::AnimationRecord record{0,0,{std::uint32_t(-12),std::uint32_t(-50),0,0,99,6,std::uint32_t(-56),1,std::uint32_t(-23)}};
    require(spriteOffset(record).x==-12 && spriteOffset(record).y==-50,"Sprite fields differ");
    require(attachmentOffset(record,AttachmentPoint::first).x==6 && attachmentOffset(record,AttachmentPoint::second).y==-23,"Attachment fields differ");
    require(spriteOffset(record,2,1).x==20 && spriteOffset(record,2,1).y==-66,"View 1 differs");
    require(spriteOffset(record,2,2).y==-82 && spriteOffset(record,2,3).x==-44,"View 2/3 differs");
    require(spriteOffset(record,1,1).x==-12 && spriteOffset(record,2,4).y==-50,"Adjustment applied outside original conditions");
    record.metadata[0]=0x7fffffff;record.metadata[1]=0x80000000;
    require(spriteOffset(record,2,1).x==std::numeric_limits<std::int32_t>::min()+31 && spriteOffset(record,2,1).y==std::numeric_limits<std::int32_t>::max()-15,"x86 word wrapping differs");
    record.opcode=6;bool rejected=false;try{spriteOffset(record);}catch(const std::runtime_error&){rejected=true;}require(rejected,"Stop interpreted as sprite placement");
    std::cout<<"ANI sprite/attachment fields, view adjustment and x86 wrap pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

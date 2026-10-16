#include "../apps/command-session/native_regional_danger.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include <iostream>
namespace cs=mnm::command_session;
int main(int argc,char **argv)try{
  if(argc!=2)throw std::runtime_error("INSTALL");
  auto p=mnm::encounter::decodeInputs(mnm::encounter::loadInputs(argv[1],{0,0,32,32},0));
  mnm::encounter::enableRegionalNavigation(p);mnm::encounter::enableOwnerKnowledge(p);
  auto c=p.initial, restored=c;
  unsigned checks=0,restores=0,refusals=0,expiryFrames=0;
  auto check=[&](bool v,const char *why){if(!v)throw std::runtime_error(why);++checks;};
  for(unsigned tick=1;tick<=2304;++tick){
    for(auto *v:{&c,&restored}){
      v->native.tick=tick;
      v->world.creatures[1].ai.visible[0]=(tick<64||tick>=1408);
      if(tick==1792)v->world.creatures[1].ai.health=0;
      cs::updateNativeRegionalDanger(*v,p.native);
    }
    const auto bytes=cs::encode(c);
    check(bytes.at(4)==10,"knowledge version");
    check(bytes==cs::encode(restored),"restored knowledge differs");
    if(tick%8==0){restored=cs::decode(bytes);++restores;}
    if(tick>1152&&tick<1408){check(c.native.ownerKnowledge->records[0].empty(),"expired threat retained");++expiryFrames;}
  }
  auto reject=[&](auto change){auto bad=c;change(bad);bool failed=false;try{(void)cs::encode(bad);}catch(const std::invalid_argument &){failed=true;}check(failed,"malformed knowledge accepted");++refusals;};
  reject([](auto &v){v.native.ownerKnowledge->owner=8;});
  reject([](auto &v){v.native.ownerKnowledge->cursor=123456;});
  reject([](auto &v){v.native.ownerKnowledge->nextKey[0]=-1;});
  reject([](auto &v){v.native.ownerKnowledge->records[0].push_back({});});
  reject([](auto &v){v.native.regionalDanger.reset();});
  reject([](auto &v){v.native.regionalDanger->inputs.memories[0].clear();v.native.ownerKnowledge->records[0].push_back({});});
  auto legacy=c;legacy.native.ownerKnowledge.reset();
  // An absent feature must retain the old bytes and old observation policy.
  for(unsigned owner=0;owner<8;++owner)legacy.native.regionalDanger->inputs.memories[owner].clear();
  auto old=cs::encode(legacy,9);auto decoded=cs::decode(old);
  check(!decoded.native.ownerKnowledge&&cs::encode(decoded,9)==old,"v9 compatibility");
  bool failed=false;try{(void)cs::encode(c,9);}catch(const std::invalid_argument &){failed=true;}
  check(failed,"lossy legacy knowledge write");++refusals;
  const auto before=cs::encode(c);failed=false;
  try{cs::prepareNativeOwnerKnowledge(c);}catch(const std::invalid_argument &){failed=true;}
  check(failed&&before==cs::encode(c),"failed configuration published state");++refusals;
  check(expiryFrames>0,"missing expiry interval");
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"restores\":"<<restores<<",\"refusals\":"<<refusals<<",\"expiry_frames\":"<<expiryFrames<<"}\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}

#include "animation_binding.hpp"
#include "resource-fixtures.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <thread>
using namespace mnm;using namespace resource_test;
using Op=scene::SceneAnimationCommand::Operation;
static Bytes directionalAnimation(){
 constexpr unsigned count=32,records=count*4,base=44+(count+1)*4;Bytes bytes(base+records*44,0);word(bytes,0,0x00494e41);word(bytes,4,bytes.size());word(bytes,8,records);word(bytes,12,5);word(bytes,20,count+1);
 for(unsigned i=0;i<=count;++i)word(bytes,44+i*4,i*4);
 for(unsigned i=0;i<count;++i){const auto at=base+i*4*44;word(bytes,at,1);word(bytes,at+4,2);word(bytes,at+44,0);word(bytes,at+48,i%2);word(bytes,at+88,0);word(bytes,at+92,(i+1)%2);word(bytes,at+132,6);}
 return bytes;
}
int main()try{
 QTemporaryDir directory;require(directory.isValid(),"Animation root");const auto root=std::filesystem::path(directory.path().toStdString());write(root,"body.spr",sprite());assets::ResourceManager resources(store(root));const assets::ResourceId body{assets::ResourceKind::creature,"body"};auto recipe=spr();recipe.animationBytes=directionalAnimation();resources.bind(body,recipe);resources.load(body);
 scene::SceneAnimationBindings bindings(resources);const scene::AnimationReceiverId main{100,0},secondary{100,2},special{100,1},other{200,0};
 bindings.apply({{main,Op::bind,0,body},{main,Op::startForward,0,{}},{secondary,Op::bind,0,body},{secondary,Op::startForward,2,{}},{special,Op::bind,0,body},{other,Op::bind,0,body},{other,Op::startForward,3,{}}});
 auto start=bindings.snapshot(main);require(start.frame==0&&start.state.forward.pc==2&&start.state.forward.delay==2&&start.state.forward.elapsed==0,"Owned ANI start");rejected([&]{bindings.snapshot(special);});rejected([&]{bindings.apply({{special,Op::switchSequence,2,{}}});});
 bindings.advance({main});bindings.advance({main});const auto phase=bindings.snapshot(main);require(phase.state.forward.elapsed==2&&bindings.snapshot(other).state.forward.elapsed==0,"Explicit receiver clock selection");
 bindings.apply({{main,Op::switchSequence,1,{}}});auto switched=bindings.snapshot(main);require(switched.frame==1&&switched.state.forward.pc==phase.state.forward.pc&&switched.state.forward.elapsed==phase.state.forward.elapsed&&switched.state.forward.displayedRecord==phase.state.forward.displayedRecord,"View switch restarted ANI phase");
 bindings.advance({main});require(bindings.snapshot(main).frame==0&&bindings.snapshot(main).state.forward.pc==3,"Switched continuation used old sequence");
 const auto before=bindings.snapshot(main);const auto bytes=bindings.retainedBytes();rejected([&]{bindings.apply({{main,Op::switchSequence,4,{}},{secondary,Op::switchSequence,999,{}}});});require(bindings.snapshot(main).sequence==before.sequence&&bindings.snapshot(main).frame==before.frame&&bindings.retainedBytes()==bytes,"Multi-receiver refusal partially applied");
 rejected([&]{bindings.advance({main,main});});require(bindings.snapshot(main).state.forward.pc==before.state.forward.pc&&bindings.snapshot(main).state.forward.elapsed==before.state.forward.elapsed,"Duplicate receiver advanced a second clock");
 auto invalid=before.state;invalid.forward.pc=999;rejected([&]{bindings.restore(main,invalid);});require(bindings.snapshot(main).state.forward.pc==before.state.forward.pc,"Invalid restore partially applied");bindings.restore(main,start.state);require(bindings.snapshot(main).frame==1&&bindings.snapshot(main).state.forward.pc==2,"Owned continuation restore lost current sequence");
 scene::SceneAnimationBindings small(resources,{1,1});rejected([&]{small.apply({{main,Op::bind,0,body}});});require(!small.retainedBytes(),"Animation budget leaked partial binding");
 bool threadRefused=false;std::thread foreign([&]{try{bindings.snapshot(main);}catch(const std::exception&){threadRefused=true;}});foreign.join();require(threadRefused,"Animation owner thread");
 {
  scene::SceneAnimationBindings transactional(resources);transactional.apply({{main,Op::bind,0,body},{main,Op::startForward,0,{}},{secondary,Op::bind,0,body},{secondary,Op::startForward,2,{}}});
  auto candidate=transactional.stage();candidate.advance({main});require(transactional.snapshot(main).state.forward.elapsed==0,"Staged clock escaped before publication");transactional.publish(std::move(candidate));require(transactional.snapshot(main).state.forward.elapsed==1,"Stage publication lost owned continuation");rejected([&]{transactional.publish(std::move(candidate));});auto stale=transactional.stage();transactional.advance({main});rejected([&]{transactional.publish(std::move(stale));});auto foreignCandidate=bindings.stage();rejected([&]{transactional.publish(std::move(foreignCandidate));});auto valid=transactional.snapshot(main).state;auto broken=valid;broken.forward.pc=999;const auto secondBefore=transactional.snapshot(secondary);rejected([&]{transactional.restore({{secondary,valid},{main,broken}});});require(transactional.snapshot(secondary).state.forward.elapsed==secondBefore.state.forward.elapsed,"Batch restore partially committed");rejected([&]{transactional.restore({{main,valid},{main,valid}});});transactional.restore({{main,valid},{secondary,valid}});transactional.retireReceiver(main);rejected([&]{transactional.snapshot(main);});require(transactional.snapshot(secondary).frame.has_value(),"Single-channel retirement removed another channel");transactional.clear();
 }
 const auto oldRevision=resources.resident(body).revision;resources.unload(body);rejected([&]{bindings.snapshot(main);});resources.load(body);require(resources.resident(body).revision!=oldRevision,"Resource reload identity");rejected([&]{bindings.apply({{main,Op::startForward,0,{}}});});bindings.apply({{main,Op::bind,0,body},{main,Op::startForward,5,{}}});require(bindings.snapshot(main).revision==resources.resident(body).revision&&bindings.snapshot(main).frame==1,"Animation revision rebind reused old state");
 bindings.retireOwner(100);rejected([&]{bindings.snapshot(main);});rejected([&]{bindings.snapshot(secondary);});require(bindings.retainedBytes()>0,"Other owner was retired");rejected([&]{bindings.snapshot(other);});bindings.retireOwner(200);require(!bindings.retainedBytes(),"Animation owner retirement charge");bindings.clear();resources.unloadAll();
 std::cout<<QJsonDocument(QJsonObject{{"success",true},{"phase_preserved",true},{"revision_budget_thread_retirement",true},{"double_clock_refused",true}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

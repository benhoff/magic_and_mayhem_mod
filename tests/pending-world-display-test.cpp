#include "resident_world_session.hpp"
#include "gl_viewport.hpp"
#include "campaign_smoke_test.hpp"
#include <QApplication>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <chrono>
#include <thread>
#include <iostream>
static void require(bool v,const char* s){if(!v)throw std::runtime_error(s);}
struct ResidentWorldSessionTestAccess {
 static void seed(ResidentWorldSession& s,const mnm::render::GpuFrame& old,const mnm::render::GpuFrame& next,bool enabled){
  s.asynchronous_=true;s.displayWhilePending_=enabled;s.completed_=2;s.imports_=1;
  s.ownedFrame_=next;s.displayedFrame_=enabled?old:mnm::render::GpuFrame{};s.ownedPending_.emplace();s.ownedPending_->generation=2;s.nextFrame_={};
 }
 static void due(ResidentWorldSession& s){s.nextFrame_={};}
 static void imported(ResidentWorldSession& s){s.imports_=2;s.displayedFrame_=s.ownedFrame_;s.ownedPending_.reset();s.nextFrame_={};}
 static void empty(ResidentWorldSession& s){s.displayedFrame_={};}
};
int main(int argc,char** argv)try{
 CampaignPresentationProbe probe;probe.active=true;probe.frames=103;probe.sourceGeneration=21;
 require(!campaignResumeConfirmed(probe,100,20,true),"Cached paints mistaken for resumed simulation");
 probe.sourceGeneration=22;require(!campaignResumeConfirmed(probe,100,20,true),"Captured two-credit backlog mistaken for fresh resume");
 probe.sourceGeneration=23;require(campaignResumeConfirmed(probe,100,20,true),"Verified fresh resident progress refused");
 probe.sourceGeneration=0;require(campaignResumeConfirmed(probe,100,20,false),"Nonresident resume changed");
 QApplication app(argc,argv);qunsetenv("MNM_ASYNC_WORLD_GPU");qunsetenv("MNM_DISPLAY_WHILE_PENDING");qunsetenv("MNM_WORLD_IMAGE_REUSE");QTemporaryDir root;require(root.isValid(),"Fixture root");
 GlViewport viewport;mnm::render::GlBlitter renderer;const mnm::render::PixelFormat rgb{16,{0xf800,0x7e0,31}};
 auto source=renderer.create({16,16,std::vector<std::uint32_t>(256,0xf800)},rgb),target=renderer.allocate(16,16,rgb);
 auto old=renderer.presentGpu(source).snapshot();renderer.update(source,0,0,{16,16,std::vector<std::uint32_t>(256,0x001f)});auto next=renderer.presentGpu(source).snapshot();
 auto check=[&](const mnm::render::GpuFrame& frame,unsigned colour){require(frame.valid(),"Pending replay returned no native frame");renderer.importGpu565(target,frame);require(renderer.read(target).pixels==std::vector<std::uint32_t>(256,colour),"Pending replay mixed old/new pixels");};
 {
  ResidentWorldSession baseline(viewport,root.filePath("baseline"),root.path(),true);ResidentWorldSessionTestAccess::seed(baseline,old,next,false);
  require(!baseline.drawIndependent(1).valid()&&!baseline.drawIndependent(2).valid(),"Comparison path changed pending admission");
 }
 {
  ResidentWorldSession session(viewport,root.filePath("candidate"),root.path(),true);ResidentWorldSessionTestAccess::seed(session,old,next,true);
  auto state=session.displayState();require(state.generation==2&&state.replayGeneration==1&&state.pending&&!state.replayPending,"Completed/imported generation state collapsed");
  require(!session.drawIndependent(2).valid(),"Pending generation displayed early");
  const auto before=session.nativeFrames();unsigned presents=0;
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(180);
  while(std::chrono::steady_clock::now()<deadline){auto frame=session.drawIndependent(1);if(frame.valid()){check(frame,0xf800);++presents;}std::this_thread::sleep_for(std::chrono::milliseconds(1));}
  require(presents>=8&&session.nativeFrames()==before+presents,"Pending clock blocked or counted undisplayed completion");
  auto report=session.report();require(report["imports"].toString()=="1"&&report["completed"].toString()=="2"&&report["source_accepted"].toString()=="0"&&report["source_returned"].toString()=="0","Replay advanced import/source credits");
  check(next,0x001f);ResidentWorldSessionTestAccess::imported(session);check(session.drawIndependent(2),0x001f);require(!session.drawIndependent(1).valid(),"Superseded generation still displayed");
  ResidentWorldSessionTestAccess::empty(session);ResidentWorldSessionTestAccess::due(session);require(!session.drawIndependent(2).valid(),"Missing imported frame invented pixels");
 }
 old={};next={};renderer.destroy(source);renderer.destroy(target);require(!renderer.stats().surfaces,"Fixture surfaces leaked");
 std::cout<<"{\"success\":true,\"delayed_import_old_pixels_preserved\":true,\"new_generation_not_imported_early\":true,\"source_credits_unchanged\":true,\"pending_native_clock_continues\":true,\"new_import_switches_pixels\":true,\"missing_and_stale_frames_refused\":true,\"comparison_path_preserved\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

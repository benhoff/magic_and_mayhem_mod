#include "../compat/legacy/world_rolling_channel.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <chrono>
#include <iostream>
#include <thread>
#include <sys/wait.h>
#include <unistd.h>
using namespace mnm::legacy;
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
template<class F> void reject(F action){try{action();}catch(const std::exception&){return;}throw std::runtime_error("Invalid channel operation accepted");}
static void edit(const QString& path,std::size_t at,std::uint32_t value){QFile f(path);require(f.open(QIODevice::ReadWrite),"Fixture file");auto* p=f.map(0,MNM_ROLL_SIZE);require(p,"Fixture map");*reinterpret_cast<std::uint32_t*>(p+at)=value;f.unmap(p);}
int main(int argc,char** argv)try{
  QCoreApplication app(argc,argv);QTemporaryDir root;require(root.isValid(),"Fixture root");unsigned refusals=0;
  auto fixture=[&](auto action){const auto path=root.filePath(QString::number(refusals));WorldRollingChannel::create(path,0x123456789abcdef0ULL);WorldRollingChannel p(path,0x123456789abcdef0ULL,WorldRollingChannel::Role::producer),c(path,0x123456789abcdef0ULL,WorldRollingChannel::Role::consumer);action(path,p,c);++refusals;};
  fixture([&](auto path,auto&,auto&){reject([&]{WorldRollingChannel wrong(path,9,WorldRollingChannel::Role::producer);});});
  fixture([&](auto path,auto&,auto&){reject([&]{WorldRollingChannel duplicate(path,0x123456789abcdef0ULL,WorldRollingChannel::Role::consumer);});});
  fixture([&](auto path,auto& p,auto& c){edit(path,16,5);reject([&]{p.publish({1});});require(c.cancelled(),"Identity mutation was not terminal");});
  fixture([&](auto path,auto& p,auto& c){edit(path,44,0);reject([&]{p.publish({1});});require(c.cancelled(),"Role ownership loss was not terminal");});
  fixture([&](auto,auto& p,auto&){reject([&]{p.publish({});});require(p.cancelled(),"Invalid input not terminal");});
  fixture([&](auto,auto& p,auto&){reject([&]{p.publish(std::vector<std::uint8_t>(MNM_ROLL_INPUT+1));});});
  for(unsigned at:{4u,8u,12u,16u,20u,36u,60u})fixture([&](auto path,auto& p,auto& c){require(p.publish({1,2,3}),"Publish");edit(path,64+at,at==12?MNM_ROLL_INPUT+1:at==4?2:1);reject([&]{c.poll();});require(c.cancelled(),"Invalid source not terminal");});
  fixture([&](auto path,auto& p,auto& c){require(p.publish({1}),"Publish");edit(path,64,5);reject([&]{c.poll();});});
  fixture([&](auto,auto& p,auto& c){p.publish({1});const auto in=c.poll();require(bool(in),"Poll");reject([&]{c.poll();});});
  fixture([&](auto,auto& p,auto& c){p.publish({1});c.poll();reject([&]{c.complete(2,{1,1,{1}});});});
  fixture([&](auto,auto& p,auto& c){p.publish({1});c.poll();reject([&]{c.complete(1,{1,1,{65536}});});});
  fixture([&](auto,auto& p,auto& c){p.publish({1});c.poll();reject([&]{c.complete(1,{2,1,{1}});});});
  for(unsigned at:{4u,8u,20u,24u,28u,32u,40u})fixture([&](auto path,auto& p,auto& c){p.publish({1});c.poll();c.complete(1,{1,1,{1}});edit(path,64+at,at==4?2:at==8?1:at==20?2049:at==24?0:at==28?MNM_ROLL_REPLY+1:123);reject([&]{p.reap();});require(p.cancelled(),"Invalid reply not terminal");});
  fixture([&](auto,auto& p,auto& c){p.cancel();require(c.cancelled(),"Peer missed cancel");reject([&]{p.publish({1});});reject([&]{c.poll();});});
  fixture([&](auto path,auto& p,auto& c){p.publish({1});edit(path,64,MNM_ROLL_WRITING);require(!c.poll(),"Incomplete publication became visible");c.cancel();reject([&]{p.publish({2});});});
  // Two independently mapped processes cycle the same slots1000 times. The
  // parent intentionally starts late; the child must retain input under pressure.
  const auto path=root.filePath("process");WorldRollingChannel::create(path,0xabcdef0123456789ULL);
  const auto pid=fork();require(pid>=0,"Fork");
  if(pid==0){try{
    WorldRollingChannel p(path,0xabcdef0123456789ULL,WorldRollingChannel::Role::producer);
    unsigned sent=0,acked=0,pressure=0;auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
    while(acked<1000){
      if(sent<1000){std::vector<std::uint8_t> bytes{std::uint8_t(sent+1),std::uint8_t((sent+1)>>8)};if(p.publish(bytes)){++sent;bytes[0]^=255;}else ++pressure;}
      if(auto done=p.reap()){require(done->sequence==++acked&&done->image.width==1&&done->image.pixels==std::vector<std::uint32_t>{acked},"Child ACK changed sequence/pixels");}
      require(std::chrono::steady_clock::now()<deadline,"Child channel stalled");std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    require(pressure>0&&p.published()==1000&&p.reclaimed()==1000,"Backpressure was not exercised");_exit(0);
  }catch(...){_exit(1);}}
  WorldRollingChannel c(path,0xabcdef0123456789ULL,WorldRollingChannel::Role::consumer);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));unsigned received=0;auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
  try{while(received<1000){if(auto in=c.poll()){++received;require(in->sequence==received&&in->bytes==std::vector<std::uint8_t>{std::uint8_t(received),std::uint8_t(received>>8)},"Input ownership/order changed");c.complete(in->sequence,{1,1,{received}});}require(std::chrono::steady_clock::now()<deadline,"Parent channel stalled");std::this_thread::sleep_for(std::chrono::microseconds(100));}}
  catch(...){c.cancel();int status;waitpid(pid,&status,0);throw;}
  int status=0;require(waitpid(pid,&status,0)==pid&&WIFEXITED(status)&&WEXITSTATUS(status)==0,"Producer process failed");
  // A producer actually exits with a published request. The surviving peer can
  // finish that owned request, but cannot resume the dead producer's role/history.
  const auto orphan=root.filePath("orphan");WorldRollingChannel::create(orphan,23);
  const auto orphanPid=fork();require(orphanPid>=0,"Orphan fork");
  if(orphanPid==0){try{WorldRollingChannel p(orphan,23,WorldRollingChannel::Role::producer);require(p.publish({7}),"Orphan publish");_exit(0);}catch(...){_exit(1);}}
  int orphanStatus=0;require(waitpid(orphanPid,&orphanStatus,0)==orphanPid&&WIFEXITED(orphanStatus)&&WEXITSTATUS(orphanStatus)==0,"Orphan producer failed before publication");
  WorldRollingChannel survivor(orphan,23,WorldRollingChannel::Role::consumer);auto held=survivor.poll();require(held&&held->bytes==std::vector<std::uint8_t>{7},"Exited producer lost owned input");survivor.complete(held->sequence,{1,1,{7}});
  const auto stalled=std::chrono::steady_clock::now();do{require(!survivor.poll(),"Dead producer published again");std::this_thread::sleep_for(std::chrono::milliseconds(1));}while(std::chrono::steady_clock::now()-stalled<std::chrono::milliseconds(20));
  survivor.cancel();reject([&]{WorldRollingChannel resume(orphan,23,WorldRollingChannel::Role::producer);});
  // A new session cannot admit an old consumer/ACK identity.
  const auto restart=root.filePath("restart");WorldRollingChannel::create(restart,17);reject([&]{WorldRollingChannel old(restart,0xabcdef0123456789ULL,WorldRollingChannel::Role::consumer);});WorldRollingChannel fresh(restart,17,WorldRollingChannel::Role::consumer);require(!fresh.cancelled(),"Wrong old attachment poisoned a fresh session");
  std::cout<<"Rolling channel:1000 ordered cross-process input/ACK cycles, backpressure, "<<refusals<<" terminal/ownership refusals and fresh-session admission pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

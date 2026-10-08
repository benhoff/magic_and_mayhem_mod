#include "world_preparation.hpp"
#include "resource-fixtures.hpp"
#include "../protocols/include/mnm/world_frame_v1.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
using namespace resource_test;
using namespace mnm::legacy;
static void put(QByteArray& bytes,qsizetype at,quint32 value){for(unsigned i=0;i<4;++i)bytes[at+i]=char(value>>(8*i));}
static QByteArray input(const Bytes& sprite){
    QByteArray frame(reinterpret_cast<const char*>(sprite.data()+804),53);put(frame,28,0);
    QByteArray bytes(144,0);std::memcpy(bytes.data(),"MNMWRLD1",8);put(bytes,8,1);put(bytes,12,80);put(bytes,20,1);put(bytes,24,5);put(bytes,28,3);put(bytes,32,5);put(bytes,36,1);put(bytes,44,MNM_WORLD_BUILD);
    put(bytes,80,629);put(bytes,88,2);put(bytes,104,5);put(bytes,108,3);put(bytes,112,53);put(bytes,116,1);put(bytes,120,512);
    bytes.append(frame);for(unsigned i=0;i<256;++i){bytes.append(char(i));bytes.append(char(i>>8));}put(bytes,16,quint32(bytes.size()));return bytes;
}
static WorldPreparationReply await(WorldPreparation& worker){
    QElapsedTimer timeout;timeout.start();
    while(timeout.elapsed()<10000){if(auto reply=worker.take())return std::move(*reply);std::this_thread::sleep_for(std::chrono::milliseconds(1));}
    throw std::runtime_error("Worker completion timed out");
}
static void drain(WorldPreparation& worker){
    worker.cancel();QElapsedTimer timeout;timeout.start();
    while(!worker.stopped()&&timeout.elapsed()<10000)std::this_thread::sleep_for(std::chrono::milliseconds(1));
    require(worker.stopped(),"Worker did not release cancelled state");require(!worker.take(),"Cancelled worker published a stale completion");
}
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);QTemporaryDir directory;const auto root=std::filesystem::path(directory.path().toStdString());
    const auto spriteBytes=sprite(true);write(root,"body.spr",spriteBytes);
    mnm::assets::ResourceManager manager(store(root));
    {WorldPreparation worker(store(root));worker.plan(input(spriteBytes));rejected([&]{worker.plan(input(spriteBytes));});
        auto plan=std::get<WorldPreparationPlan>(await(worker));require(plan.files.size()==1&&plan.frame->draws.size()==1,"Worker plan differs");
        const auto& file=plan.files.front();manager.bind(file.id,spr(file.path));
        worker.prepare({manager.request(file.id)},128*1024*1024,64);
        auto prepared=std::get<PreparedWorld>(await(worker));require(prepared.resources.size()==1&&prepared.draws.size()==1&&prepared.draws[0].frame==0,"Prepared binding differs");
        require(prepared.resources[0].sourceImage().empty(),"Completion retained encoded scratch");
        std::filesystem::remove(root/"body.spr");manager.adopt(std::move(prepared.resources[0]));
        const auto& draw=prepared.draws.front();
        mnm::render::SceneUploadNeed need{0,{draw.resource,manager.resident(draw.resource).revision,draw.frame,draw.colours,{}}};
        worker.upload(need);rejected([&]{worker.upload(need);});
        auto upload=std::get<PreparedWorldUpload>(await(worker));
        require(upload.frame==prepared.frame&&upload.need.request==need.request&&upload.planes.mask.pixels==std::vector<std::uint32_t>({1,1,0})&&upload.planes.pixels.pixels[0]==0&&upload.planes.pixels.pixels[1]==1,"Owned worker upload differs");
        require(worker.stats().uploadsPrepared==1&&worker.stats().retainedDecodedBytes>0,"Upload preparation counters differ");
        worker.plan(input(spriteBytes));await(worker);worker.prepare({},0,0);
        auto warm=std::get<PreparedWorld>(await(worker));require(warm.resources.empty()&&warm.draws.size()==1&&worker.stats().resourcesPrepared==1,"Warm frame repeated file decoding");
        drain(worker);
    }
    write(root,"body.spr",spriteBytes);
    {WorldPreparation worker(store(root));worker.plan(input(spriteBytes));auto plan=std::get<WorldPreparationPlan>(await(worker));
        const auto& file=plan.files.front();manager.unload(file.id);
        auto changed=spriteBytes;changed[27]=1;write(root,"body.spr",changed);
        worker.prepare({manager.request(file.id)},128*1024*1024,64);
        require(std::get<std::string>(await(worker)).find("hash mismatch")!=std::string::npos,"Changed pinned SPR was admitted");
        require(!manager.isResident(file.id),"Failed worker changed manager residency");drain(worker);
    }
    write(root,"body.spr",spriteBytes);
    {WorldPreparation worker(store(root));worker.plan(input(spriteBytes));auto plan=std::get<WorldPreparationPlan>(await(worker));
        worker.prepare({manager.request(plan.files.front().id)},1,1);
        require(std::get<std::string>(await(worker)).find("byte budget")!=std::string::npos,"Worker completion exceeded byte budget");drain(worker);
    }
    {auto entered=std::make_shared<std::atomic<bool>>(false);
        WorldPreparation worker(store(root),[entered](const std::atomic<bool>& stop){*entered=true;while(!stop)std::this_thread::sleep_for(std::chrono::milliseconds(1));});
        QElapsedTimer timeout;timeout.start();while(!*entered&&timeout.elapsed()<10000)std::this_thread::sleep_for(std::chrono::milliseconds(1));
        require(*entered,"Catalogue checkpoint not reached");worker.plan(input(spriteBytes));timeout.restart();worker.cancel();
        require(timeout.elapsed()<100,"Cancellation joined blocked catalogue work");drain(worker);
    }
    std::cout<<"Owned cold/warm World preparation, queue bounds, fingerprint refusal, completion budgets, cancellation and worker retirement pass\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}

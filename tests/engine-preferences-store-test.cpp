#include "engine_preferences_store.hpp"
#include "engine_preferences_persistence.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLockFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>
#include <future>
#include <chrono>
#include <vector>
#include <cstdio>
#include <cstdlib>
static void check(bool yes,int line){if(!yes){std::fprintf(stderr,"Preference store check failed at %d\n",line);std::exit(1);}}
#define require(x) check((x),__LINE__)
static QByteArray read(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly));return f.readAll();}
static void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly));require(f.write(bytes)==bytes.size());}
static void asynchronous(QCoreApplication& app,QTemporaryDir& dir,const QByteArray& cfg,const std::array<int,7>& values){
    using namespace std::chrono_literals;
    const auto profile=dir.filePath("async.cfg"),store=dir.filePath("async.json");
    write(profile,cfg);EnginePreferencesStore disk;disk.begin(store,"missing");
    std::promise<void> entered,allowed;auto started=entered.get_future();auto gate=allowed.get_future().share();
    const auto owner=std::this_thread::get_id();std::vector<int> committed;
    EnginePreferencesPersistence queue([&](const QByteArray& bytes,const auto& expected,QString* error){
        require(std::this_thread::get_id()!=owner);
        if(committed.empty()){entered.set_value();gate.wait();}
        committed.push_back(expected[0]);return disk.acceptSnapshot(bytes,expected,error);
    });queue.begin(store,"missing");
    require(queue.accept(profile,values));require(started.wait_for(2s)==std::future_status::ready);
    auto next=values;next[0]=8;write(profile,QByteArray(cfg).replace("MusicVolume=9","MusicVolume=8"));
    require(queue.accept(profile,next));write(profile,"Not a valid profile anymore");
    int pulses=0;bool opened=false;QTimer heartbeat;heartbeat.setInterval(10);
    QObject::connect(&heartbeat,&QTimer::timeout,[&]{if(++pulses==5){opened=true;allowed.set_value();}});heartbeat.start();
    QElapsedTimer bound;bound.start();
    while(queue.pending()&&bound.elapsed()<3000){app.processEvents();QThread::msleep(1);}
    heartbeat.stop();require(opened&&pulses>=5&&!queue.pending());queue.wait();
    auto results=queue.takeResults();require(results.size()==2&&results[0].saved&&results[1].saved);
    require(committed==std::vector<int>({9,8}));require(QJsonDocument::fromJson(read(store)).object()["values"].toArray()[0].toInt()==8);

    // A competing accepted revision must still refuse without overwriting it.
    write(profile,cfg);const auto conflict=dir.filePath("conflict.json");
    EnginePreferencesStore other;other.begin(conflict,"missing");require(other.accept(profile,values));const auto previous=read(conflict);
    EnginePreferencesPersistence stale;stale.begin(conflict,"missing");require(stale.accept(profile,values));stale.wait();
    results=stale.takeResults();require(results.size()==1&&!results[0].saved&&results[0].error.contains("another session")&&read(conflict)==previous);

    // The total in-flight, queued and unconsumed-result budget is finite.
    std::promise<void> fullEntered,fullAllowed;auto fullStarted=fullEntered.get_future();auto fullGate=fullAllowed.get_future().share();
    unsigned writes=0;EnginePreferencesStore capacity;capacity.begin(dir.filePath("capacity.json"),"missing");
    EnginePreferencesPersistence bounded([&](const QByteArray& bytes,const auto& expected,QString* error){
        if(!writes++){fullEntered.set_value();fullGate.wait();}return capacity.acceptSnapshot(bytes,expected,error);
    });bounded.begin(dir.filePath("capacity.json"),"missing");require(bounded.accept(profile,values));require(fullStarted.wait_for(2s)==std::future_status::ready);
    for(std::size_t i=1;i<EnginePreferencesPersistence::Capacity;++i)require(bounded.accept(profile,values));
    QString error;require(!bounded.accept(profile,values,&error)&&error.contains("backlog"));fullAllowed.set_value();bounded.wait();
    results=bounded.takeResults();require(results.size()==EnginePreferencesPersistence::Capacity);
    for(const auto& result:results)require(result.saved);

    // Destruction drains accepted jobs; the writer never outlives its owner.
    std::promise<void> closingEntered,closingAllowed;auto closingStarted=closingEntered.get_future();auto closingGate=closingAllowed.get_future().share();
    std::promise<unsigned> retired;auto retiredResult=retired.get_future();
    const auto shutdown=dir.filePath("shutdown.json");
    std::thread retiring([&]{
        unsigned count=0;EnginePreferencesStore target;target.begin(shutdown,"missing");
        {EnginePreferencesPersistence pending([&](const QByteArray& bytes,const auto& expected,QString* problem){
            if(!count++){closingEntered.set_value();closingGate.wait();}return target.acceptSnapshot(bytes,expected,problem);
         });pending.begin(shutdown,"missing");require(pending.accept(profile,values)&&pending.accept(profile,values));}
        retired.set_value(count);
    });
    require(closingStarted.wait_for(2s)==std::future_status::ready&&retiredResult.wait_for(0ms)==std::future_status::timeout);
    closingAllowed.set_value();require(retiredResult.wait_for(2s)==std::future_status::ready&&retiredResult.get()==2);retiring.join();
    require(QJsonDocument::fromJson(read(shutdown)).object()["values"].toArray()[0].toInt()==9);
    std::puts("Asynchronous Preferences checks passed: heartbeat, immutable FIFO snapshots, conflict, bounded backlog, retirement");
}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir dir;const auto store=dir.filePath("preferences.json"),profile=dir.filePath("prefs.cfg");
    const QByteArray cfg="[VIDEO]\r\nIsHighRes=TRUE\r\nCutDownAnims=FALSE\r\nDialogSpeed=2\r\nMaxFramesPerSec=17\r\nWindowSize=1\r\nTerrainLightLevels=64\r\n[SOUND]\r\nMusicVolume=9\r\nSFXVolume=-500 ; retained comment\r\nCDMusicEnabled=FALSE\r\n";
    write(profile,cfg);const std::array<int,7> values{9,-500,0,0,2,1,1};EnginePreferencesStore first,second;first.begin(store,"missing");second.begin(store,"missing");
    require(!QFile::exists(store));std::array<int,7> actual{};require(EnginePreferencesStore::readProfile(profile,actual)&&actual==values);
    auto wrong=values;wrong[4]=0;QString error;require(!first.accept(profile,wrong,&error)&&!error.isEmpty()&&!QFile::exists(store));
    require(first.accept(profile,values,&error)&&error.isEmpty()&&read(profile)==cfg);
    const auto saved=read(store);const auto json=QJsonDocument::fromJson(saved).object();require(json.value("schema").toInt()==1&&json.value("values").toArray()[1].toInt()==-500);
    require(!second.accept(profile,values,&error)&&error.contains("another session")&&read(store)==saved);
    require(first.accept(profile,values)); // Same-session subsequent OK uses updated revision.
    const auto revision=QCryptographicHash::hash(read(store),QCryptographicHash::Sha256).toHex();second.begin(store,revision);
    QLockFile lock(store+".lock");require(lock.tryLock());require(!second.accept(profile,values,&error)&&read(store)==saved);lock.unlock();
    require(second.accept(profile,values));
    write(profile,cfg+"SFXVolume=-1000\r\n");require(!first.accept(profile,values,&error)&&error.contains("Ambiguous")&&read(store)==saved);
    write(profile,QByteArray(cfg).replace("MaxFramesPerSec=17","MaxFramesPerSec=100"));require(!first.accept(profile,values,&error)&&read(store)==saved);
    write(profile,cfg);EnginePreferencesStore unavailable;unavailable.begin(dir.filePath("missing-parent/file/store.json"),"missing");
    write(dir.filePath("missing-parent"),"a file");require(!unavailable.accept(profile,values,&error)&&!error.isEmpty());
    write(store,"{ corrupt");second.begin(store,QCryptographicHash::hash(read(store),QCryptographicHash::Sha256).toHex());require(second.accept(profile,values)); // Accepted, validated settings can repair corrupt store.
    auto invalid=values;invalid[0]=-1;require(!second.accept(profile,invalid,&error)&&!error.isEmpty());
    asynchronous(app,dir,cfg,values);
    std::puts("Engine Preferences store checks passed");
}

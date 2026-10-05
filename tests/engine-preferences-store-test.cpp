#include "engine_preferences_store.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLockFile>
#include <cstdio>
#include <cstdlib>
static void check(bool yes,int line){if(!yes){std::fprintf(stderr,"Preference store check failed at %d\n",line);std::exit(1);}}
#define require(x) check((x),__LINE__)
static QByteArray read(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly));return f.readAll();}
static void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly));require(f.write(bytes)==bytes.size());}
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
    std::puts("Engine Preferences store checks passed");
}

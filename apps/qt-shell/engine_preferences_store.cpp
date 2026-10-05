#include "engine_preferences_store.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QMap>
namespace {
const char* build="40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168";
bool fail(QString* error,const QString& text){if(error)*error=text;return false;}
}
void EnginePreferencesStore::begin(QString path,QByteArray revision){path_=std::move(path);revision_=std::move(revision);}
bool EnginePreferencesStore::valid(const std::array<int,7>& v){
    return v[0]>=0&&v[0]<=15&&v[1]>=-2500&&v[1]<=0&&v[2]>=0&&v[2]<=1&&v[3]>=0&&v[3]<=1&&v[4]>=0&&v[4]<=2&&v[5]>=0&&v[5]<=2&&v[6]>=0&&v[6]<=1;
}
bool EnginePreferencesStore::readProfile(const QString& path,std::array<int,7>& values,QString* error){
    QFile f(path);if(!f.open(QIODevice::ReadOnly)||f.size()>65536)return fail(error,"Cannot read engine-written preferences.");
    const auto text=QString::fromLatin1(f.readAll());QString section;QMap<QString,QString> found;
    const QStringList keys{"sound/musicvolume","sound/sfxvolume","video/ishighres","video/cutdownanims","video/dialogspeed","video/maxframespersec","video/windowsize"};
    const QRegularExpression heading("^\\s*\\[([^]]+)\\]\\s*(?:;.*)?$");
    const QRegularExpression assignment("^\\s*([^=;]+?)\\s*=\\s*([^;]*?)(?:\\s*;.*)?$");
    for(auto line:text.split('\n')){
        line=line.trimmed();const auto h=heading.match(line);if(h.hasMatch()){section=h.captured(1).toLower();continue;}
        const auto a=assignment.match(line);if(!a.hasMatch())continue;
        const auto key=section+"/"+a.captured(1).trimmed().toLower();if(!keys.contains(key))continue;
        if(found.contains(key))return fail(error,"Ambiguous engine-written preferences.");
        found.insert(key,a.captured(2).trimmed());
    }
    std::array<int,7> next{};
    for(int i=0;i<7;++i){if(!found.contains(keys[i]))return fail(error,"Incomplete engine-written preferences.");
        const auto value=found[keys[i]];bool ok=false;
        if(i==2||i==3){if(value.compare("TRUE",Qt::CaseInsensitive)==0)next[i]=i==2?0:1;
            else if(value.compare("FALSE",Qt::CaseInsensitive)==0)next[i]=i==2?1:0;
            else return fail(error,"Invalid engine preference boolean.");}
        else {next[i]=value.toInt(&ok);if(!ok)return fail(error,"Invalid engine preference integer.");}
    }
    const int rate=next[5];if(rate!=20&&rate!=17&&rate!=14)return fail(error,"Invalid engine game speed.");next[5]=rate==20?0:rate==17?1:2;
    if(!valid(next))return fail(error,"Engine preferences are outside recovered bounds.");
    values=next;return true;
}
bool EnginePreferencesStore::accept(const QString& profile,const std::array<int,7>& expected,QString* error){
    if(error)error->clear();
    std::array<int,7> actual{};
    if(path_.isEmpty()||revision_.isEmpty())return fail(error,"Preference persistence was not initialized.");
    if(!valid(expected))return fail(error,"Invalid accepted Preferences values.");
    if(!readProfile(profile,actual,error))return false;
    if(actual!=expected)return fail(error,"Original preference writer did not write the accepted settings.");
    const auto parent=QFileInfo(path_).absolutePath();if(!QDir().mkpath(parent))return fail(error,"Cannot create preference store directory.");
    QLockFile lock(path_+".lock");if(!lock.tryLock(0))return fail(error,"Preference store is in use by another session.");
    QByteArray current="missing";QFile previous(path_);
    if(previous.exists()){
        if(!previous.open(QIODevice::ReadOnly)||previous.size()>65536)return fail(error,"Cannot read existing preference store.");
        current=QCryptographicHash::hash(previous.readAll(),QCryptographicHash::Sha256).toHex();previous.close();
    }
    if(current!=revision_)return fail(error,"Preferences changed in another session; relaunch before saving again.");
    QJsonArray values;for(int v:actual)values.append(v);
    const auto bytes=QJsonDocument(QJsonObject{{"schema",1},{"source_sha256",build},{"values",values}}).toJson();
    QSaveFile file(path_);file.setDirectWriteFallback(false);
    if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.commit())return fail(error,"Could not atomically save preferences.");
    revision_=QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex();return true;
}

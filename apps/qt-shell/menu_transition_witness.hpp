#pragma once
#include <QFile>
#include <QCoreApplication>
#include <QtEndian>
#include <array>
#include <cstring>
#include <time.h>
#include "../../protocols/include/mnm/menu_timeline_v1.h"

// Opt-in diagnostic only. File allocation/mapping precedes producer launch;
// recording copies 64 bytes into a bounded append-only mapped journal. No
// synchronous database or log write on the live GUI path. Page faults and
// external observation can still perturb scheduling; no latency bound implied.
class MenuTransitionWitness final {
public:
    ~MenuTransitionWitness(){close();}
    bool open(const QString& path){
        if(qEnvironmentVariableIntValue("MNM_MENU_TIMELINE")!=1 ||
           qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")!=1 ||
           qEnvironmentVariableIntValue("MNM_ACCEPTANCE_DIAGNOSTIC")!=1)return false;
        if(bytes_)return false;
        file_.setFileName(path);
        if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(MNM_MENU_TIMELINE_SIZE))return false;
        bytes_=file_.map(0,MNM_MENU_TIMELINE_SIZE);if(!bytes_)return false;
        std::memset(bytes_,0,MNM_MENU_TIMELINE_SIZE);
        std::memcpy(bytes_,MNM_MENU_TIMELINE_MAGIC,8);
        word(2,MNM_MENU_TIMELINE_VERSION);word(3,MNM_MENU_TIMELINE_RECORD);
        word(4,MNM_MENU_TIMELINE_CAPACITY);word(7,quint32(QCoreApplication::applicationPid()));word(9,1);
        control_=qEnvironmentVariableIntValue("MNM_MENU_TIMELINE_CONTROL")==1;word(11,control_);
        return true;
    }
    bool enabled() const{return bytes_!=nullptr;}
    bool tracked() const{return tracked_!=0;}
    bool resolved() const{return resolved_;}
    bool suppressRead() const{return enabled()&&control_&&published_&&!resolved_;}
    void resolve(){resolved_=true;}
    void record(quint32 event,std::array<quint32,16> row){
        if(!bytes_)return;
        if(event==MNM_MTL_REQUEST_BEFORE&&!tracked_){tracked_=row[4];word(10,tracked_);}
        if(event==MNM_MTL_REQUEST_PUBLISHED)published_=true;
        if(count_==MNM_MENU_TIMELINE_CAPACITY){word(6,++dropped_);return;}
        timespec ts{};if(clock_gettime(CLOCK_MONOTONIC,&ts)!=0){word(6,++dropped_);return;}
        const quint64 ns=quint64(ts.tv_sec)*1000000000ULL+quint64(ts.tv_nsec);
        row[0]=count_+1;row[1]=event;row[2]=quint32(ns);row[3]=quint32(ns>>32);
        for(auto& value:row)value=qToLittleEndian(value);
        std::memcpy(bytes_+MNM_MENU_TIMELINE_HEADER+count_*MNM_MENU_TIMELINE_RECORD,row.data(),MNM_MENU_TIMELINE_RECORD);
        ++count_;__atomic_store_n(reinterpret_cast<quint32*>(bytes_)+5,qToLittleEndian(count_),__ATOMIC_RELEASE);
    }
    void close(){
        if(!bytes_)return;
        __atomic_store_n(reinterpret_cast<quint32*>(bytes_)+8,qToLittleEndian(quint32(1)),__ATOMIC_RELEASE);
        file_.unmap(bytes_);bytes_=nullptr;file_.close();
    }
private:
    void word(unsigned index,quint32 value){qToLittleEndian(value,bytes_+4*index);}
    QFile file_;uchar* bytes_=nullptr;quint32 count_=0,dropped_=0,tracked_=0;
    bool control_=false,resolved_=false,published_=false;
};

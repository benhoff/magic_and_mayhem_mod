#pragma once
#include "../../protocols/include/mnm/gameplay_fps_v1.h"
#include "gameplay_fps.hpp"
#include <QFile>
#include <optional>

// Session-owned read-only observation. No widget or renderer dependencies.
class GameplayFpsChannel {
public:
    ~GameplayFpsChannel(){close();}
    void close(){if(words_)file_.unmap(reinterpret_cast<uchar*>(words_));words_=nullptr;file_.close();}
    bool create(const QString& path){
        close();file_.setFileName(path);
        if(!file_.open(QIODevice::ReadWrite|QIODevice::Truncate))return false;
        const std::uint32_t header[8]={MNM_GAMEPLAY_FPS_MAGIC0,MNM_GAMEPLAY_FPS_MAGIC1,1,MNM_GAMEPLAY_FPS_BYTES,0,0,0,0};
        if(file_.write(reinterpret_cast<const char*>(header),sizeof(header))!=sizeof(header)||!file_.flush()){close();return false;}
        words_=reinterpret_cast<std::uint32_t*>(file_.map(0,MNM_GAMEPLAY_FPS_BYTES));return words_!=nullptr;
    }
    std::optional<GameplayFpsProgress> progress() const{
        if(!words_||!mnm_gameplay_fps_header(words_))return {};
        const auto sequence=__atomic_load_n(words_+4,__ATOMIC_ACQUIRE);
        if(sequence&1)return {};
        const GameplayFpsProgress result{__atomic_load_n(words_+5,__ATOMIC_RELAXED),__atomic_load_n(words_+6,__ATOMIC_RELAXED)};
        const auto ready=__atomic_load_n(words_+7,__ATOMIC_RELAXED);
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        if(ready!=1||sequence!=__atomic_load_n(words_+4,__ATOMIC_ACQUIRE))return {};
        return result;
    }
private:
    QFile file_;
    std::uint32_t* words_=nullptr;
};

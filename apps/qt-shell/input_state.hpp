#pragma once
#include "../../protocols/include/mnm/input_v1.h"
#include <QFile>
#include <QPoint>
#include <QSize>
#include <array>

class InputState {
public:
    static constexpr int Size=MNM_INPUT_V1_SIZE;
    ~InputState();
    bool create(const QString& path);
    void publish(bool active,QPoint position,QSize size);
    void key(unsigned code,bool down);
    void clear();
private:
    QFile file_;uchar* mapping_=nullptr;std::array<quint32,MNM_INPUT_V1_KEY_COUNT> keys_{};
};

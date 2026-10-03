#pragma once
#include <QFile>
#include <QPoint>
#include <QSize>
#include <array>

class InputState {
public:
    static constexpr int Size=1088;
    ~InputState();
    bool create(const QString& path);
    void publish(bool active,QPoint position,QSize size);
    void key(unsigned code,bool down);
    void clear();
private:
    QFile file_;uchar* mapping_=nullptr;std::array<quint32,256> keys_{};
};

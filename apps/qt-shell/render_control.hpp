#pragma once
#include <QFile>
#include <QElapsedTimer>
#include "../../protocols/include/mnm/render_control_v1.h"
// Single outstanding host request; cancel is permanent for this launch.
class RenderControl final {
public:
    ~RenderControl();
    bool create(const QString& path,quint32 launch);
    bool recover(const QString& path,quint32 session);
    bool checkpoint(const QString& path,quint32 session);
    // 0 waiting, 1 accepted, -1 refused/invalid/timed out.
    int poll();
    void cancel();
    QString error() const{return error_;}
private:
    bool request(const QString& path,quint32 session,quint32 operation);
    quint32 load(unsigned offset) const;
    void store(unsigned offset,quint32 value);
    bool identity() const;
    QFile file_;uchar* map_=nullptr;quint32 launch_=0,sequence_=0;
    bool cancelled_=false,pending_=false;QString error_;QElapsedTimer deadline_;
};

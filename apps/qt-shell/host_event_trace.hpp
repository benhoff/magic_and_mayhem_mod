#pragma once
#include <QApplication>
#include <QEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <chrono>
#include <cstdio>

// Observe complete GUI callbacks, including service work outside render polls.
// Object identities are local to this execution; no lease/deadline is renewed.
class HostEventApplication final : public QApplication {
public:
    using QApplication::QApplication;
    bool notify(QObject* receiver,QEvent* event) override {
        if(!enabled_ || !receiver || !event || QThread::currentThread()!=thread())
            return QApplication::notify(receiver,event);
        // A callback may destroy its receiver or event. Copy labels first.
        const auto receiverClass=QString::fromLatin1(receiver->metaObject()->className());
        const auto receiverName=receiver->objectName();
        const auto identity=quintptr(receiver);
        const auto type=int(event->type());
        const auto begin=Clock::now();
        const bool result=QApplication::notify(receiver,event);
        const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-begin).count();
        if(elapsed>=50000 && rows_++<256){
            const auto row=QJsonDocument(QJsonObject{{"receiver_class",receiverClass},{"receiver_name",receiverName},
                {"receiver_identity",QString::number(identity,16)},{"event_type",type},
                {"begin_host_us",QString::number(std::chrono::duration_cast<std::chrono::microseconds>(begin.time_since_epoch()).count())},
                {"elapsed_us",QString::number(elapsed)}}).toJson(QJsonDocument::Compact);
            std::fprintf(stderr,"NATIVE_HOST_EVENT %s\n",row.constData());
        }
        return result;
    }
private:
    using Clock=std::chrono::steady_clock;
    const bool enabled_=qEnvironmentVariableIntValue("MNM_RESIDENT_POLL_TIMING")==1;
    unsigned rows_=0;
};

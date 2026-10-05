#pragma once
#include <QString>
#include <QByteArray>
#include <array>
// Application persistence for engine-written, exposed Preferences only.
// No widgets, runtime pointers, channel offsets or original-file writes.
class EnginePreferencesStore final {
public:
    void begin(QString path,QByteArray revision);
    bool accept(const QString& profile,const std::array<int,7>& expected,QString* error=nullptr);
    static bool readProfile(const QString&,std::array<int,7>&,QString* error=nullptr);
    static bool valid(const std::array<int,7>&);
private:
    QString path_;QByteArray revision_;
};

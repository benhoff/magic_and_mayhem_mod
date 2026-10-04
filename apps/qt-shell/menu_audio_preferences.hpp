#pragma once
#include <QRegularExpression>
#include <QSettings>

// Versioned native preview policy, separate from recovered game preferences.
namespace menuAudioPreferences {
inline int read(QSettings* settings,const QString& key,int fallback){
    if(!settings)return fallback;
    settings->setFallbacksEnabled(false);
    const auto value=settings->value(key).toString();
    bool ok=false;const int level=value.toInt(&ok);
    return QRegularExpression("^-?[0-9]+$").match(value).hasMatch() && ok && level>= -10000 && level<=0?level:fallback;
}
inline bool save(QSettings* settings,const QString& key,int level){
    if(!settings)return true;
    if(level< -10000 || level>0)return false;
    settings->setValue(key,level);settings->sync();
    return settings->status()==QSettings::NoError;
}
}

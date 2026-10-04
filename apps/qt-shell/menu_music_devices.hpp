#pragma once
#include <QByteArray>
#include <vector>
#include <algorithm>

enum class MenuMusicDeviceChange {none,disconnected,defaultChanged};
// Only the selected device matters while playing. Unrelated list changes must
// not restart a track; actual Qt notification delivery is a hardware boundary.
inline MenuMusicDeviceChange musicDeviceChange(bool active,const QByteArray& selected,
    const std::vector<QByteArray>& devices,const QByteArray& currentDefault){
    if(!active)return MenuMusicDeviceChange::none;
    if(std::find(devices.begin(),devices.end(),selected)==devices.end())return MenuMusicDeviceChange::disconnected;
    return selected==currentDefault?MenuMusicDeviceChange::none:MenuMusicDeviceChange::defaultChanged;
}

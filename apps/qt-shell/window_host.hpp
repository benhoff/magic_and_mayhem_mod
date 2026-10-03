#pragma once
#include <QSet>
#include <QString>
#include <QVector>
#include <xcb/xcb.h>

// A separate XCB connection is used only for discovery and liveness checks.
// Qt owns reparenting through QWindow/createWindowContainer.
class WindowHost {
public:
    WindowHost();
    ~WindowHost();
    WindowHost(const WindowHost&)=delete;
    WindowHost& operator=(const WindowHost&)=delete;
    bool available() const;
    QSet<xcb_window_t> windows() const;
    QVector<xcb_window_t> desktops(const QSet<xcb_window_t>& excluded) const;
    bool exists(xcb_window_t window) const;
    bool descendantOf(xcb_window_t window,xcb_window_t ancestor) const;
private:
    QString title(xcb_window_t window) const;
    xcb_connection_t* connection_=nullptr;
    xcb_window_t root_=0;
    xcb_atom_t netName_=0;
};

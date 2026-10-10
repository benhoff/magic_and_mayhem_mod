#pragma once
#include <QSet>
#include <QString>
#include <QVector>
#include <QSize>
#include <QPoint>
#include <xcb/xcb.h>

// A separate XCB connection discovers clients and sends targeted input events.
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
    bool lowerDesktop(xcb_window_t window) const;
    bool descendantOf(xcb_window_t window,xcb_window_t ancestor) const;
    xcb_window_t inputWindow(xcb_window_t desktop,QSize size) const;
    bool sendInput(xcb_window_t window,uint8_t type,uint8_t detail,uint16_t state,QPoint position) const;
private:
    QString title(xcb_window_t window) const;
    xcb_connection_t* connection_=nullptr;
    xcb_window_t root_=0;
    xcb_atom_t netName_=0;
};

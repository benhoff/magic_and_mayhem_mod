#pragma once
#include "gl_viewport.hpp"
#include "window_host.hpp"
#include <QObject>
#include <QSet>
#include <QHash>
#include "input_state.hpp"

// Targeted X11 message forwarding; does not change desktop focus or inject
// global input. Windows polling/DirectInput compatibility needs live evidence.
class InputForwarder final:public QObject {
public:
    InputForwarder(GlViewport& viewport,WindowHost& host);
    ~InputForwarder() override;
    void setTarget(xcb_window_t target);
    xcb_window_t target() const{return target_;}
    void release();
    void setState(InputState* state){release();state_=state;}
    void heartbeat();
    void suspend(bool value){if(value)release();suspended_=value;heartbeat();}
protected:
    bool eventFilter(QObject* object,QEvent* event) override;
private:
    bool send(uint8_t type,uint8_t detail,uint16_t state);
    uint16_t state(Qt::KeyboardModifiers modifiers) const;
    void modifiers(Qt::KeyboardModifiers modifiers,int key=0);
    GlViewport& viewport_;WindowHost& host_;xcb_window_t target_=0;
    QSet<uint8_t> keys_,buttons_;QPoint position_;int wheel_=0;
    bool suspended_=false;QHash<uint8_t,unsigned> virtualKeys_;InputState* state_=nullptr;
};

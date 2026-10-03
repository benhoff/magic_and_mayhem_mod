#include "input_forwarder.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace {
uint8_t button(Qt::MouseButton value){
    return value==Qt::LeftButton?1:value==Qt::MiddleButton?2:value==Qt::RightButton?3:0;
}
unsigned virtualKey(const QKeyEvent& key){
    const auto code=key.key();if(code>=Qt::Key_A && code<=Qt::Key_Z)return unsigned(code);
    if(code>=Qt::Key_0 && code<=Qt::Key_9)return (key.modifiers()&Qt::KeypadModifier)?unsigned(0x60+code-Qt::Key_0):unsigned(code);
    if(code>=Qt::Key_F1 && code<=Qt::Key_F24)return unsigned(0x70+code-Qt::Key_F1);
    switch(code){
    case Qt::Key_Backspace:return 8;case Qt::Key_Tab:return 9;case Qt::Key_Return:case Qt::Key_Enter:return 13;
    case Qt::Key_Shift:return 16;case Qt::Key_Control:return 17;case Qt::Key_Alt:return 18;case Qt::Key_Pause:return 19;
    case Qt::Key_Escape:return 27;case Qt::Key_Space:return 32;case Qt::Key_PageUp:return 33;case Qt::Key_PageDown:return 34;
    case Qt::Key_End:return 35;case Qt::Key_Home:return 36;case Qt::Key_Left:return 37;case Qt::Key_Up:return 38;
    case Qt::Key_Right:return 39;case Qt::Key_Down:return 40;case Qt::Key_Insert:return 45;case Qt::Key_Delete:return 46;
    case Qt::Key_Semicolon:case Qt::Key_Colon:return 0xba;case Qt::Key_Equal:case Qt::Key_Plus:return 0xbb;
    case Qt::Key_Comma:case Qt::Key_Less:return 0xbc;case Qt::Key_Minus:case Qt::Key_Underscore:return 0xbd;
    case Qt::Key_Period:case Qt::Key_Greater:return 0xbe;case Qt::Key_Slash:case Qt::Key_Question:return 0xbf;
    case Qt::Key_QuoteLeft:case Qt::Key_AsciiTilde:return 0xc0;case Qt::Key_BracketLeft:case Qt::Key_BraceLeft:return 0xdb;
    case Qt::Key_Backslash:case Qt::Key_Bar:return 0xdc;case Qt::Key_BracketRight:case Qt::Key_BraceRight:return 0xdd;
    case Qt::Key_Apostrophe:case Qt::Key_QuoteDbl:return 0xde;default:return 0;
    }
}
}
InputForwarder::InputForwarder(GlViewport& viewport,WindowHost& host):QObject(&viewport),viewport_(viewport),host_(host){viewport.installEventFilter(this);}
InputForwarder::~InputForwarder(){release();viewport_.removeEventFilter(this);}
void InputForwarder::setTarget(xcb_window_t target){if(target==target_)return;release();target_=target;}
uint16_t InputForwarder::state(Qt::KeyboardModifiers modifiers) const{
    uint16_t result=0;
    if(modifiers & Qt::ShiftModifier)result|=XCB_MOD_MASK_SHIFT;
    if(modifiers & Qt::ControlModifier)result|=XCB_MOD_MASK_CONTROL;
    if(modifiers & Qt::AltModifier)result|=XCB_MOD_MASK_1;
    if(modifiers & Qt::MetaModifier)result|=XCB_MOD_MASK_4;
    for(auto b:buttons_)result|=uint16_t(XCB_BUTTON_MASK_1<<(b-1));
    return result;
}
void InputForwarder::modifiers(Qt::KeyboardModifiers modifiers,int key){
    if(!state_ || !target_)return;
    const Qt::KeyboardModifier flags[]={Qt::ShiftModifier,Qt::ControlModifier,Qt::AltModifier};
    const int qtKeys[]={Qt::Key_Shift,Qt::Key_Control,Qt::Key_Alt};
    for(unsigned i=0;i<3;++i){const unsigned vk=16+i;
        state_->key(vk,virtualKeys_.values().contains(vk) || (key!=qtKeys[i] && bool(modifiers&flags[i])));}
}
bool InputForwarder::send(uint8_t type,uint8_t detail,uint16_t mask){
    if(host_.sendInput(target_,type,detail,mask,position_))return true;
    keys_.clear();buttons_.clear();virtualKeys_.clear();wheel_=0;target_=0;
    if(state_)state_->clear();
    return false;
}
void InputForwarder::release(){
    const auto keys=keys_,buttons=buttons_;
    for(auto key:keys)send(XCB_KEY_RELEASE,key,state({}));
    for(auto b:buttons){send(XCB_BUTTON_RELEASE,b,state({}));buttons_.remove(b);}
    keys_.clear();buttons_.clear();wheel_=0;
    virtualKeys_.clear();if(state_){state_->clear();state_->publish(false,position_,viewport_.frameSize());}
}
void InputForwarder::heartbeat(){if(state_)state_->publish(target_ && viewport_.hasFocus() && viewport_.isVisible(),position_,viewport_.frameSize());}
bool InputForwarder::eventFilter(QObject* object,QEvent* event){
    if(object!=&viewport_)return false;
    if(event->type()==QEvent::FocusOut || event->type()==QEvent::Hide || event->type()==QEvent::WindowDeactivate){release();return false;}
    if(!target_)return false;
    if(event->type()==QEvent::ShortcutOverride){event->accept();return true;}
    if(event->type()==QEvent::KeyPress || event->type()==QEvent::KeyRelease){
        auto* key=static_cast<QKeyEvent*>(event);const auto code=key->nativeScanCode();
        if(code<8 || code>255)return false;
        if(key->isAutoRepeat() && event->type()==QEvent::KeyRelease)return true;
        if(event->type()==QEvent::KeyPress){if(send(XCB_KEY_PRESS,uint8_t(code),state(key->modifiers()))){keys_.insert(uint8_t(code));virtualKeys_.insert(uint8_t(code),virtualKey(*key));if(state_ && virtualKey(*key))state_->key(virtualKey(*key),true);}}
        else if(keys_.contains(uint8_t(code))){send(XCB_KEY_RELEASE,uint8_t(code),state(key->modifiers()));keys_.remove(uint8_t(code));const auto vk=virtualKeys_.take(uint8_t(code));if(state_ && vk)state_->key(vk,virtualKeys_.values().contains(vk));}
        modifiers(key->modifiers(),key->key());heartbeat();
        event->accept();return true;
    }
    if(event->type()==QEvent::MouseMove || event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseButtonDblClick || event->type()==QEvent::MouseButtonRelease){
        auto* mouse=static_cast<QMouseEvent*>(event);const auto b=button(mouse->button());QPoint point;
        const bool releasing=event->type()==QEvent::MouseButtonRelease;
        if(!viewport_.imagePoint(mouse->position(),point,!buttons_.isEmpty()))return false;
        position_=point;
        if(event->type()==QEvent::MouseMove){send(XCB_MOTION_NOTIFY,0,state(mouse->modifiers()));}
        else if(!b)return false;
        else if(releasing){if(buttons_.contains(b)){send(XCB_BUTTON_RELEASE,b,state(mouse->modifiers()));buttons_.remove(b);}}
        else {viewport_.setFocus(Qt::MouseFocusReason);if(send(XCB_BUTTON_PRESS,b,state(mouse->modifiers())))buttons_.insert(b);}
        if(state_ && b && target_)state_->key(b==1?1:b==2?4:2,!releasing);
        modifiers(mouse->modifiers());heartbeat();
        event->accept();return true;
    }
    if(event->type()==QEvent::Wheel){
        auto* wheel=static_cast<QWheelEvent*>(event);QPoint point;
        if(!viewport_.imagePoint(wheel->position(),point))return false;
        position_=point;wheel_=int(qBound(qint64(-1920),qint64(wheel_)+wheel->angleDelta().y(),qint64(1920)));
        // Preserve sub-notch deltas; bound one dispatch to 16 notches.
        int count=qBound(-16,wheel_/120,16);wheel_-=count*120;
        while(count && target_){const uint8_t b=count>0?4:5;send(XCB_BUTTON_PRESS,b,state(wheel->modifiers()));
            if(target_)send(XCB_BUTTON_RELEASE,b,state(wheel->modifiers())|uint16_t(XCB_BUTTON_MASK_1<<(b-1)));
            count+=count>0?-1:1;}
        modifiers(wheel->modifiers());heartbeat();
        event->accept();return true;
    }
    return false;
}

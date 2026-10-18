#include "../apps/qt-shell/window_host.hpp"
#include <QGuiApplication>
#include <QByteArray>
#include <QElapsedTimer>
#include <cstdlib>
#include <cstdio>
#include <poll.h>

static void require(bool value,const char* message){if(!value){std::fprintf(stderr,"%s\n",message);std::exit(1);}}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    WindowHost host;
    if(argc==2 && QByteArray(argv[1])=="--bad-cookie"){
        require(!host.available(),"Bad private authentication fell back to the host display");
        std::puts("{\"success\":true,\"bad_cookie_refused\":true}");return 0;
    }
    require(host.available(),"Private WindowHost connection unavailable");
    auto cookie=QByteArray::fromHex(qgetenv("MNM_LEGACY_XCOOKIE"));char name[]="MIT-MAGIC-COOKIE-1";
    xcb_auth_info_t auth{int(sizeof(name)-1),name,int(cookie.size()),cookie.data()};
    int screen=0;auto* connection=xcb_connect_to_display_with_auth_info(qgetenv("MNM_LEGACY_DISPLAY").constData(),&auth,&screen);
    require(!xcb_connection_has_error(connection),"Private fixture connection failed");
    auto roots=xcb_setup_roots_iterator(xcb_get_setup(connection));while(screen-- && roots.rem)xcb_screen_next(&roots);
    require(roots.rem,"Private fixture screen missing");
    const auto desktop=xcb_generate_id(connection),target=xcb_generate_id(connection);
    xcb_create_window(connection,XCB_COPY_FROM_PARENT,desktop,roots.data->root,0,0,800,600,0,XCB_WINDOW_CLASS_INPUT_OUTPUT,roots.data->root_visual,0,nullptr);
    constexpr char title[]="MagicMayhem";
    xcb_change_property(connection,XCB_PROP_MODE_REPLACE,desktop,XCB_ATOM_WM_NAME,XCB_ATOM_STRING,8,sizeof(title)-1,title);
    const uint32_t mask=XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_KEY_RELEASE|XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE|XCB_EVENT_MASK_POINTER_MOTION;
    xcb_create_window(connection,XCB_COPY_FROM_PARENT,target,desktop,0,0,800,600,0,XCB_WINDOW_CLASS_INPUT_OUTPUT,roots.data->root_visual,XCB_CW_EVENT_MASK,&mask);
    xcb_map_window(connection,target);xcb_map_window(connection,desktop);xcb_flush(connection);
    auto* reply=xcb_get_input_focus_reply(connection,xcb_get_input_focus(connection),nullptr);require(reply,"Fixture server round trip failed");std::free(reply);
    require(host.desktops({}).contains(desktop),"Private desktop discovery failed");
    require(host.inputWindow(desktop,QSize(800,600))==target,"Private input target discovery failed");
    require(host.sendInput(target,XCB_KEY_PRESS,38,XCB_MOD_MASK_SHIFT,QPoint(12,13)),"Targeted private input failed");
    QElapsedTimer timer;timer.start();bool found=false;
    while(timer.elapsed()<1000 && !found){
        auto* event=xcb_poll_for_event(connection);
        if(event){
            if((event->response_type&127)==XCB_KEY_PRESS){
                auto* key=reinterpret_cast<xcb_key_press_event_t*>(event);
                found=key->event==target && key->detail==38 && key->state==XCB_MOD_MASK_SHIFT && key->event_x==12 && key->event_y==13;
            }
            std::free(event);
        }else{pollfd fd{xcb_get_file_descriptor(connection),POLLIN,0};poll(&fd,1,20);}
    }
    require(found,"Private client did not receive its targeted input event");
    xcb_destroy_window(connection,desktop);xcb_flush(connection);xcb_disconnect(connection);
    std::puts("{\"success\":true,\"private_discovery\":true,\"targeted_input_received\":true}");
}

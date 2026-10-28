#include "gl_viewport.hpp"
#include "input_forwarder.hpp"
#include <QApplication>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>

static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){
    QApplication app(argc,argv);
    int screen=0;auto* connection=xcb_connect(nullptr,&screen);
    if(xcb_connection_has_error(connection))return 2;
    auto screens=xcb_setup_roots_iterator(xcb_get_setup(connection));while(screen-- && screens.rem)xcb_screen_next(&screens);
    const auto root=screens.data->root;
    auto create=[&](xcb_window_t parent,int width,int height,bool input=true){
        const auto window=xcb_generate_id(connection);
        const uint32_t mask=input?(XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_KEY_RELEASE|XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE|XCB_EVENT_MASK_POINTER_MOTION):0;
        xcb_create_window(connection,XCB_COPY_FROM_PARENT,window,parent,0,0,width,height,0,XCB_WINDOW_CLASS_INPUT_OUTPUT,XCB_COPY_FROM_PARENT,XCB_CW_EVENT_MASK,&mask);
        xcb_map_window(connection,window);return window;
    };
    const auto desktop=create(root,800,600),target=create(desktop,800,600);
    constexpr char title[]="MagicMayhem";
    xcb_change_property(connection,XCB_PROP_MODE_REPLACE,desktop,XCB_ATOM_WM_NAME,XCB_ATOM_STRING,8,sizeof(title)-1,title);xcb_flush(connection);
    auto sync=[&]{auto* reply=xcb_get_input_focus_reply(connection,xcb_get_input_focus(connection),nullptr);std::free(reply);};sync();
    auto events=[&]{sync();std::vector<xcb_key_press_event_t> result;
        while(auto* event=xcb_poll_for_event(connection)){
            const auto type=event->response_type&0x7f;
            if(type>=XCB_KEY_PRESS && type<=XCB_MOTION_NOTIFY){
                require((event->response_type&0x80)!=0,"Expected targeted synthetic event");
                result.push_back(*reinterpret_cast<xcb_key_press_event_t*>(event));
            }
            std::free(event);
        }
        return result;
    };
    int status=0;
    try{
        WindowHost host;require(host.available(),"XCB host unavailable");
        require(host.desktops({}).contains(desktop),"Fixture discovery failed");
        require(host.inputWindow(desktop,{800,600})==target,"Must choose deepest client, not Wine desktop");
        create(target,800,600,false);sync();
        require(host.inputWindow(desktop,{800,600})==target,"Rendering-only child must not receive input");
        require(!host.inputWindow(desktop,{640,480}),"Mismatching resolution must not bind");
        GlViewport viewport;viewport.resize(1000,600);QImage frame(800,600,QImage::Format_RGBA8888);frame.fill(Qt::black);viewport.setFrame(frame);
        QTemporaryDir stateDirectory;InputState state;const auto statePath=stateDirectory.filePath("input.bin");
        require(state.create(statePath),"Input state creation");
        require(!state.desktopCoordinates(),"Ordinary client input must retain client-relative coordinates");
        InputForwarder input(viewport,host);input.setTarget(target);input.setState(&state);
        auto stateKey=[&](unsigned key){QFile file(statePath);require(file.open(QIODevice::ReadOnly),"Read input state");file.seek(64+key*4);const auto bytes=file.read(4);return qFromLittleEndian<quint32>(bytes.constData());};
        QPoint mapped;require(viewport.imagePoint({100,0},mapped) && mapped==QPoint(0,0),"Top-left mapping");
        require(viewport.imagePoint({899,599},mapped) && mapped==QPoint(799,599),"Bottom-right mapping");
        auto cursorState=[&]{QFile file(statePath);require(file.open(QIODevice::ReadOnly),"Read cursor state");const auto bytes=file.readAll();return QPoint(int(qFromLittleEndian<quint32>(bytes.constData()+24)),int(qFromLittleEndian<quint32>(bytes.constData()+28)));};
        require(!viewport.imagePoint({99,300},mapped) && !viewport.imagePoint({900,300},mapped),"Letterbox rejection");
        auto mouse=[&](QEvent::Type type,QPointF point,Qt::MouseButton button,Qt::MouseButtons buttons,Qt::KeyboardModifiers modifiers=Qt::NoModifier){
            QMouseEvent event(type,point,point,button,buttons,modifiers);QApplication::sendEvent(&viewport,&event);
        };
        auto key=[&](QEvent::Type type,bool repeat=false){QKeyEvent event(type,Qt::Key_A,Qt::ShiftModifier,38,0x61,0,"A",repeat);QApplication::sendEvent(&viewport,&event);};
        mouse(QEvent::MouseMove,{899,599},Qt::NoButton,{});events();
        require(cursorState()==QPoint(799,599),"Original high-resolution cursor");
        QImage smallFrame(640,480,QImage::Format_RGBA8888);smallFrame.fill(Qt::black);viewport.setFrame(smallFrame);input.heartbeat();
        require(cursorState()==QPoint(639,479),"Heartbeat constrains stale cursor after resolution shrink without a mouse event");
        for(auto type:{QEvent::KeyPress,QEvent::KeyRelease}){QKeyEvent event(type,Qt::Key_Right,Qt::NoModifier,114,0,0);QApplication::sendEvent(&viewport,&event);}auto resizedEvents=events();
        require(resizedEvents.size()==2&&resizedEvents[0].event_x==639&&resizedEvents[0].event_y==479,"First key after resolution shrink uses current coordinates");
        input.release();require(cursorState()==QPoint(639,479),"Inactive release keeps current cursor extent");
        viewport.setFrame(frame);input.heartbeat();require(cursorState()==QPoint(639,479),"Growing frame preserves the last constrained position");
        mouse(QEvent::MouseButtonPress,{50,300},Qt::LeftButton,Qt::LeftButton);require(events().empty(),"Bar click was forwarded");
        mouse(QEvent::MouseMove,{500,300},Qt::NoButton,{});
        auto received=events();require(received.size()==1 && received[0].event==target && received[0].event_x==400 && received[0].event_y==300,"Mouse center forwarding");
        viewport.resize(1001,701);
        for(auto mode:{GlViewport::Scaling::Sharp,GlViewport::Scaling::Smooth,GlViewport::Scaling::Integer}){
            viewport.setScaling(mode);const auto rect=viewport.imageRect();
            // Probe the center of a logical pixel, avoiding an exact texel boundary.
            mouse(QEvent::MouseMove,{rect.x()+400.5*rect.width()/800,rect.y()+300.5*rect.height()/600},Qt::NoButton,{});received=events();
            require(received.size()==1 && received[0].event_x==400 && received[0].event_y==300,"Scaled logical input forwarding");
            mouse(QEvent::MouseButtonPress,{rect.x()-1,rect.center().y()},Qt::LeftButton,Qt::LeftButton);
            require(events().empty(),"Scaled letterbox forwarded a click");
        }
        viewport.setScaling(GlViewport::Scaling::Sharp);viewport.resize(1000,600);
        mouse(QEvent::MouseButtonPress,{500,300},Qt::LeftButton,Qt::LeftButton,Qt::ControlModifier);
        mouse(QEvent::MouseMove,{999,700},Qt::NoButton,Qt::LeftButton);
        mouse(QEvent::MouseButtonRelease,{999,700},Qt::LeftButton,{});
        received=events();require(received.size()==3,"Drag sequence");
        require(stateKey(17)==1,"Mouse modifier polling state");
        require(received[0].detail==1 && received[0].state==XCB_MOD_MASK_CONTROL,"Button modifiers");
        require(received[1].event_x==799 && received[1].event_y==599 && received[1].state==XCB_BUTTON_MASK_1,"Clamp drag outside image");
        require((received[2].response_type&0x7f)==XCB_BUTTON_RELEASE && received[2].state==XCB_BUTTON_MASK_1,"Release state");
        key(QEvent::KeyPress);key(QEvent::KeyRelease,true);key(QEvent::KeyPress,true);key(QEvent::KeyRelease);
        require(stateKey(65)==1,"Windows key generation/repeat state");
        require(stateKey(16)==0x80000001,"Held modifier polling state");
        received=events();require(received.size()==3 && received[0].detail==38 && received[0].state==XCB_MOD_MASK_SHIFT,"Key scan code and repeat");
        require((received[1].response_type&0x7f)==XCB_KEY_PRESS && (received[2].response_type&0x7f)==XCB_KEY_RELEASE,"Repeat must retain key state");
        auto wheel=[&](int delta){QWheelEvent event({500,300},{500,300},{},{0,delta},{},Qt::NoModifier,Qt::NoScrollPhase,false);QApplication::sendEvent(&viewport,&event);};
        wheel(60);require(events().empty(),"Partial wheel notch");wheel(60);wheel(-120);
        received=events();require(received.size()==4 && received[0].detail==4 && received[2].detail==5,"Wheel direction and accumulation");
        mouse(QEvent::MouseButtonPress,{500,300},Qt::RightButton,Qt::RightButton);key(QEvent::KeyPress);events();
        QFocusEvent lost(QEvent::FocusOut);QApplication::sendEvent(&viewport,&lost);
        received=events();require(received.size()==2,"Focus loss must release keys and buttons");
        require(stateKey(65)==2 && !(stateKey(2)&0x80000000u),"Polling state cleanup");
        mouse(QEvent::MouseButtonPress,{500,300},Qt::MiddleButton,Qt::MiddleButton);events();input.setTarget(0);
        received=events();require(received.size()==1 && received[0].detail==2 && (received[0].response_type&0x7f)==XCB_BUTTON_RELEASE,"Unbind must release held button");
        key(QEvent::KeyPress);require(events().empty(),"Unbound viewport forwarded input");
        input.setTarget(target);key(QEvent::KeyPress);events();input.suspend(true);
        received=events();require(received.size()==1 && (received[0].response_type&0x7f)==XCB_KEY_RELEASE,"Movie must release held input");
        require(!(stateKey(65)&0x80000000u),"Movie must clear polling keys");
        key(QEvent::KeyPress);mouse(QEvent::MouseMove,{500,300},Qt::NoButton,{});wheel(120);
        require(events().empty(),"Movie must suppress game input");
        input.suspend(false);key(QEvent::KeyPress);key(QEvent::KeyRelease);
        require(events().size()==2,"Game input must resume after movie");input.setTarget(0);
        const auto competing=create(desktop,800,600);sync();require(!host.inputWindow(desktop,{800,600}),"Ambiguous clients must not bind");
        xcb_destroy_window(connection,competing);sync();input.setTarget(target);
        xcb_destroy_window(connection,target);sync();key(QEvent::KeyPress);require(!input.target(),"Destroyed target must disable forwarding");
        require(!host.inputWindow(desktop,{800,600}),"Wine desktop must not substitute for game client");
        // Wine's windowed outer window includes decorations: the displayed
        // primary frame remains 800x600. Desktop coordinates must be translated
        // to this outer X11 window, not scaled to its dimensions.
        const auto decorated=create(desktop,646,512);
        constexpr char gameTitle[]="Magic & Mayhem";
        xcb_change_property(connection,XCB_PROP_MODE_REPLACE,decorated,XCB_ATOM_WM_NAME,XCB_ATOM_STRING,8,sizeof(gameTitle)-1,gameTitle);
        const uint32_t location[]={77,44};xcb_configure_window(connection,decorated,XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y,location);sync();
        require(host.desktopInputWindow(desktop,{800,600})==decorated,"Decorated original window discovery");
        require(!host.desktopInputWindow(desktop,{640,480}),"Unknown frame space must refuse");
        input.setTarget(decorated,desktop);
        mouse(QEvent::MouseMove,{500,300},Qt::NoButton,{});received=events();
        require(received.size()==1 && received[0].event_x==323 && received[0].event_y==256,"Desktop to outer-window translation");
        // A normal move must be observed on the next event, without rebinding.
        const uint32_t moved[]={uint32_t(-323),44};xcb_configure_window(connection,decorated,XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y,moved);sync();
        mouse(QEvent::MouseMove,{500,300},Qt::NoButton,{});received=events();
        require(received.size()==1 && received[0].event_x==723 && received[0].event_y==256,"Moved target translation");
        const auto duplicate=create(desktop,640,480);xcb_change_property(connection,XCB_PROP_MODE_REPLACE,duplicate,XCB_ATOM_WM_NAME,XCB_ATOM_STRING,8,sizeof(gameTitle)-1,gameTitle);sync();
        require(!host.desktopInputWindow(desktop,{800,600}),"Ambiguous named game windows must refuse");
        xcb_destroy_window(connection,duplicate);sync();input.setTarget(0);xcb_destroy_window(connection,decorated);sync();
        require(!host.desktopInputWindow(desktop,{800,600}),"Desktop cannot replace retired game window");
        const auto smallerClient=create(desktop,640,480);sync();
        require(host.inputWindow(desktop,{640,480})==smallerClient,"Ordinary 640 client in 800 desktop must bind by client extent");
        require(!host.desktopInputWindow(desktop,{640,480}),"Client frame must not be misclassified as desktop coordinates");
        xcb_destroy_window(connection,smallerClient);sync();
        InputState desktopState;const auto desktopPath=stateDirectory.filePath("desktop.bin");require(desktopState.create(desktopPath,true),"Desktop input creation");
        require(desktopState.desktopCoordinates(),"Explicit desktop wire must retain its selector coordinate mode");
        desktopState.publish(true,{400,300},{800,600});QFile desktopFile(desktopPath);require(desktopFile.open(QIODevice::ReadOnly),"Desktop header open");
        const auto desktopBytes=desktopFile.readAll();require(desktopBytes.left(8)=="MNMINK02" && qFromLittleEndian<quint32>(desktopBytes.constData()+8)==2 && qFromLittleEndian<quint32>(desktopBytes.constData()+40)==1,"Explicit desktop wire identity");
        viewport.resize(1001,601);const auto rect=viewport.imageRect();
        require(viewport.imagePoint(rect.center(),mapped) && mapped==QPoint(400,300),"Resized/high-DPI center");
        input.setState(nullptr);
        if(argc==2){
            InputState fixture;const auto fixturePath=stateDirectory.filePath("fixture.bin");
            require(fixture.create(fixturePath),"Fixture input state");fixture.key(65,true);fixture.publish(true,{400,300},{800,600});
            QFile source(fixturePath),output(QString::fromLocal8Bit(argv[1]));
            require(source.open(QIODevice::ReadOnly) && output.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Export input snapshot");
            require(output.write(source.readAll())==InputState::Size,"Export input bytes");
        }
        std::puts("Qt targeted input: discovery, keys, repeat, drag, wheel, focus cleanup, resize and failure passed");
    }catch(const std::exception& error){std::fprintf(stderr,"Input test failed: %s\n",error.what());status=1;}
    xcb_destroy_window(connection,desktop);xcb_disconnect(connection);return status;
}

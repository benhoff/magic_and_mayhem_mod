#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QThread>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>
static void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
struct LiveCommandSessionTestAccess {
    static void exercise(LiveCommandSession& session){
        session.deadline_.start();QThread::msleep(10);
        session.setMediaActive(true);const auto before=session.waitingElapsed();
        QThread::msleep(30);session.setMediaActive(true);
        check(session.waitingElapsed()==before,"movie must suspend initial frame budget");
        check(session.poll()&&session.state()==LiveCommandSession::State::WaitingFrame,"movie must not request rendering recovery");
        session.setMediaActive(false);const auto resumed=session.waitingElapsed();
        check(resumed>=before&&resumed<before+10,"completion preserves remaining frame budget");
        session.setMediaActive(false);QThread::msleep(15);
        check(session.waitingElapsed()>=resumed+10,"frame deadline resumes and completion is idempotent");
        session.setMediaActive(true);QThread::msleep(20);session.setMediaActive(false);
        check(session.mediaPausedMs_>=45,"sequential movies accumulate paused time");
    }
};
int main(int argc,char** argv)try{
    QApplication app(argc,argv);GlViewport viewport;LiveCommandSession session(viewport);QTemporaryDir directory;
    check(directory.isValid()&&session.create(directory.filePath("commands.bin"),123,2),"create owned rendering channel");
    LiveCommandSessionTestAccess::exercise(session);
    std::cout<<"Native movie pauses rendering frame deadline; no game or output device\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}

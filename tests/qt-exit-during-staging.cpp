/* LD_PRELOAD fixture: simulate an application deadline while its real Qt
 * launcher is staging. The preparer is a sleeping fixture, not the game. */
#include <QCoreApplication>
#include <QTimer>
#include <dlfcn.h>
#include <cstdlib>

extern "C" int fixtureExec() asm("_ZN16QCoreApplication4execEv");
int fixtureExec(){
    auto original=reinterpret_cast<int(*)()>(dlsym(RTLD_NEXT,"_ZN16QCoreApplication4execEv"));
    if(!original)std::abort();
    QTimer::singleShot(500,QCoreApplication::instance(),[]{QCoreApplication::exit(1);});
    return original();
}

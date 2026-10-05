#include "command_replay.hpp"
#include <QApplication>
#include <QSurfaceFormat>
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    if(argc<2 || argc>3)return 2;
    return runCommandReplay(QString::fromLocal8Bit(argv[1]),true,argc==3 && QByteArray(argv[2])=="--command-checks");
}

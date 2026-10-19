#include "../apps/qt-shell/cpu_reply_file.hpp"
#include <QCoreApplication>
#include <QSaveFile>
#include <QtEndian>
#include <cstring>

int main(int argc,char** argv){
    QCoreApplication app(argc,argv);
    if(argc!=3)return 2;
    const QString path=QString::fromLocal8Bit(argv[2]);
    QByteArray reply(64+800*600*2,0);
    std::memcpy(reply.data(),"MNMCREAD",8);
    const quint32 fields[]={1,64,501,1,0,2,800,600,960000,7,0,5,6,1};
    for(unsigned i=0;i<14;++i)qToLittleEndian(fields[i],reply.data()+8+i*4);
    for(unsigned i=0;i<480000;++i)qToLittleEndian(quint16(i*71+19),reply.data()+64+i*2);
    if(std::strcmp(argv[1],"durable")==0){
        QSaveFile file(path);
        return file.open(QIODevice::WriteOnly) && file.write(reply)==reply.size() && file.commit()?0:1;
    }
    if(std::strcmp(argv[1],"volatile")!=0)return 2;
    return publishCpuReplyFile(path,reply)?0:1;
}

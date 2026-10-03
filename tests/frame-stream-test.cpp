#include "../apps/qt-shell/frame_stream.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstring>
#include <stdexcept>
static void check(bool value){if(!value)throw std::runtime_error("Frame stream guard failed");}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir directory;check(directory.isValid());
    const auto path=directory.filePath("frame.bin");FrameStream reader;check(reader.create(path));
    check(reader.nextFrame().isNull());
    QFile file(path);check(file.open(QIODevice::ReadWrite));auto* mapping=file.map(0,FrameStream::Size);check(mapping);
    auto* words=reinterpret_cast<quint32*>(mapping);
    const auto put=[&](int index,quint32 value){qToLittleEndian(value,mapping+index*4);};
    for(quint32 status=0;status<=10;++status){put(9,status);check(reader.status()==status && !reader.diagnostic().isEmpty());}
    put(9,8);put(11,0x887600ff);check(reader.diagnostic().contains("0x887600ff"));
    put(9,5);check(reader.diagnostic().contains("hook armed"));
    put(9,6);check(reader.diagnostic().contains("--software-rendering"));
    put(9,9);check(reader.diagnostic().contains("could not install"));
    put(4,1);put(5,4);put(6,2);put(7,16);put(8,1);put(9,1);put(10,1);
    std::memset(mapping+64,255,32);
    check(reader.nextFrame().isNull());
    __atomic_store_n(words+4,qToLittleEndian<quint32>(2),__ATOMIC_RELEASE);
    auto frame=reader.nextFrame();check(frame.width()==4 && frame.height()==2 && frame.pixelColor(0,0)==QColor(Qt::white));
    check(reader.nextFrame().isNull());
    put(10,2);put(5,2049);put(4,4);check(reader.nextFrame().isNull());
    put(5,4);put(7,17);put(4,6);check(reader.nextFrame().isNull());
    put(7,16);put(4,8);check(!reader.nextFrame().isNull());
    file.unmap(mapping);file.close();
    const auto bad=directory.filePath("bad.bin");QFile invalid(bad);check(invalid.open(QIODevice::WriteOnly));check(invalid.resize(FrameStream::Size));invalid.close();
    FrameStream rejected;check(!rejected.open(bad));
}

#include "commands.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <stdexcept>

int main(int argc,char** argv){
    if(argc==2 && QByteArray(argv[1])=="--help"){
        std::puts("Usage: mnm-render-commands COMMANDS --output NATIVE_PIXELS [--preview PNG]");return 0;
    }
    QJsonObject report;
    try {
        if((argc!=4 && argc!=6) || QByteArray(argv[2])!="--output" || (argc==6 && QByteArray(argv[4])!="--preview"))
            throw std::runtime_error("Expected COMMANDS --output NATIVE_PIXELS [--preview PNG]");
        const auto output=QString::fromLocal8Bit(argv[3]);const auto preview=argc==6?QString::fromLocal8Bit(argv[5]):QString{};
        QFile input(QString::fromLocal8Bit(argv[1]));
        if(!input.open(QIODevice::ReadOnly) || input.size()>mnm::render::maxCommandBytes)throw std::runtime_error("Cannot read bounded command stream");
        const auto data=input.read(mnm::render::maxCommandBytes+1);
        const auto commands=mnm::render::decodeCommands(data);
        QGuiApplication app(argc,argv);const auto result=mnm::render::replayCommands(commands);
        QFile destination(output);
        if(!destination.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(destination.errorString().toStdString());
        if(destination.write(result.native)!=result.native.size() || !destination.flush()){
            destination.remove();throw std::runtime_error("Failed to write command output");}
        if(!preview.isEmpty()){
            QFile png(preview);
            if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(png.errorString().toStdString());
            if(!result.presentation.save(&png,"PNG") || !png.flush()){png.remove();throw std::runtime_error("Failed to write command preview");}
        }
        report={{"rendered",true},{"commands",int(commands.size())},{"checks",int(result.checks)},{"presentations",int(result.presents)},
            {"backend","opengl_native_integer"},{"vendor",QString::fromStdString(result.driver.vendor)},
            {"renderer",QString::fromStdString(result.driver.renderer)},{"version",QString::fromStdString(result.driver.version)},
            {"command_sha256",QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex())},
            {"output_sha256",QString::fromLatin1(QCryptographicHash::hash(result.native,QCryptographicHash::Sha256).toHex())},
            {"presentation_rgba_sha256",QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(result.presentation.constBits()),result.presentation.sizeInBytes()),QCryptographicHash::Sha256).toHex())},
            {"surface_stats",QJsonObject{{"uploads",qint64(result.stats.uploads)},{"copies",qint64(result.stats.copies)},
                {"palette_updates",qint64(result.stats.paletteUpdates)},{"surfaces",int(result.stats.surfaces)}}}};
        std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 0;
    }catch(const std::exception& e){report["rendered"]=false;report["error"]=e.what();
        std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 2;}
}

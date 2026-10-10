#include "../apps/qt-shell/native_playback.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QSaveFile>
#include <QFile>
#include <QTimer>
#include <stdexcept>
#include <iostream>
int main(int argc,char **argv) try {
  QGuiApplication app(argc,argv);
  if(argc!=5)throw std::runtime_error("Expected movie, frame limit (0 for complete), report, RGBA output");
  QFile pixels(QString::fromLocal8Bit(argv[4]));
  if(!pixels.open(QIODevice::WriteOnly | QIODevice::NewOnly))throw std::runtime_error("Pixel output exists or cannot open");
  bool ok=false;const auto limit=QString::fromLocal8Bit(argv[2]).toUInt(&ok);
  if(!ok)throw std::runtime_error("Invalid frame limit");
  NativePlayback player(true,false);unsigned frames=0;QImage first;
  QByteArray firstHash;bool opaque=true;
  auto hash=[](const QImage &im) {
    QCryptographicHash h(QCryptographicHash::Sha256);
    for(int y=0;y<im.height();++y)h.addData(QByteArrayView(reinterpret_cast<const char *>(im.constScanLine(y)),im.width()*4));
    return h.result().toHex();
  };
  player.frame=[&](QImage image) {
    if(first.isNull()){first=image;firstHash=hash(first);}
    for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x)
      opaque=opaque && image.constScanLine(y)[x*4+3]==255;
    for(int y=0;y<image.height();++y)
      if(pixels.write(reinterpret_cast<const char *>(image.constScanLine(y)),image.width()*4)!=image.width()*4)
        throw std::runtime_error("Owned pixel output failed");
    ++frames;
    if(limit && frames>=limit)player.stop();
  };
  player.finished=[&](unsigned status) {
    auto result=player.report();const bool passed=!player.active() && opaque && frames>0 &&
      hash(first)==firstHash && result.value("video_frames").toInt()==int(frames) &&
      status==(limit ? MNM_MEDIA_V1_STATUS_CANCELLED : MNM_MEDIA_V1_STATUS_COMPLETE);
    result.insert("owned_first_frame_retained",hash(first)==firstHash);
    result.insert("all_frames_opaque",opaque);result.insert("inactive_after_completion",!player.active());
    result.insert("success",passed);
    QSaveFile file(QString::fromLocal8Bit(argv[3]));
    const bool saved=file.open(QIODevice::WriteOnly) && file.write(QJsonDocument(result).toJson())>0 && file.commit();
    app.exit(passed && saved ? 0 : 1);
  };
  QTimer::singleShot(0,&app,[&]{player.start(QString::fromLocal8Bit(argv[1]));});
  QTimer::singleShot(240000,&app,[&]{player.stop(MNM_MEDIA_V1_STATUS_DECODER_ERROR);});
  return app.exec();
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}

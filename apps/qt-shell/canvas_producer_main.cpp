#include "canvas_producer_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <iostream>
int main(int argc, char **argv) {
  QSurfaceFormat format; format.setVersion(3,3); format.setProfile(QSurfaceFormat::CoreProfile); QSurfaceFormat::setDefaultFormat(format);
  QApplication app(argc, argv);
  bool handoff=false,batch=false;QString proof;
  if(argc<4){std::cerr<<"Usage: mnm-canvas-producers-live INPUT.bin GAME_ROOT OUTPUT_DIRECTORY [--world-handoff] [--world-batch] [--world-proof PIPE]\n";return 2;}
  for(int i=4;i<argc;++i){const auto arg=QString::fromLocal8Bit(argv[i]);if(arg=="--world-handoff")handoff=true;else if(arg=="--world-batch")batch=true;else if(arg=="--world-proof"&&i+1<argc)proof=QString::fromLocal8Bit(argv[++i]);else{std::cerr<<"Unknown native producer option\n";return 2;}}
  try {
    GlViewport viewport; viewport.setWindowTitle("Magic & Mayhem native canvas sequence"); viewport.resize(800,600); viewport.show();
    CanvasProducerSession session(viewport, QString::fromLocal8Bit(argv[1]), QString::fromLocal8Bit(argv[2]), QString::fromLocal8Bit(argv[3]), handoff,batch,proof); session.start();
    return app.exec();
  } catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}

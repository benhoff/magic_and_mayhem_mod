#include "canvas_producer_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <iostream>
int main(int argc, char **argv) {
  QSurfaceFormat format; format.setVersion(3,3); format.setProfile(QSurfaceFormat::CoreProfile); QSurfaceFormat::setDefaultFormat(format);
  QApplication app(argc, argv);
  if (argc != 4 && !(argc==5 && QString::fromLocal8Bit(argv[4])=="--world-handoff")) { std::cerr << "Usage: mnm-canvas-producers-live INPUT.bin GAME_ROOT OUTPUT_DIRECTORY [--world-handoff]\n"; return 2; }
  try {
    GlViewport viewport; viewport.setWindowTitle("Magic & Mayhem native canvas sequence"); viewport.resize(800,600); viewport.show();
    CanvasProducerSession session(viewport, QString::fromLocal8Bit(argv[1]), QString::fromLocal8Bit(argv[2]), QString::fromLocal8Bit(argv[3]), argc==5); session.start();
    return app.exec();
  } catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}

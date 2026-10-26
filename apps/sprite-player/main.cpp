#include "sprite_window.hpp"
#include <QApplication>
#include <iostream>
int main(int argc,char** argv)try {
    QApplication app(argc,argv);const auto args=app.arguments();
    if(args.size()<2 || args.size()>3) {std::cerr<<"Usage: mnm-sprite-player INSTALLATION [RELATIVE_TAG]\n";return 2;}
    mnm::ui::SpriteWindow window(std::filesystem::path(args[1].toStdString()),args.size()==3?args[2]:QString());
    window.show();return app.exec();
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}

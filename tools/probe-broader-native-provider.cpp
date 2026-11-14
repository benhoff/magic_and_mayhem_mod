#include "../renderer/blit.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <stdexcept>
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);
 mnm::render::GlBlitter renderer;
 const auto driver=renderer.driver();
 const mnm::render::Image input{3,2,{0x1234,0x5678,31,0xf800,0x7e0,0xffff}};
 const auto surface=renderer.create(input,{16,{0xf800,0x7e0,31}});
 if(renderer.read(surface).pixels!=input.pixels)throw std::runtime_error("Independent WORD read differs");
 renderer.destroy(surface);
 QJsonObject result{{"schema",1},{"success",true},{"vendor",QString::fromStdString(driver.vendor)},
  {"renderer",QString::fromStdString(driver.renderer)},{"version",QString::fromStdString(driver.version)},
  {"independent_word_check",true},{"original_game_runs",0}};
 const auto json=QJsonDocument(result).toJson(QJsonDocument::Compact);std::puts(json.constData());return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}

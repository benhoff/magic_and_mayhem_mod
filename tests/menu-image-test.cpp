#include "menu_assets.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>
static void require(bool value,const char* message) {if (!value) throw std::runtime_error(message);}
int main(int argc,char** argv) {
    QApplication app(argc,argv);
    try {
        QTemporaryDir temporary;require(temporary.isValid(),"fixture root");
        const auto root=temporary.path();
        QImage source(7,3,QImage::Format_RGB888);
        for (int y=0;y<3;++y) for (int x=0;x<7;++x) source.setPixelColor(x,y,QColor(x*30,y*70,20+x+y));
        require(source.save(root+"/odd.BMP","BMP"),"24-bit BMP fixture");
        auto image=mnm::ui::loadMenuImage(root,"odd.bmp",QSize(7,3));
        for (int y=0;y<3;++y) for(int x=0;x<7;++x) require(image.pixelColor(x,y)==source.pixelColor(x,y),"RGB channels, row padding and bottom-up orientation");
        QFile file(root+"/odd.BMP");require(file.open(QIODevice::WriteOnly)&&file.write("bad")==3,"replace file");file.close();
        require(image.pixelColor(6,2)==source.pixelColor(6,2),"Qt copy owns decoded pixels after result/file lifetime");
        const auto rejected=[&](const QString& path,const QSize& size) {try {(void)mnm::ui::loadMenuImage(root,path,size);return false;}catch(const std::exception&){return true;}};
        require(rejected("odd.bmp",QSize(7,3)),"corrupt BMP rejected");
        require(source.save(root+"/odd.BMP","BMP"),"restore BMP");
        require(rejected("odd.BMP",QSize(6,3))&&rejected("odd.BMP",QSize(8,3)),"smaller limit and larger mismatched dimensions rejected");
        require(rejected("../odd.BMP",QSize(7,3))&&rejected("odd.BMP",QSize(0,3))&&rejected("odd.BMP",QSize(2000,3)),"paths and caller dimensions bounded");
        QImage jpeg(13,9,QImage::Format_RGB888);jpeg.fill(QColor(90,130,170));require(jpeg.save(root+"/sample.jpeg","JPG"),"JPEG fixture");
        const auto decoded=mnm::ui::loadMenuImage(root,"sample.jpeg",jpeg.size());
        require(decoded.size()==jpeg.size()&&qAbs(decoded.pixelColor(2,2).red()-90)<3,"JPEG loader presentation");
        require(rejected("sample.jpeg",QSize(14,9))&&rejected("missing.jpg",QSize(13,9))&&rejected("sample.png",QSize(13,9)),"JPEG size, missing and unsupported images rejected");
        QFile renamed(root+"/fake.jpg");require(QFile::copy(root+"/odd.BMP",renamed.fileName()),"wrong codec fixture");
        require(rejected("fake.jpg",QSize(7,3)),"format-specific decoding rejects fake JPEG");
        return 0;
    } catch (const std::exception& failure) {std::fprintf(stderr,"Menu image test: %s\n",failure.what());return 1;}
}

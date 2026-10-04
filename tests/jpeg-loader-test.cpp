#include "jpeg.hpp"
#include <QBuffer>
#include <QCoreApplication>
#include <QImage>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void requireAt(bool ok,int line) {if(!ok) throw std::runtime_error("JPEG assertion failed at line "+std::to_string(line));}
#define require(ok) requireAt((ok),__LINE__)
std::vector<std::uint8_t> fixture() {
    QImage image(3,2,QImage::Format_RGB888);image.fill(QColor(Qt::red));
    QByteArray bytes;QBuffer buffer(&bytes);require(buffer.open(QIODevice::WriteOnly));
    require(image.save(&buffer,"JPEG",100));
    return {reinterpret_cast<const std::uint8_t*>(bytes.constData()),reinterpret_cast<const std::uint8_t*>(bytes.constData())+bytes.size()};
}
void error(const std::vector<std::uint8_t>& bytes,JpegErrorCode code,const JpegLimits& limits={}) {
    const auto result=decodeJpeg(bytes,limits);require(std::holds_alternative<JpegError>(result));
    const auto& failure=std::get<JpegError>(result);
    if(failure.code!=code) throw std::runtime_error("Unexpected JPEG error: "+failure.detail+"; expected "+std::to_string(int(code)));
}
}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    const auto good=fixture();auto bytes=good;
    const auto decoded=decodeJpeg(bytes);require(std::holds_alternative<JpegImage>(decoded));
    auto image=std::get<JpegImage>(decoded);bytes.clear();
    require(image.width==3 && image.height==2 && image.rgb.size()==18 && image.sourceBytes==good.size());
    for(std::size_t p=0;p<image.rgb.size();p+=3)
        require(image.rgb[p]>=253 && image.rgb[p+1]<=2 && image.rgb[p+2]<=2);
    error({},JpegErrorCode::invalidFormat);
    error({0xff,0xd8},JpegErrorCode::malformedData);
    error({0xff,0xd8,0xff,0xd9},JpegErrorCode::malformedData);
    bytes=good;bytes[0]=0;error(bytes,JpegErrorCode::invalidFormat);
    bytes=good;bytes.back()=0;error(bytes,JpegErrorCode::malformedData);
    bytes=good;bytes.push_back(0);error(bytes,JpegErrorCode::malformedData);
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        require(std::holds_alternative<JpegError>(decodeJpeg(bytes)));
    }
    JpegLimits limits;limits.inputBytes=good.size()-1;error(good,JpegErrorCode::limitExceeded,limits);
    limits={};limits.width=2;error(good,JpegErrorCode::limitExceeded,limits);
    limits={};limits.height=1;error(good,JpegErrorCode::limitExceeded,limits);
    limits={};limits.pixels=5;error(good,JpegErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=17;error(good,JpegErrorCode::limitExceeded,limits);
    limits={};limits.codecImageBytes=23;error(good,JpegErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=18;limits.codecImageBytes=24;
    require(std::holds_alternative<JpegImage>(decodeJpeg(good,limits)));
    // Mutate SOF dimensions: the limit must reject before allocating pixels.
    bytes=good;bool found=false;
    for(std::size_t p=0;p+8<bytes.size();++p) {
        if(bytes[p]==0xff && bytes[p+1]==0xc0) {
            bytes[p+5]=0xea;bytes[p+6]=0x60;bytes[p+7]=0xea;bytes[p+8]=0x60;
            found=true;break;
        }
    }
    require(found);error(bytes,JpegErrorCode::limitExceeded);
    std::cout<<"JPEG ownership, RGB, complete framing, truncation and preallocation limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}

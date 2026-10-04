#include "jpeg.hpp"
#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(JpegErrorCode code,const std::string& detail) {
    throw JpegError{code,detail,std::nullopt};
}
}
JpegResult decodeJpeg(const std::vector<std::uint8_t>& bytes,const JpegLimits& limits) try {
    if(bytes.size()>limits.inputBytes || bytes.size()>std::uint64_t(std::numeric_limits<qsizetype>::max()))
        fail(JpegErrorCode::limitExceeded,"JPEG input limit exceeded");
    if(bytes.size()<2 || bytes[0]!=0xff || bytes[1]!=0xd8)
        fail(JpegErrorCode::invalidFormat,"Missing JPEG SOI marker");
    if(bytes.size()<4 || bytes[bytes.size()-2]!=0xff || bytes.back()!=0xd9)
        fail(JpegErrorCode::malformedData,"Missing terminal JPEG EOI marker");
    const auto formats=QImageReader::supportedImageFormats();
    if(!formats.contains("jpeg") && !formats.contains("jpg"))
        fail(JpegErrorCode::codecUnavailable,"Qt JPEG codec unavailable");
    QByteArray encoded=QByteArray::fromRawData(reinterpret_cast<const char*>(bytes.data()),static_cast<qsizetype>(bytes.size()));
    QBuffer buffer(&encoded);
    if(!buffer.open(QIODevice::ReadOnly)) fail(JpegErrorCode::malformedData,"Cannot open JPEG memory buffer");
    QImageReader reader(&buffer,"jpeg");
    reader.setAutoDetectImageFormat(false);reader.setAutoTransform(false);
    const auto dimensions=reader.size();
    if(!dimensions.isValid() || dimensions.width()<=0 || dimensions.height()<=0)
        fail(JpegErrorCode::malformedData,reader.errorString().toStdString());
    const auto width=static_cast<std::uint32_t>(dimensions.width());
    const auto height=static_cast<std::uint32_t>(dimensions.height());
    const std::uint64_t pixels=std::uint64_t(width)*height;
    if(width>limits.width || height>limits.height || pixels>limits.pixels || pixels>limits.decodedBytes/3 || pixels>limits.codecImageBytes/4)
        fail(JpegErrorCode::limitExceeded,"JPEG dimensions, RGB or codec image limit exceeded");
    // Preflight before codec pixel allocation. Keep Qt's process allocation limit
    // intact; it is an additional backend constraint, not a per-call setting.
    QImage decoded=reader.read();
    if(decoded.isNull()) fail(JpegErrorCode::malformedData,reader.errorString().toStdString());
    if(decoded.size()!=dimensions) fail(JpegErrorCode::malformedData,"JPEG decoded dimensions changed");
    if(std::uint64_t(decoded.sizeInBytes())>limits.codecImageBytes)
        fail(JpegErrorCode::limitExceeded,"JPEG codec image exceeded budget");
    QImage converted=decoded.convertToFormat(QImage::Format_RGB888);
    if(converted.isNull()) fail(JpegErrorCode::limitExceeded,"JPEG RGB conversion allocation failed");
    if(std::uint64_t(converted.sizeInBytes())>limits.codecImageBytes)
        fail(JpegErrorCode::limitExceeded,"JPEG converted image exceeded budget");
    JpegImage result;result.width=width;result.height=height;result.sourceBytes=bytes.size();
    if(pixels>result.rgb.max_size()/3) fail(JpegErrorCode::limitExceeded,"JPEG RGB output cannot fit in memory");
    result.rgb.resize(static_cast<std::size_t>(pixels*3));
    const auto rowBytes=static_cast<std::size_t>(width)*3;
    for(std::uint32_t y=0;y<height;++y)
        std::copy_n(converted.constScanLine(static_cast<int>(y)),rowBytes,result.rgb.begin()+static_cast<std::ptrdiff_t>(std::size_t(y)*rowBytes));
    return result;
} catch(const JpegError& error) {return error;}
  catch(const std::bad_alloc&) {return JpegError{JpegErrorCode::limitExceeded,"JPEG allocation failed",std::nullopt};}
  catch(const std::length_error&) {return JpegError{JpegErrorCode::limitExceeded,"JPEG allocation too large",std::nullopt};}
JpegResult loadJpeg(AssetFile& file,const JpegLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return JpegError{JpegErrorCode::invalidArgument,"JPEG input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes)) return JpegError{JpegErrorCode::assetInput,error->detail,*error};
    return decodeJpeg(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}

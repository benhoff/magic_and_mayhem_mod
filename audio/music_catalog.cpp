#include "music_catalog.hpp"
#include "../assets/asset_file.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
namespace mnm::audio {
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::uint64_t integer(const QJsonValue& value,std::uint64_t maximum){
    require(value.isDouble(),"Missing numeric music metadata");const auto number=value.toDouble();
    require(std::isfinite(number)&&number>=0&&number<=double(maximum)&&number==std::floor(number),"Invalid music metadata integer");
    return std::uint64_t(number);
}
std::unique_ptr<mnm::assets::AssetFile> open(const mnm::assets::AssetStore& store,const std::string& path){
    auto result=store.open(path);require(std::holds_alternative<std::unique_ptr<mnm::assets::AssetFile>>(result),"Cannot open prepared music asset");
    return std::move(std::get<std::unique_ptr<mnm::assets::AssetFile>>(result));
}
void validateWave(mnm::assets::AssetFile& file,std::uint64_t bytes,const QString& expectedHash){
    const auto size=file.size();require(std::holds_alternative<std::int64_t>(size)&&std::get<std::int64_t>(size)==std::int64_t(bytes+44),"CD WAV size differs from catalog");
    std::array<unsigned char,44> header{};const auto result=mnm::assets::readExact(file,header.data(),header.size());
    require(!result.error&&result.transferred==44,"Truncated CD WAV header");
    require(!std::memcmp(header.data(),"RIFF",4)&&!std::memcmp(header.data()+8,"WAVEfmt ",8)&&!std::memcmp(header.data()+36,"data",4),"Unsupported prepared CD WAV layout");
    require(qFromLittleEndian<quint32>(header.data()+4)==bytes+36&&qFromLittleEndian<quint32>(header.data()+16)==16&&
            qFromLittleEndian<quint16>(header.data()+20)==1&&qFromLittleEndian<quint16>(header.data()+22)==2&&
            qFromLittleEndian<quint32>(header.data()+24)==44100&&qFromLittleEndian<quint32>(header.data()+28)==176400&&
            qFromLittleEndian<quint16>(header.data()+32)==4&&qFromLittleEndian<quint16>(header.data()+34)==16&&
            qFromLittleEndian<quint32>(header.data()+40)==bytes,"Prepared music must be exact stereo signed16 PCM44100");
    QCryptographicHash hash(QCryptographicHash::Sha256);hash.addData(QByteArrayView(reinterpret_cast<const char*>(header.data()),header.size()));
    std::array<char,65536> block{};std::uint64_t remaining=bytes;
    while(remaining){const auto count=std::min<std::uint64_t>(remaining,block.size());const auto read=mnm::assets::readExact(file,block.data(),std::int64_t(count));
        require(!read.error&&read.transferred==std::int64_t(count),"Truncated CD WAV PCM");hash.addData(QByteArrayView(block.data(),count));remaining-=count;}
    require(expectedHash.size()==64&&QString::fromLatin1(hash.result().toHex())==expectedHash,"Prepared CD WAV hash differs from catalog");
}
}
MusicCatalog MusicCatalog::load(const std::filesystem::path& root){
    auto created=mnm::assets::AssetStore::create(root);require(std::holds_alternative<mnm::assets::AssetStore>(created),"Invalid native music root");
    const auto store=std::get<mnm::assets::AssetStore>(std::move(created));auto manifest=open(store,"manifest.json");
    auto bytes=mnm::assets::readWhole(*manifest,1024*1024);require(std::holds_alternative<std::vector<std::uint8_t>>(bytes),"Invalid music manifest input");
    const auto& contents=std::get<std::vector<std::uint8_t>>(bytes);QJsonParseError error;
    const auto document=QJsonDocument::fromJson(QByteArray(reinterpret_cast<const char*>(contents.data()),qsizetype(contents.size())),&error);
    require(error.error==QJsonParseError::NoError&&document.isObject(),"Malformed music catalog JSON");
    const auto object=document.object();require(integer(object.value("schema"),1)==1&&object.value("success").isBool()&&object.value("success").toBool(),"Unsuccessful music preparation manifest");
    const auto format=object.value("format").toObject();require(integer(format.value("channels"),2)==2&&integer(format.value("bits"),16)==16&&integer(format.value("rate"),44100)==44100&&format.value("byte_order").toString()=="little","Invalid CD PCM metadata");
    require(object.value("tracks").isArray(),"Missing CD track table");const auto rows=object.value("tracks").toArray();require(!rows.empty()&&rows.size()<=98,"Invalid CD audio track count");
    MusicCatalog catalog;std::uint32_t next=2,previousEnd=0;
    for(const auto& value:rows){require(value.isObject(),"Invalid CD track entry");const auto row=value.toObject();
        const auto number=integer(row.value("track"),99),start=integer(row.value("start_sector"),std::numeric_limits<std::uint32_t>::max()),end=integer(row.value("end_sector"),std::numeric_limits<std::uint32_t>::max());
        require(number==next&&start<end&&start>0&&(!previousEnd||start==previousEnd),"Noncontiguous or invalid CD track range");
        const auto pcm=integer(row.value("pcm_bytes"),std::numeric_limits<std::uint32_t>::max()-36),frames=integer(row.value("frames"),std::numeric_limits<std::uint32_t>::max());
        require(pcm==(end-start)*2352&&frames==pcm/4,"CD sector/PCM size mismatch");
        const auto name=QString("Track%1.wav").arg(next,2,10,QChar('0'));
        require(row.value("file").toString()==name,"Redirected CD track filename");auto file=open(store,name.toStdString());validateWave(*file,pcm,row.value("wav_sha256").toString());
        if(next==2)catalog.tracks_.push_back({1,0,std::uint32_t(start),start*588,{}});
        catalog.tracks_.push_back({next,std::uint32_t(start),std::uint32_t(end),frames,store.root()/name.toStdString()});previousEnd=std::uint32_t(end);++next;
    }
    return catalog;
}
const MusicTrack* MusicCatalog::track(std::uint32_t number)const{
    if(!number||number>tracks_.size())return nullptr;
    return &tracks_[number-1];
}
}

#include "map.hpp"
#include <new>
#include <stdexcept>
namespace mnm::assets {
const MapCell& MapAsset::cell(std::uint32_t x,std::uint32_t y,std::uint32_t z) const {
    if(x>=width || y>=height || z>=layers)throw std::out_of_range("MAP coordinate outside owned grid");
    return cells.at((std::size_t(z)*height+y)*width+x);
}
MapResult decodeMapPayload(const Bytes& bytes,const MapLimits& limits)try{
    const auto fail=[](PersistenceErrorCode code,std::size_t at,const char* message)->MapResult{return PersistenceError{code,at,message,{}};};
    if(bytes.size()>limits.decodedBytes)return fail(PersistenceErrorCode::limitExceeded,0,"MAP decoded byte limit exceeded");
    if(bytes.size()<76)return fail(PersistenceErrorCode::malformedData,0,"MAP requires a 76-byte header");
    const auto word=[&](std::size_t at){return std::uint32_t(bytes[at])|(std::uint32_t(bytes[at+1])<<8)|(std::uint32_t(bytes[at+2])<<16)|(std::uint32_t(bytes[at+3])<<24);};
    if(word(0)!=6)return fail(PersistenceErrorCode::unsupportedVersion,0,"Only MAP version 6 is supported");
    MapAsset map;map.width=word(4);map.height=word(8);map.layers=word(12);
    if(!map.width || !map.height || !map.layers)return fail(PersistenceErrorCode::malformedData,4,"MAP dimensions must be nonzero");
    if(map.width>limits.dimension || map.height>limits.dimension || map.layers>limits.layers)return fail(PersistenceErrorCode::limitExceeded,4,"MAP dimension/layer limit exceeded");
    const auto plane=std::uint64_t(map.width)*map.height;
    if(plane>limits.cells || map.layers>limits.cells/plane)return fail(PersistenceErrorCode::limitExceeded,20,"MAP cell limit exceeded");
    const auto count=plane*map.layers;
    if(word(16)!=plane || word(20)!=count || 76+12*count!=bytes.size())return fail(PersistenceErrorCode::malformedData,16,"MAP dimensions, strides and payload extent disagree");
    for(unsigned i=0;i<13;++i)map.metadata[i]=word(24+4*i);
    map.cells.resize(count);
    const auto half=[&](std::size_t at){return std::uint16_t(bytes[at])|(std::uint16_t(bytes[at+1])<<8);};
    for(std::size_t i=0;i<count;++i){const auto at=76+12*i;auto& cell=map.cells[i];cell.definition=half(at);for(unsigned j=0;j<3;++j)cell.references[j]=half(at+2+2*j);cell.flags8=half(at+8);cell.flags10=half(at+10);}
    return map;
}catch(const std::bad_alloc&){return PersistenceError{PersistenceErrorCode::limitExceeded,0,"MAP allocation failed",{}};}
MapResult decodeMap(const Bytes& bytes,const MapLimits& limits){
    PersistenceLimits container;container.inputBytes=limits.inputBytes;container.decodedBytes=limits.decodedBytes;
    auto decoded=decodePackedContainer(bytes,container,ContainerTransform::cfgBytes);
    if(const auto* error=std::get_if<PersistenceError>(&decoded))return *error;
    return decodeMapPayload(std::get<PackedContainer>(decoded).decoded,limits);
}
MapResult loadMap(AssetFile& file,const MapLimits& limits){
    auto bytes=readWhole(file,limits.inputBytes);
    if(const auto* error=std::get_if<Error>(&bytes))return PersistenceError{PersistenceErrorCode::assetInput,0,error->detail,*error};
    return decodeMap(std::get<Bytes>(bytes),limits);
}
}

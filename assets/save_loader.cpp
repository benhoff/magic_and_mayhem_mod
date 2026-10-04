#include "persistence_internal.hpp"

namespace mnm::assets {
namespace {
using namespace persistence_detail;
RealmState realmState(const Bytes& bytes) {
    if(bytes.size()!=0x13dc) fail(PersistenceErrorCode::malformedData,0,"realm block must be 0x13dc bytes");
    RealmState r;r.raw=bytes;r.name=fixedString(bytes,0,256);r.nextRealm=fixedString(bytes,0x10c,256);
    r.playerWizard=signedWord(bytes,0x100);r.wizardCount=signedWord(bytes,0x104);r.lastRegion=signedWord(bytes,0x108);
    // Parse all storage slots. Do not apply config defaults or discard dormant slots.
    for(std::size_t i=0;i<80;++i) {
        r.wizardNames[i]=fixedString(bytes,0x84c+i*16,16);
        r.icons[i]=signedWord(bytes,0xe8c+i*4);r.eligibility[i]=signedWord(bytes,0xd4c+i*4);
        r.locations[i]=signedWord(bytes,0xfcc+i*4);r.requestedLocations[i]=signedWord(bytes,0x110c+i*4);r.flags[i]=signedWord(bytes,0x124c+i*4);
    }
    for(std::size_t i=0;i<20;++i) r.regionOwners[i]=signedWord(bytes,0x138c+i*4);
    return r;
}
}
PersistenceResult<RealmState> decodeRealmState(const Bytes& bytes,const PersistenceLimits& limits) {
    return guarded<RealmState>([&]{cap(bytes.size(),limits.decodedBytes);return realmState(bytes);});
}
PersistenceResult<RealmState> loadRealmState(AssetFile& f,const PersistenceLimits& l) {return persistence_detail::load<RealmState>(f,l,[&](const Bytes& b){return decodeRealmState(b,l);});}
PersistenceResult<SavedGame> decodeSavedGame(const Bytes& bytes,const PersistenceLimits& limits) {
    return guarded<SavedGame>([&] {
        cap(bytes.size(),limits.decodedBytes);Cursor c{bytes};SavedGame save;
        if(c.u32()!=0x00564153) fail(PersistenceErrorCode::malformedData,0,"expected SAV signature");
        save.version=c.u32();if(save.version!=20) fail(PersistenceErrorCode::unsupportedVersion,4,"only version-20 saves supported");
        save.accountingValue=c.u32();save.paths=c.take(0x200);
        for(auto& w:save.wizards) {
            w.prefix=c.take(0x14a);
            auto count=c.u32();cap(count,42,c.p-4);c.require(std::size_t(count)*20);
            w.entries.resize(count);for(auto& e:w.entries) for(auto& v:e) v=c.u32();
            w.records=c.take(1000);for(auto& v:w.listHeader) v=c.u32();
            count=c.u32();cap(count,limits.listWords,c.p-4);c.require(std::size_t(count)*4);
            w.list.resize(count);for(auto& v:w.list) v=c.u32();
            auto flag=c.u8();if(flag>1) fail(PersistenceErrorCode::malformedData,c.p-1,"invalid extension presence flag");
            if(flag) w.extension=c.take(0x57c);
        }
        save.realm=realmState(c.take(0x13dc));save.controller=c.take(0x16c);save.globals=c.take(0x18);save.scriptState=c.take(0x3268);
        for(auto& v:save.counters) v=c.u32();
        save.worldMarker=c.u32();
        if(save.worldMarker==0x17) {save.worldGlobals=c.take(0x18);save.worldTail=c.take(bytes.size()-c.p);}
        else if(save.worldMarker!=0) fail(PersistenceErrorCode::malformedData,c.p-4,"unknown world marker");
        if(c.p!=bytes.size()) fail(PersistenceErrorCode::malformedData,c.p,"unexpected trailing campaign bytes");
        return save;
    });
}
PersistenceResult<SavedGame> loadSavedGame(AssetFile& file,bool packed,const PersistenceLimits& limits) {
    return persistence_detail::load<SavedGame>(file,limits,[&](const Bytes& b)->PersistenceResult<SavedGame> {
        if(!packed) return decodeSavedGame(b,limits);
        auto result=decodePackedContainer(b,limits,ContainerTransform::noCdSave);if(auto* e=std::get_if<PersistenceError>(&result)) return *e;
        return decodeSavedGame(std::get<PackedContainer>(result).decoded,limits);
    });
}
}

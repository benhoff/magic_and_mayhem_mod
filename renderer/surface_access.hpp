#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace mnm::render {
// Single-thread admission model, not a COM surface or a borrowed pixel pointer.
// Descriptor/DC values are caller-owned opaque 32-bit tokens, never host pointers.
class SurfaceAccessState final {
public:
    using Descriptor=std::array<std::uint32_t,27>;
    static constexpr std::uint32_t busy=0x887601ae,notLocked=0x88760248;
    static constexpr std::uint32_t noDc=0x8876024a,dcAlreadyCreated=0x8876026c,badDc=0x8876086c;
private:
    Descriptor layout_{};
    std::uint32_t dcToken_,storageDcToken_=0;
    int balance_=0;
    bool dc_=false;
    void adjust(int delta){
        if(balance_+delta < -64 || balance_+delta > 64)
            throw std::runtime_error("Surface access sequence budget exceeded");
        balance_+=delta;
    }
public:
    SurfaceAccessState(unsigned width,unsigned height,unsigned bits,unsigned caps,
                       std::uint32_t storageToken,std::uint32_t dcToken):dcToken_(dcToken){
        if(!width || !height || width>2048 || height>2048 ||
           (bits!=8 && bits!=16 && bits!=32) || (caps!=0x840 && caps!=0x40) ||
           !storageToken || !dcToken || storageToken==dcToken ||
           storageToken==0xabababab || dcToken==0xabababab)
            throw std::runtime_error("Unsupported owned surface access context");
        layout_[0]=108;layout_[1]=0x100f;layout_[2]=height;layout_[3]=width;
        layout_[4]=(width*(bits/8)+3)&~3u;layout_[9]=storageToken;
        layout_[18]=32;layout_[19]=bits==8?0x60:0x40;layout_[21]=bits;
        layout_[22]=bits==16?0xf800:bits==32?0xff0000:0;
        layout_[23]=bits==16?0x7e0:bits==32?0xff00:0;
        layout_[24]=bits==16?0x1f:bits==32?0xff:0;layout_[26]=caps==0x840?0x840:0x10004040;
    }
    SurfaceAccessState(const SurfaceAccessState&)=delete;
    SurfaceAccessState& operator=(const SurfaceAccessState&)=delete;
    void exchangeStorageLease(SurfaceAccessState& other){
        if(layout_[2]!=other.layout_[2] || layout_[3]!=other.layout_[3] || layout_[21]!=other.layout_[21] || poisoned() || other.poisoned())
            throw std::runtime_error("Incompatible storage lease exchange");
        std::swap(balance_,other.balance_);std::swap(storageDcToken_,other.storageDcToken_);std::swap(layout_[9],other.layout_[9]);
    }
    bool dcMatchesStorage() const {return dc_ && storageDcToken_==dcToken_;}
    bool storageDcActive() const {return storageDcToken_!=0;}
    bool storageBorrowed() const {return balance_!=0 || storageDcToken_!=0;}
    bool dcActive() const {return dc_;}
    bool borrowed() const {return dc_ || balance_!=0 || storageDcToken_!=0;}
    bool poisoned() const {return balance_<0;}
    int mapBalance() const {return balance_;}
    std::uint32_t lock(Descriptor& output){
        // Busy Lock clears the descriptor. Its trailing caps word is not a
        // stable driver output on this failure branch; use owned caps policy.
        if(balance_!=0 || dc_ || storageDcToken_){output={};output[0]=108;output[26]=layout_[26];return busy;}
        adjust(1);output=layout_;return 0;
    }
    std::uint32_t unlock(){
        if(balance_==0)return notLocked;
        adjust(-1);return 0;
    }
    std::uint32_t acquireDc(std::uint32_t& output){
        if(dc_)return dcAlreadyCreated; // Failed GetDC preserves output token.
        if(storageDcToken_)throw std::runtime_error("DC acquire on storage with a foreign lease is unvalidated");
        adjust(1);dc_=true;storageDcToken_=dcToken_;output=dcToken_;return 0;
    }
    std::uint32_t releaseDc(std::uint32_t suppliedToken){
        if(!dc_)return noDc;
        if(suppliedToken!=dcToken_ || storageDcToken_!=dcToken_)return badDc;
        // Represent an over-unlocked DC as explicit signed debt, never unsigned
        // arithmetic wraparound or an actual invalid memory mapping.
        adjust(-1);dc_=false;storageDcToken_=0;return 0;
    }
};
}

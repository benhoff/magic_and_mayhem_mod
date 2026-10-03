#include "commands.hpp"
#include "capture.hpp"
#include <QtEndian>
#include <map>
#include <set>
#include <stdexcept>

namespace mnm::render {
namespace {
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
struct Description {int width,height;PixelFormat format;};
void formatValid(PixelFormat f){
    require(f.bits==8 || f.bits==16 || f.bits==24 || f.bits==32,"Unsupported command pixel format");
    if(f.bits==8){require(f.masks==std::array<std::uint32_t,3>{},"Indexed masks must be zero");return;}
    const auto maximum=f.bits==32?UINT32_MAX:((1u<<f.bits)-1);
    for(auto mask:f.masks){const auto low=mask&(~mask+1),normal=low?mask/low:0;
        require(mask && mask<=maximum && normal<=255 && !(normal&(normal+1)),"Unsupported command RGB mask");}
    require(!(f.masks[0]&f.masks[1]) && !(f.masks[0]&f.masks[2]) && !(f.masks[1]&f.masks[2]),"Overlapping command masks");
}
Image pixels(const QByteArray& payload,qsizetype start,unsigned w,unsigned h,unsigned bits){
    require(w && h && w<=2048 && h<=2048,"Invalid command image dimensions");
    require(payload.size()-start==qsizetype(w)*h*(bits/8),"Invalid command pixel length");
    Image image{int(w),int(h),{}};image.pixels.reserve(std::size_t(w)*h);
    while(start<payload.size()){
        std::uint32_t value=0;
        for(unsigned byte=0;byte<bits/8;++byte)value|=std::uint32_t(static_cast<unsigned char>(payload[start++]))<<(byte*8);
        image.pixels.push_back(value);
    }
    return image;
}
}
std::vector<SurfaceCommand> decodeCommands(const QByteArray& data){
    require(data.size()>=16 && data.size()<=maxCommandBytes && data.first(8)=="MNMCMD01","Invalid command stream header");
    const auto word=[](const QByteArray& b,qsizetype at){return qFromLittleEndian<quint32>(b.constData()+at);};
    require(word(data,8)==1 && word(data,12)==16,"Unsupported command stream version");
    std::map<unsigned,Description> live;std::set<unsigned> used;
    std::size_t pixelCount=0;bool ended=false,presented=false;
    std::vector<SurfaceCommand> commands;qsizetype at=16;
    while(at<data.size()){
        require(!ended && commands.size()<4096 && data.size()-at>=12,"Truncated, oversized or trailing command stream");
        SurfaceCommand c;c.operation=word(data,at);const auto serial=word(data,at+4),length=word(data,at+8);at+=12;
        require(serial==commands.size()+1 && length<=unsigned(data.size()-at),"Command sequence gap or truncated payload");
        const auto p=data.mid(at,length);at+=length;
        const auto fields=[&](unsigned n){require(length>=n*4,"Short command fields");for(unsigned i=0;i<n;++i)c.words[i]=word(p,i*4);};
        const auto get=[&](unsigned id)->const Description& {const auto i=live.find(id);require(i!=live.end(),"Unknown or destroyed surface ID");return i->second;};
        if(c.operation==1){
            fields(7);const auto id=c.words[0],w=c.words[1],h=c.words[2];
            require(id && !used.count(id) && live.size()<64,"Reused, zero or excessive surface ID");
            c.format={c.words[3],{c.words[4],c.words[5],c.words[6]}};formatValid(c.format);
            c.image=pixels(p,28,w,h,c.format.bits);
            require(pixelCount+std::size_t(w)*h<=16777216,"Command surface pixel budget exceeded");
            pixelCount+=std::size_t(w)*h;used.insert(id);live.emplace(id,Description{int(w),int(h),c.format});
        }else if(c.operation==2){
            fields(5);const auto& s=get(c.words[0]);
            c.image=pixels(p,20,c.words[3],c.words[4],s.format.bits);
            require(c.image.width<=s.width && c.image.height<=s.height &&
                    c.words[1]<=unsigned(s.width-c.image.width) && c.words[2]<=unsigned(s.height-c.image.height),"Update outside surface");
        }else if(c.operation==3){
            fields(10);require(length==40,"Invalid copy length");const auto& s=get(c.words[0]);const auto& d=get(c.words[1]);
            require(c.words[0]!=c.words[1] && s.format.bits==d.format.bits && s.format.masks==d.format.masks,"Aliased or incompatible copy");
            const auto& w=c.words;
            require(w[2]<w[4] && w[3]<w[5] && w[4]<=unsigned(s.width) && w[5]<=unsigned(s.height) &&
                    w[4]-w[2]<=unsigned(d.width) && w[5]-w[3]<=unsigned(d.height) &&
                    w[6]<=unsigned(d.width)-(w[4]-w[2]) && w[7]<=unsigned(d.height)-(w[5]-w[3]),"Copy outside surface");
            require(w[8]<=1 && (w[8] || !w[9]) && (s.format.bits==32 || w[9]<(1u<<s.format.bits)),"Invalid copy key");
        }else if(c.operation==4){
            fields(3);const auto& s=get(c.words[0]);const auto first=c.words[1],count=c.words[2];
            require(s.format.bits==8 && count && first<256 && count<=256-first && length==12+count*3,"Invalid palette command");
            for(unsigned i=0;i<count;++i)c.colors.push_back({static_cast<std::uint8_t>(p[12+i*3]),
                static_cast<std::uint8_t>(p[13+i*3]),static_cast<std::uint8_t>(p[14+i*3])});
        }else if(c.operation==5){
            fields(1);const auto& s=get(c.words[0]);
            require(length==4+unsigned(s.width*s.height)*(s.format.bits/8),"Invalid check length");c.expected=p.mid(4);
        }else if(c.operation==6 || c.operation==7){
            fields(1);require(length==4,"Invalid surface command length");const auto s=get(c.words[0]);
            if(c.operation==6)presented=true;
            else {pixelCount-=std::size_t(s.width)*s.height;live.erase(c.words[0]);}
        }else if(c.operation==8){
            require(!length && live.empty() && presented,"Incomplete command session");ended=true;
        }else if(c.operation==10){
            fields(1);const auto& s=get(c.words[0]);
            require(length==4+unsigned(s.width*s.height)*4,"Invalid RGBA check length");c.expected=p.mid(4);
        }else if(c.operation==9){
            fields(1);require(length==4,"Invalid capture-gap record");
            throw std::runtime_error("Capture history contains a gap (reason "+std::to_string(c.words[0])+")");
        }else throw std::runtime_error("Unsupported command opcode");
        commands.push_back(std::move(c));
    }
    require(ended,"Missing command stream END");return commands;
}
CommandResult replayCommands(const std::vector<SurfaceCommand>& commands){
    GlBlitter gl;std::map<unsigned,SurfaceId> handles;std::map<unsigned,unsigned> bits;CommandResult result;
    for(const auto& c:commands){const auto& w=c.words;
        switch(c.operation){
        case 1:handles.emplace(w[0],gl.create(c.image,c.format));bits.emplace(w[0],c.format.bits);break;
        case 2:gl.update(handles.at(w[0]),int(w[1]),int(w[2]),c.image);break;
        case 3:gl.copy(handles.at(w[0]),handles.at(w[1]),{int(w[2]),int(w[3]),int(w[4]),int(w[5])},int(w[6]),int(w[7]),
                      w[8]?std::optional<std::uint32_t>(w[9]):std::nullopt);break;
        case 4:gl.setPalette(handles.at(w[0]),w[1],c.colors);break;
        case 5:require(encodeNative(gl.read(handles.at(w[0])),bits.at(w[0]))==c.expected,"Original native pixels disagree with command replay");++result.checks;break;
        case 6:result.native=encodeNative(gl.read(handles.at(w[0])),bits.at(w[0]));result.presentation=gl.present(handles.at(w[0]));++result.presents;break;
        case 7:gl.destroy(handles.at(w[0]));handles.erase(w[0]);bits.erase(w[0]);break;
        case 8:break;
        case 10:{const auto image=gl.present(handles.at(w[0]));
            require(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes())==c.expected,
                    "Original RGBA colors disagree with command replay");++result.colorChecks;break;}
        default:throw std::runtime_error("Unsupported replay command");
        }
    }
    result.stats=gl.stats();result.driver=gl.driver();return result;
}
}

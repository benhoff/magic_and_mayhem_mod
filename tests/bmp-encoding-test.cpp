#include "bmp.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <filesystem>
using namespace mnm::assets;
namespace {
unsigned checks=0,refusals=0;
void require(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
std::vector<std::uint8_t> bytes(const std::filesystem::path& p){
    std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Missing fixture");
    return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};
}
void put(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t value){for(unsigned i=0;i<4;++i)b.at(p+i)=std::uint8_t(value>>(8*i));}
void bad(const std::vector<std::uint8_t>& b){require(std::holds_alternative<BmpError>(decodeBmp(b)),"Malformed BMP admitted");++refusals;}
}
int main(int argc,char** argv)try{
    require(argc==2,"Require fixture root");const std::filesystem::path root=argv[1];
    std::ifstream manifest(root/"cases.txt");require(bool(manifest),"Missing cases");
    std::string name;unsigned width,height,cases=0;std::uint64_t mutations=0;std::uint32_t rng=0x891234;
    while(manifest>>name>>width>>height){
        auto b=bytes(root/(name+".bmp"));const auto expected=bytes(root/(name+".rgb"));
        auto result=decodeBmp(b);require(std::holds_alternative<BmpImage>(result),name.c_str());
        auto image=std::get<BmpImage>(std::move(result));
        require(image.width==width&&image.height==height,"Decoded dimensions differ");
        require(image.rgb==expected,"Decoded complete RGB differs");
        require(image.sourceBytes==b.size(),"Encoded extent metadata differs");
        auto copy=b;b.assign(b.size(),0);require(image.rgb==expected,"Output borrows source bytes");b=copy;
        BmpLimits limit;limit.decodedBytes=expected.size()-1;
        require(std::holds_alternative<BmpError>(decodeBmp(b,limit)),"Decoded budget ignored");
        limit={};limit.inputBytes=b.size()-1;
        require(std::holds_alternative<BmpError>(decodeBmp(b,limit)),"Input budget ignored");
        for(std::size_t n=0;n<b.size();++n)bad({b.begin(),b.begin()+static_cast<std::ptrdiff_t>(n)});
        // Deterministic hostile-input smoke: success must still have a complete
        // bounded RGB image. Sanitizers check memory safety, not equivalence.
        for(unsigned trial=0;trial<256;++trial){
            copy=b;rng=rng*1664525+1013904223;const auto at=rng%copy.size();rng=rng*1664525+1013904223;copy[at]^=std::uint8_t(rng>>24);
            const auto mutated=decodeBmp(copy);++mutations;
            if(const auto* im=std::get_if<BmpImage>(&mutated))require(im->rgb.size()==std::uint64_t(im->width)*im->height*3,"Mutation returned partial image");
        }
        ++cases;
    }
    const auto rle=bytes(root/"rle8-mixed.bmp");
    auto b=rle;put(b,22,0xfffffffd);bad(b);b=rle;put(b,34,0);bad(b);
    // Crafted commands cover row crossing, invalid palette references, delta
    // escape bounds, truncated literal padding, missing end, and excess EOL.
    const auto stream=[&](std::initializer_list<std::uint8_t> data){
        auto v=rle;v.resize(70);v.insert(v.end(),data);put(v,2,v.size());put(v,34,data.size());bad(v);
    };
    stream({14,1,0,1});stream({1,4,0,1});stream({0,2,14,0,0,1});stream({0,2,0,3,0,1});
    stream({0,3,0,1,2});stream({1,1});stream({0,0,0,0,0,0,0,0,0,1});
    const auto rle4=bytes(root/"rle4-mixed.bmp");
    const auto stream4=[&](std::initializer_list<std::uint8_t> data,bool valid=false){
        auto v=rle4;v.resize(70);v.insert(v.end(),data);put(v,2,v.size());put(v,34,data.size());
        if(valid)require(std::holds_alternative<BmpImage>(decodeBmp(v)),"Unused low nibble interpreted as a pixel");
        else bad(v);
    };
    stream4({1,0x40,0,1});stream4({2,0x14,0,1});stream4({1,0x14,0,1},true);
    stream4({0,3,0x12,0x40,0,1});stream4({0,5,0x12,0x30,0x10});
    b=bytes(root/"masks16-bottom-13.bmp");put(b,54,0);bad(b);
    b=bytes(root/"masks16-bottom-13.bmp");put(b,54,0xf801);bad(b);
    b=bytes(root/"masks16-bottom-13.bmp");put(b,54,0x1f0000);bad(b);
    b=bytes(root/"masks16-bottom-13.bmp");put(b,58,0xf800);bad(b);
    b=bytes(root/"indexed8-bottom-13.bmp");put(b,46,257);bad(b);
    b=bytes(root/"indexed8-bottom-13.bmp");b[70]=4;bad(b);
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"checks\":"<<checks<<",\"refusals\":"<<refusals<<",\"mutations\":"<<mutations<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

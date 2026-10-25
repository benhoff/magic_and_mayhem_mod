#include "resource_cache.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Require fixture root");
    const std::filesystem::path root=argv[1];assets::ResourceManager resources(store(root));
    render::GlBlitter renderer;const auto baseline=renderer.stats();unsigned cases=0;
    std::ifstream manifest(root/"cases.txt");require(bool(manifest),"Missing fixture manifest");
    std::string name;unsigned width,height;
    while(manifest>>name>>width>>height){
        const assets::ResourceId id{assets::ResourceKind::ui,name};
        resources.bind(id,{assets::ResourceImageFormat::bmp,name+".bmp",{},{},{}});
        const auto& owned=resources.load(id);const auto revision=owned.revision;
        std::ifstream expectedFile(root/(name+".rgb"),std::ios::binary);
        const Bytes rgb{std::istreambuf_iterator<char>(expectedFile),std::istreambuf_iterator<char>()};
        require(std::get<assets::BmpImage>(owned.image).rgb==rgb,"Manager changed decoded RGB");
        const auto prepared=render::prepareResourceUpload(owned,{id,revision,0,{},{},false});
        require(prepared.pixels.width==int(width)&&prepared.pixels.height==int(height),"Upload dimensions differ");
        std::vector<std::uint32_t> expected;
        for(std::size_t p=0;p<rgb.size();p+=3)expected.push_back((unsigned(rgb[p]>>3)<<11)|(unsigned(rgb[p+1]>>2)<<5)|(rgb[p+2]>>3));
        require(prepared.pixels.pixels==expected,"RGB565 conversion differs");
        {
            render::ResourceCache cache(renderer,resources);
            const auto dest=renderer.create({int(width),int(height),std::vector<std::uint32_t>(width*height,0xdead)},render::spriteFormat);
            cache.draw(id,0,dest,0,0);require(renderer.read(dest).pixels==expected,"GPU bitmap pixels differ");
            const auto before=renderer.stats();cache.draw(id,0,dest,0,0);const auto after=renderer.stats();
            require(before.uploads==after.uploads&&before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Resident draw uploaded or read back");
            cache.retire(id);require(!resources.isResident(id)&&resources.stats().decodedBytes==0,"Retirement retained decoded image");
            require(resources.load(id).revision>revision,"Reload reused retired revision");
            resources.unload(id);renderer.destroy(dest);
        }
        require(renderer.stats().surfaces==baseline.surfaces&&renderer.stats().pixels==baseline.pixels,"GPU surfaces leaked");
        ++cases;
    }
    // Existing manager must not publish an invalid decode or consume its budget.
    const assets::ResourceId bad{assets::ResourceKind::ui,"bad"};
    resources.bind(bad,{assets::ResourceImageFormat::bmp,"malformed.bmp",{},{},{}});
    rejected([&]{resources.load(bad);});require(!resources.isResident(bad)&&resources.stats().decodedBytes==0,"Failed decode published a resource");
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"gpu_round_trips\":"<<cases<<",\"resident_reuse\":"<<cases<<",\"retire_reload\":"<<cases<<",\"failure_atomic\":true,\"surface_leaks\":0}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

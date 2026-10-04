#include "dsound_setup.hpp"
#include "wave_loader.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc,char** argv){
    QCoreApplication app(argc,argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Read installed PCM WAV through the native asset interface and dump uploaded samples.");
    parser.addHelpOption();
    parser.addPositionalArgument("PCM.wav","Host WAV path (alternative to --assets/--path).","[PCM.wav]");
    parser.addOption({"assets","Installation root for a game path.","directory"});
    parser.addOption({"path","Windows-style asset path under --assets.","path"});
    parser.addOption({"prefix","Windows installation prefix (repeatable, requires --assets).","prefix"});
    parser.addOption({"dump","Raw PCM output outside the asset root.","file"});
    parser.process(app);
    const auto positional=parser.positionalArguments();
    const bool explicitRoot=parser.isSet("assets") || parser.isSet("path");
    if(!parser.isSet("dump") || (explicitRoot && (!parser.isSet("assets") || !parser.isSet("path") || !positional.isEmpty())) ||
       (!explicitRoot && (positional.size()!=1 || parser.isSet("prefix"))))parser.showHelp(2);
    try {
        const QFileInfo hostInput(explicitRoot?QString{}:positional.first());
        const auto root=explicitRoot?QFileInfo(parser.value("assets")).filesystemAbsoluteFilePath():hostInput.filesystemAbsolutePath();
        const auto request=(explicitRoot?parser.value("path"):hostInput.fileName()).toStdString();
        std::vector<std::string> prefixes;
        for(const auto& prefix:parser.values("prefix"))prefixes.push_back(prefix.toStdString());
        auto configured=mnm::assets::AssetStore::create(root,prefixes);
        if(const auto* error=std::get_if<mnm::assets::Error>(&configured))throw mnm::audio::AssetInputError(*error);
        const auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
        const auto dump=QFileInfo(parser.value("dump")).filesystemAbsoluteFilePath();
        const auto canonicalDump=std::filesystem::weakly_canonical(dump);
        const auto relativeDump=canonicalDump.lexically_relative(store.root());
        if(!relativeDump.empty() && *relativeDump.begin()!="..")throw std::runtime_error("PCM dump must be outside the asset root");
        auto opened=store.open(request);
        if(const auto* error=std::get_if<mnm::assets::Error>(&opened))throw mnm::audio::AssetInputError(*error);
        auto input=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));
        const auto wave=mnm::audio::loadWave(*input);
        input.reset(); // Parsed PCM must survive file closure before upload.
        mnm::audio::Device device;
        mnm::audio::BufferId primary=0;
        if(device.createPrimary(mnm::reconstruction::audio::primaryDescriptor().flags,primary)!=mnm::audio::Error::ok ||
           device.setPrimaryFormat(primary,mnm::reconstruction::audio::primaryFormat(device.capabilities))!=mnm::audio::Error::ok)
            throw std::runtime_error("Primary setup failed");
        const auto uploaded=mnm::reconstruction::audio::uploadStatic(device,wave);
        if(uploaded.error!=mnm::audio::Error::ok)throw std::runtime_error("Native sample upload failed");
        mnm::audio::BufferId duplicate=0;
        if(device.duplicate(uploaded.buffer,duplicate)!=mnm::audio::Error::ok)throw std::runtime_error("Duplicate failed");
        device.release(uploaded.buffer);const auto samples=device.samples(duplicate);
        std::ofstream output(dump,std::ios::binary);output.write(reinterpret_cast<const char*>(samples.data()),std::streamsize(samples.size()));
        output.close();if(!output)throw std::runtime_error("Sample dump failed");
        const auto info=device.info(duplicate).value();device.release(duplicate);device.release(primary);
        std::cout<<"{\"channels\":"<<wave.format.channels<<",\"rate\":"<<wave.format.rate<<",\"bits\":"<<wave.format.bits
            <<",\"alignment\":"<<wave.format.alignment<<",\"bytes_per_second\":"<<wave.format.bytesPerSecond
            <<",\"samples\":"<<uploaded.copied<<",\"revision\":"<<info.revision<<",\"remaining_buffers\":"<<device.count()
            <<",\"audible_output\":false}\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}

#include "dsound_setup.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

int main(int argc,char** argv){
    if(argc!=4 || std::string(argv[2])!="--dump"){
        std::cerr<<"Usage: mnm-audio-upload PCM.wav --dump samples.bin\n";return 2;
    }
    try {
        std::ifstream input(argv[1],std::ios::binary|std::ios::ate);
        if(!input || input.tellg()<0 || input.tellg()>32*1024*1024)throw std::runtime_error("Cannot read bounded WAV");
        const auto size=static_cast<std::size_t>(input.tellg());input.seekg(0);
        std::vector<std::uint8_t> file(size);if(!input.read(reinterpret_cast<char*>(file.data()),std::streamsize(size)))throw std::runtime_error("Short WAV read");
        const auto wave=mnm::audio::readWave(file);mnm::audio::Device device;
        mnm::audio::BufferId primary=0;
        if(device.createPrimary(mnm::reconstruction::audio::primaryDescriptor().flags,primary)!=mnm::audio::Error::ok ||
           device.setPrimaryFormat(primary,mnm::reconstruction::audio::primaryFormat(device.capabilities))!=mnm::audio::Error::ok)
            throw std::runtime_error("Primary setup failed");
        const auto uploaded=mnm::reconstruction::audio::uploadStatic(device,wave);
        if(uploaded.error!=mnm::audio::Error::ok)throw std::runtime_error("Native sample upload failed");
        mnm::audio::BufferId duplicate=0;
        if(device.duplicate(uploaded.buffer,duplicate)!=mnm::audio::Error::ok)throw std::runtime_error("Duplicate failed");
        device.release(uploaded.buffer);const auto samples=device.samples(duplicate);
        std::ofstream output(argv[3],std::ios::binary);output.write(reinterpret_cast<const char*>(samples.data()),std::streamsize(samples.size()));
        output.close();if(!output)throw std::runtime_error("Sample dump failed");
        const auto info=device.info(duplicate).value();device.release(duplicate);device.release(primary);
        std::cout<<"{\"channels\":"<<wave.format.channels<<",\"rate\":"<<wave.format.rate<<",\"bits\":"<<wave.format.bits
            <<",\"alignment\":"<<wave.format.alignment<<",\"bytes_per_second\":"<<wave.format.bytesPerSecond
            <<",\"samples\":"<<uploaded.copied<<",\"revision\":"<<info.revision<<",\"remaining_buffers\":"<<device.count()
            <<",\"audible_output\":false}\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}

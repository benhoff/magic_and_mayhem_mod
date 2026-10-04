#include "catalog_preflight.hpp"
#include "audio-catalog-fixture.hpp"
#include <iostream>
namespace r=mnm::reconstruction::audio;using namespace audioFixture;
int main(){try{
    QTemporaryDir dir;check(dir.isValid(),"temp directory");const auto root=std::filesystem::path(dir.path().toStdString());
    write(root/"Tone.wav",wave(4000));write(root/"Broken.wav",{'b','a','d'});
    profile(root,"[Sounds]\n10='Tone' ; comment\n20=Stream\n30=Broken\n40=Logical\n410=Tone\n[Randomised]\n40=10,410\n50=999\n60=40\n70=\n80=10,20\n[Optimisation]\nMaxSimultaneousSounds=4\n");
    const auto strict=r::preflightCatalog(store(root),r::NativeSourcePathPolicy::literal);
    const auto compatible=r::preflightCatalog(store(root),r::NativeSourcePathPolicy::dequoteMissingLeaf);
    check(strict.valid() && compatible.valid() && compatible.sources.size()==4 && compatible.groups.size()==5,"groups distinct from file sources including group-only IDs");
    check(!strict.playable(10) && compatible.playable(10) && compatible.playable(40),"explicit filename policy and all group choices");
    check(!compatible.playable(20) && compatible.sources[1].path=="Stream.wav" && !compatible.sources[1].diagnostic.empty(),"missing Stream reported");
    check(!compatible.playable(30) && !compatible.sources[2].diagnostic.empty(),"existing malformed WAV not considered playable");
    check(compatible.playable(50) && compatible.groups[1].resolved==std::vector<int>{410},"recovered unknown group member fallback");
    check(!compatible.playable(60) && !compatible.playable(70) && !compatible.playable(80) && !compatible.playable(999),"nested, empty, partially missing groups and unknown requests rejected");
    check(compatible.sources[0].pcmBytes==256 && compatible.sources[0].durationMs==2 && compatible.simultaneousLimit==4,"PCM metadata and schedule configuration");
    check(!std::filesystem::exists(root/"'Tone'.wav") && !std::filesystem::exists(root/"Stream.wav"),"preflight read only");
    profile(root,"[Sounds]\n10=Tone\n[Optimisation]\nMaxSimultaneousSounds=1\n");
    check(!r::preflightCatalog(store(root),r::NativeSourcePathPolicy::literal).valid(),"invalid schedule count fatal before manager construction");
    profile(root,"[Sounds]\n10=Tone\n10=Tone\n");check(!r::preflightCatalog(store(root),r::NativeSourcePathPolicy::literal).valid(),"malformed profile diagnostic");
    std::filesystem::remove(root/"Sounds.ini");check(!r::preflightCatalog(store(root),r::NativeSourcePathPolicy::literal).valid(),"missing profile diagnostic");
    std::cout<<"Catalog preflight passed; read-only PCM probes, policies, group choices and missing assets\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

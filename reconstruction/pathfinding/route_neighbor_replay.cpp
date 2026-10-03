#include "route_world.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <stdexcept>
using namespace mnm::reconstruction;
int main(int argc,char** argv) {
    try {
        if(argc!=3)throw std::invalid_argument("usage: neighbor-replay WORLD.bin EXPANSION.bin");
        const auto s=read_route_world(argv[1]);
        std::ifstream input(argv[2],std::ios::binary);
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),{});
        if(bytes.size()<126 || bytes.size()>126+(4096*2+26)*36 || std::memcmp(bytes.data(),"MNMEXP01",8))
            throw std::invalid_argument("invalid expansion record");
        const auto u=[&](std::size_t offset) {std::uint32_t n;std::memcpy(&n,bytes.data()+offset,4);return n;};
        const auto before=u(20),after=u(24);
        if(before>4096 || after<before || after-before>26 || bytes.size()!=126+(before+after)*36)
            throw std::invalid_argument("expansion candidate lengths invalid");
        NeighborDescriptor descriptor;MovementPayload prior;
        std::memcpy(&descriptor,bytes.data()+28,70);std::memcpy(&prior,bytes.data()+98,28);
        if(descriptor.mode || descriptor.linked || descriptor.object!=s->object_token)
            throw std::invalid_argument("shadow scope is standard creature expansion");
        if(s->plane_stride!=s->dimensions.x*s->dimensions.y)
            throw std::invalid_argument("shadow replay requires canonical plane stride");
        for(int y=0;y<s->dimensions.y;++y)
            if(s->rows[y]!=y*s->dimensions.x)
                throw std::invalid_argument("shadow replay requires canonical row offsets");
        for(int z=0;z<s->dimensions.z;++z)
            if(s->layers[z]!=z*s->plane_stride)
                throw std::invalid_argument("shadow replay requires canonical layer offsets");
        auto helpers=snapshot_neighbors(s);
        auto remaining=s->budget;
        if(std::uint32_t(remaining)!=u(12))throw std::invalid_argument("world/expansion input budget mismatch");
        std::vector<Candidate> expected(before);
        if(before)std::memcpy(expected.data(),bytes.data()+126,before*36);
        expand_neighbors(u(8),prior,descriptor,helpers,remaining,expected);
        std::cout<<"{\"budget_remaining\":"<<remaining<<",\"candidate_count\":"<<expected.size()<<",\"candidate_hex\":\"";
        const auto* raw=reinterpret_cast<const unsigned char*>(expected.data());
        for(std::size_t i=0;i<expected.size()*36;++i)std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(raw[i]);
        std::cout<<"\"}\n";
    } catch(const std::exception& error) {std::cerr<<"Neighbor replay refused: "<<error.what()<<'\n';return 1;}
}

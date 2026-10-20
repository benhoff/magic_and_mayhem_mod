#include "../reconstruction/rendering/world_queue.hpp"
#include <iostream>
using namespace mnm::reconstruction;
static const char* name(QueueBackend b) {
    switch (b) {
    case QueueBackend::copy: return "copy";
    case QueueBackend::terrain: return "terrain";
    case QueueBackend::clippedCopy: return "clippedCopy";
    case QueueBackend::wordCopy: return "wordCopy";
    case QueueBackend::shadow: return "shadow";
    case QueueBackend::opaque: return "opaque";
    case QueueBackend::quarter: return "quarter";
    case QueueBackend::half: return "half";
    case QueueBackend::threeQuarter: return "threeQuarter";
    case QueueBackend::displacement: return "displacement";
    case QueueBackend::alternateShadow: return "alternateShadow";
    case QueueBackend::distortion: return "distortion";
    case QueueBackend::alternateBlend: return "alternateBlend";
    case QueueBackend::tint: return "tint";
    case QueueBackend::wave: return "wave";
    case QueueBackend::virtualObject: return "virtualObject";
    case QueueBackend::objectHelper: return "objectHelper";
    }
    throw std::logic_error("Unknown backend");
}

struct Backend {
    std::vector<QueueCall> calls;
    unsigned result{};
    void clip(const std::array<std::uint32_t,4>& values) {
        QueueCall call{QueueBackend::copy,{},4};
        std::copy(values.begin(), values.end(), call.values.begin());
        call.count=100; calls.push_back(call);
    }
    unsigned draw(QueueTable, const QueueCall& call) {calls.push_back(call);return result;}
    void postlude(const std::array<std::uint32_t,3>& values) {
        QueueCall call{QueueBackend::copy,{},3};
        std::copy(values.begin(), values.end(), call.values.begin());
        call.count=101; calls.push_back(call);
    }
};
int main() try {
    unsigned ordinal=0,cases=0;
    for(unsigned indexed:{0u,1u})for(int offset:{-127,3})
    for(unsigned video:{0u,1u})for(unsigned count:{0u,1u,2u})for(unsigned post:{0u,1u})
    for(int kind=-2;kind<=35;++kind)for(int y:{-20,-12,0,1,100,468,480,588,600})
    for(std::uint16_t first:{std::uint16_t(0x8ad0),std::uint16_t(-13),std::uint16_t(-12),std::uint16_t(-11),std::uint16_t(0),std::uint16_t(5)})
    for(std::uint16_t prior:{std::uint16_t(0x8ad0),std::uint16_t(0),std::uint16_t(1),std::uint16_t(0x1234),std::uint16_t(0xffff)})
    for(unsigned height_flag:{0u,1u})for(unsigned value:{0u,1u,0x12340001u,0x12345678u,0xffffffffu}) {
        if (ordinal++ % 97) continue;
        std::vector<WorldQueueRow> rows;
        for(unsigned i=0;i<2;++i) rows.push_back({{kind,42,120+i,std::uint32_t(y+int(i)*3),std::int16_t(offset),7,bool(indexed),{77,16,16,3}},16,4,first,prior});
        WorldQueueState state{bool(video),bool(height_flag),bool(post),{0,3,799,600},19,{0x10203040,0x11223344,0xaabbccdd}};
        Backend backend;backend.result=value; consumeWorldQueue(rows,count,state,backend);
        std::cout<<"{\"indexed\":"<<indexed<<",\"offset\":"<<offset<<",\"video\":"<<video<<",\"count\":"<<count<<",\"postlude\":"<<post<<",\"kind\":"<<kind
                 <<",\"y\":"<<y<<",\"first\":"<<first<<",\"prior\":"<<prior<<",\"height_flag\":"<<height_flag<<",\"result\":"<<value<<",\"rows_after\":[";
        for(unsigned i=0;i<2;++i) {
            const auto& row=rows[i]; const auto& d=row.draw;
            const std::array<std::uint32_t,9> words{0,0,d.x,d.y,std::uint16_t(d.offsetX)|(std::uint32_t(7)<<16),0,std::uint32_t(d.kind),row.first|(std::uint32_t(row.prior)<<16),0};
            for(unsigned j=0;j<words.size();++j){if(i||j)std::cout<<',';std::cout<<words[j];}
        }
        std::cout<<"],\"postlude_after\":[";
        for(unsigned i=0;i<3;++i){if(i)std::cout<<',';std::cout<<state.pending[i];}
        std::cout<<"],\"calls\":[";
        for(unsigned i=0;i<backend.calls.size();++i) {
            if(i)std::cout<<',';
            const auto& c=backend.calls[i];
            std::cout<<"{\"backend\":\""<<(c.count==100?"clip":c.count==101?"postlude":name(c.backend))<<"\",\"values\":[";
            const unsigned n=c.count==100?4:c.count==101?3:c.count;
            for(unsigned j=0;j<n;++j){if(j)std::cout<<',';std::cout<<c.values[j];}std::cout<<"]}";
        }
        std::cout<<"]}\n";++cases;
    }
    if(cases!=50772)throw std::runtime_error("Incomplete state matrix sample");
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}

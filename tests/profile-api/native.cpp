#include "profile.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>
struct Query {unsigned op;const char *section,*key;unsigned capacity;const char* fallback;unsigned number;};
#define Q(id,op,section,key,cap,fallback,number) {op,section,key,cap,fallback,number},
static const Query queries[]={
#include "queries.inc"
};
#undef Q
int main(){
    std::ifstream input("fixture.ini",std::ios::binary);
    if(!input)return 2;
    const std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input),{}};
    const auto profile=mnm::assets::ProfileSnapshot::parse(bytes);
    std::ofstream output("native-results.bin",std::ios::binary);
    std::ofstream rejected("native-rejections.txt");unsigned index=0;
    for(const auto& q:queries){
        struct Record {std::uint32_t returned=0;char bytes[128]={};} record;
        static_assert(sizeof(Record)==132);
        try {
            if(q.op==1){const auto value=profile.value(q.section,q.key,q.capacity,q.fallback);record.returned=value.size();std::copy(value.begin(),value.end(),record.bytes);}
            if(q.op==2){const auto list=profile.section(q.section,q.capacity);record.returned=list.returned;std::size_t offset=0;for(const auto& entry:list.entries){std::copy(entry.begin(),entry.end(),record.bytes+offset);offset+=entry.size()+1;}}
            if(q.op==3)record.returned=profile.integer(q.section,q.key,q.number);
        }catch(const std::invalid_argument&){record.returned=0xffffffff;rejected<<index<<"\n";}
        ++index;output.write(reinterpret_cast<const char*>(&record),sizeof(record));
    }
    return output && rejected?0:3;
}

#include "installed_wire.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "profile.hpp"
#include "manager_configuration.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
namespace a=mnm::assets;namespace r=mnm::reconstruction::audio;
static_assert(sizeof(ProfileQuery)==268 && sizeof(ProfileRecord)==16388);
template<class T>T read(std::ifstream& input){T value{};input.read(reinterpret_cast<char*>(&value),sizeof(value));if(!input)throw std::runtime_error("Short fixture record");return value;}
std::vector<ProfileQuery> queries(){
    std::ifstream input("queries.bin",std::ios::binary);const auto magic=read<unsigned>(input),version=read<unsigned>(input),count=read<unsigned>(input);
    if(magic!=PROFILE_MAGIC || version!=PROFILE_VERSION || count>PROFILE_QUERY_LIMIT)throw std::runtime_error("Invalid fixture header");
    std::vector<ProfileQuery> out;
    for(unsigned i=0;i<count;++i){auto q=read<ProfileQuery>(input);if(q.op<1 || q.op>3 || !q.capacity || q.capacity>PROFILE_CAPACITY || !std::memchr(q.section,0,128) || !std::memchr(q.key,0,128))throw std::runtime_error("Invalid query");out.push_back(q);}
    if(input.peek()!=EOF)throw std::runtime_error("Trailing query data");
    return out;
}
QJsonArray ids(const std::vector<std::int32_t>& values){QJsonArray out;for(auto value:values)out.append(value);return out;}
r::ProfileSection section(const ProfileRecord& record){
    if(record.returned>=PROFILE_CAPACITY-2)throw std::runtime_error("Truncated installed section");
    r::ProfileSection out;out.returned=record.returned;
    std::size_t offset=0;while(offset<record.returned){const auto* end=static_cast<const char*>(std::memchr(record.bytes+offset,0,record.returned-offset));if(!end)throw std::runtime_error("Missing section terminator");out.entries.emplace_back(record.bytes+offset,end);offset+=out.entries.back().size()+1;}
    if(record.bytes[offset])throw std::runtime_error("Missing final section terminator");
    return out;
}
int main(int argc,char** argv){try{
    const auto list=queries();
    if(argc==1){
        auto made=a::AssetStore::create(std::filesystem::current_path());auto& store=std::get<a::AssetStore>(made);auto opened=store.open("fixture.ini");auto loaded=a::loadProfile(*std::get<std::unique_ptr<a::AssetFile>>(opened));
        if(auto* error=std::get_if<a::Error>(&loaded))throw std::runtime_error(error->detail);
        const auto& profile=std::get<a::ProfileSnapshot>(loaded);std::ofstream output("native-results.bin",std::ios::binary);
        for(const auto& q:list){ProfileRecord result{};
            if(q.op==1){const auto value=profile.value(q.section,q.key,q.capacity);result.returned=value.size();std::copy(value.begin(),value.end(),result.bytes);}
            if(q.op==2){const auto entries=profile.section(q.section,q.capacity);result.returned=entries.returned;std::size_t at=0;for(const auto& e:entries.entries){std::copy(e.begin(),e.end(),result.bytes+at);at+=e.size()+1;}}
            if(q.op==3)result.returned=profile.integer(q.section,q.key,q.fallback);
            output.write(reinterpret_cast<const char*>(&result),sizeof(result));
        }
        if(!output)throw std::runtime_error("Cannot write native results");
    }else if(argc==2){
        std::ifstream input(argv[1],std::ios::binary);QJsonObject catalog;QJsonArray groups,names;QJsonObject sections;
        for(const auto& q:list){const auto result=read<ProfileRecord>(input);const auto name=QString::fromLatin1(q.section);
            if(q.op==2){const auto decoded=section(result);if(name=="Sounds" || name=="Randomised")catalog.insert(name,ids(r::soundTable(decoded)));else{QJsonArray entries;for(const auto& entry:decoded.entries)entries.append(QString::fromStdString(entry));sections.insert(name,entries);}}
            if(q.op==1){if(result.returned>=q.capacity || !std::memchr(result.bytes,0,q.capacity))throw std::runtime_error("Truncated installed string");const std::string value(result.bytes,result.returned);
                if(name=="Randomised")groups.append(QJsonObject{{"id",std::stoi(q.key)},{"members",ids(r::groupMembers(value))}});
                if(name=="Sounds")names.append(QJsonObject{{"id",std::stoi(q.key)},{"name",QString::fromStdString(r::sourceEntryName(value))}});
            }
            if(q.op==3)catalog.insert("simultaneousLimit",int(result.returned));
        }
        if(input.peek()!=EOF)throw std::runtime_error("Trailing result data");
        catalog.insert("groups",groups);catalog.insert("sourceNames",names);catalog.insert("sections",sections);std::cout<<QJsonDocument(catalog).toJson().constData();
    }else throw std::runtime_error("Usage: installed-native [results.bin]");
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

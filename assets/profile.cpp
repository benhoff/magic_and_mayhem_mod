#include "profile.hpp"
#include <charconv>
#include <stdexcept>
#include <unordered_set>
namespace mnm::assets {
namespace {
std::string trim(std::string_view s){const auto first=s.find_first_not_of(" \t\r");if(first==s.npos)return {};return std::string(s.substr(first,s.find_last_not_of(" \t\r")-first+1));}
std::string folded(std::string_view s){std::string out(s);for(auto& c:out)if(c>='A' && c<='Z')c=char(c-'A'+'a');return out;}
std::string unquote(std::string value){if(value.size()>=2 && (value.front()=='\'' || value.front()=='"') && value.back()==value.front())return value.substr(1,value.size()-2);return value;}
}
ProfileSnapshot ProfileSnapshot::parse(const std::vector<std::uint8_t>& bytes){
    if(bytes.size()>std::size_t(ProfileInputLimit))throw std::invalid_argument("Profile exceeds native byte limit");
    for(auto c:bytes)if(c>=127 || (c<32 && c!='\n' && c!='\r' && c!='\t'))throw std::invalid_argument("Profile requires printable ASCII/tab/CR/LF");
    const std::string text(bytes.begin(),bytes.end());ProfileSnapshot result;std::string current;std::map<std::string,std::unordered_set<std::string>> seen;
    for(std::size_t start=0;start<text.size();){
        const auto end=text.find('\n',start);const auto line=trim(std::string_view(text).substr(start,end==text.npos?text.size()-start:end-start));
        start=end==text.npos?text.size():end+1;
        if(line.empty() || line.front()==';')continue;
        if(line.front()=='['){
            const auto close=line.find(']',1);
            if(close==line.npos)throw std::invalid_argument("Malformed profile section");
            const auto suffix=trim(std::string_view(line).substr(close+1));
            if(!suffix.empty() && suffix.front()!=';')throw std::invalid_argument("Malformed profile section suffix");
            current=folded(trim(std::string_view(line).substr(1,close-1)));
            if(current.empty() || !result.sections_.emplace(current,std::vector<Entry>{}).second)throw std::invalid_argument("Empty/duplicate profile section");
            continue;
        }
        if(current.empty())throw std::invalid_argument("Profile entry precedes a section");
        const auto equal=line.find('=');const auto key=trim(std::string_view(line).substr(0,equal));
        if(key.empty())throw std::invalid_argument("Empty profile key");
        auto& entries=result.sections_[current];
        if(!seen[current].insert(folded(key)).second)throw std::invalid_argument("Duplicate profile key");
        std::optional<std::string> value;if(equal!=line.npos)value=trim(std::string_view(line).substr(equal+1));
        entries.push_back({key,value?key+"="+*value:key,std::move(value)});
    }
    return result;
}
ProfileListing ProfileSnapshot::section(std::string_view name,std::uint32_t capacity) const{
    if(capacity<2)throw std::invalid_argument("Section capacity must include two terminators");
    ProfileListing out;const auto found=sections_.find(folded(name));if(found==sections_.end())return out;
    std::uint64_t count=0;for(const auto& e:found->second)count+=e.raw.size()+1;
    // Wine marks an exact-fit list truncated too; consumers reject this marker.
    // Partial enumeration is not exposed as complete.
    if(count && count+1>=capacity)return {capacity-2,{}};
    for(const auto& e:found->second)out.entries.push_back(e.raw);
    out.returned=std::uint32_t(count);return out;
}
std::string ProfileSnapshot::value(std::string_view name,std::string_view key,std::uint32_t capacity,std::string_view fallback) const{
    if(!capacity)throw std::invalid_argument("String capacity must include a terminator");
    auto value=std::string(fallback);while(!value.empty() && value.back()==' ')value.pop_back();const auto found=sections_.find(folded(name));
    if(found!=sections_.end())for(const auto& e:found->second)if(folded(e.key)==folded(key) && e.value){value=unquote(*e.value);break;}
    if(value.size()>=capacity)value.resize(capacity-1);
    return value;
}
std::uint32_t ProfileSnapshot::integer(std::string_view name,std::string_view key,std::uint32_t fallback) const{
    const auto found=sections_.find(folded(name));if(found==sections_.end())return fallback;
    for(const auto& e:found->second)if(folded(e.key)==folded(key) && e.value){
        const auto text=unquote(*e.value);std::uint32_t value=0;
        const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);
        if(text.empty() || parsed.ec!=std::errc{} || parsed.ptr!=text.data()+text.size())throw std::invalid_argument("Profile integer requires complete unsigned decimal");
        return value;
    }
    return fallback;
}
Result<ProfileSnapshot> loadProfile(AssetFile& file){
    auto seek=file.seek(0);if(auto* e=std::get_if<Error>(&seek))return *e;
    auto bytes=readWhole(file,ProfileInputLimit);if(auto* e=std::get_if<Error>(&bytes))return *e;
    try{return ProfileSnapshot::parse(std::get<std::vector<std::uint8_t>>(bytes));}
    catch(const std::invalid_argument& e){return Error{ErrorCode::invalidArgument,"parse profile",{},{},e.what()};}
}
}

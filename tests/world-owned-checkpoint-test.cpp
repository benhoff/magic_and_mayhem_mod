#include "../compat/legacy/world_rolling_handoff.hpp"
#include <fstream>
#include <iostream>
#include <limits>
using namespace mnm::legacy;
namespace {
void require(bool okay,const char* reason){if(!okay)throw std::runtime_error(reason);}
std::vector<std::uint8_t> read(const char* path){
 std::ifstream file(path,std::ios::binary);require(bool(file),"Input open");return {std::istreambuf_iterator<char>(file),{}};
}
template<class F> void refuses(F f){bool caught=false;try{f();}catch(const std::invalid_argument&){caught=true;}require(caught,"Malformed input accepted");}
}
int main(int argc,char** argv)try{
 require(argc==3||argc==4,"Require retained source and request");
 auto stream=decodeCanvasProducers(read(argv[1]),false);const auto request=read(argv[2]);
 std::size_t checkpoint=0;unsigned maximum=0;
 for(std::size_t i=0;i<stream.operations.size();++i)if(stream.operations[i].fields[2]==10&&stream.operations[i].fields[14]>maximum){checkpoint=i;maximum=stream.operations[i].fields[14];}
 require(maximum>=9999&&maximum<=stream.operations.size(),"Require a real bounded long-prefix checkpoint case");
 auto prepare=[&](const auto& input,const auto& req){return prepareRollingHandoff(input,0,req,1,0,WorldBatchSourcePolicy::ownedV5,false);};
 if(argc==4){refuses([&]{prepare(stream,request);});std::cout<<"{\"legacy_limit_refusal_reproduced\":true}\n";return 0;}
 auto handoff=prepare(stream,request);
 require(handoff.checkpointOracle==maximum+1&&handoff.stream.operations.size()==stream.operations.size()+2,"Owned closing identities differ");
 require(handoff.batch.descriptors.size()>0,"No validated raster descriptors");
 // The finite file-oracle policy keeps its four-digit name boundary.
 refuses([&]{prepareRollingHandoff(stream,0,request,1,0,WorldBatchSourcePolicy::canonicalV3,false);});
 auto bad=stream;bad.operations[checkpoint].fields[14]=unsigned(stream.operations.size()+1);
 refuses([&]{prepare(bad,request);});bad.operations[checkpoint].fields[14]=std::numeric_limits<unsigned>::max();refuses([&]{prepare(bad,request);});
 bad=stream;for(auto& c:bad.operations)if(c.fields[2]==11)c.fields[2]=4;refuses([&]{prepare(bad,request);});
 bad=stream;bad.operations.resize(131071,bad.operations.back());refuses([&]{prepare(bad,request);});
 auto changed=request;changed.back()^=1;refuses([&]{prepare(stream,changed);});
 std::cout<<"{\"success\":true,\"source_records\":"<<stream.operations.size()<<",\"source_checkpoint\":"<<maximum<<",\"closing_checkpoint\":"<<handoff.checkpointOracle<<",\"finite_boundary_kept\":true,\"identity_overflow_refused\":true,\"record_budget_kept\":true,\"request_digest_checked\":true,\"original_return_executed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

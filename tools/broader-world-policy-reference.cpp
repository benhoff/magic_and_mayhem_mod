// A separate policy oracle; never replaces the raw original-reference executable.
#include "broader_finite_edge_oracle.hpp"
#define MNM_ACCEPTANCE_DRAW_WATCH(ordinal,entry,record,pixels) broader_edge::Guard finiteEdgeGuard(ordinal,entry,record,pixels)
#ifndef MNM_BROADER_POLICY_EXECUTOR
#error Build through build-broader-policy-reference.py
#endif
#include MNM_BROADER_POLICY_EXECUTOR
int main(int argc,char** argv){
 const int result=broader_original_policy_main(argc,argv);
 if(result||argc!=5)return result;
 std::ofstream log(std::string(argv[4])+".finite-edge.json",std::ios::binary);
 if(!log)return 2;
 log<<"{\"schema\":1,\"policy\":\"native-retain-outside-displacement-v1\",\"original_padding_equivalence_claimed\":false,\"taps\":[";
 bool first=true;for(const auto& tap:broader_edge::taps){if(!first)log<<',';first=false;log<<"{\"draw\":"<<tap.ordinal<<",\"destination\":"<<tap.destination<<",\"outside_sample\":"<<tap.sample<<",\"retained_word\":"<<tap.retained<<'}';}
 log<<"]}\n";log.flush();return log?0:2;
}

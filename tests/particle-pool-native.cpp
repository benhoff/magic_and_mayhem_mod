#include "../reconstruction/rendering/particle_pool.hpp"
#include <iostream>
#include <stdexcept>
using u32=std::uint32_t;using i32=std::int32_t;
#include "particle-pool-fixture.h"
static mnm::reconstruction::ParticlePool pool;
static void fixture_require(int ok,u32 code){if(!ok)throw std::runtime_error("Pool fixture "+std::to_string(code));}
static void fixture_write(const void* data,u32 bytes){std::cout.write(static_cast<const char*>(data),bytes);}
static void pool_allocate(u32 token){nodes[token].address=token;}
static void pool_validate_object(u32 token){fixture_require(nodes[token].alive,209);}
static void pool_increment_age(u32 token){fixture_require(nodes[token].age>0,210);}
struct PoolBackend {
    i32 priority(u32 token){return fixture_priority(token);}
    bool update(u32 token){return fixture_update(token)!=0;}
    void destroy(u32 token){fixture_destroyed(token);}
};
static PoolBackend backend;
static u32 pool_insert(u32 token){return pool.insert(token,backend);}
static void pool_update(){pool.update(backend);}
static void pool_clear(){pool.clear(backend);}
static u32 pool_entry(u32 index){return pool.entries.at(index);}
static i32 pool_count(){return pool.count;}
static i32 pool_hint(){return pool.hint;}
static void pool_initialize(u32 count,i32 hint){
    pool={};pool.count=i32(count);pool.hint=hint;
    for(u32 i=0;i<count;++i)pool.entries[i]=i+1;
}
static void pool_free_unowned(u32 token){fixture_destroyed(token);}
int main()try{fixture_run();return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

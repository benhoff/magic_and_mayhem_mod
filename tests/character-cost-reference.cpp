#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../reconstruction/campaign/character_cost.hpp"
#include <array>
#include "character_cost_fixture.h"
#include <cstdlib>
#include <limits>

using namespace mnm::recovered::campaign;
namespace {
constexpr unsigned menu=0x6c5148, bars=0x6e8000, gems=0x6e8100;
static void check(bool ok,const char* text) { if(!ok){std::cerr<<text<<'\n';std::exit(1);} }
static unsigned word(unsigned at) { unsigned value;std::memcpy(&value,reinterpret_cast<void*>(at),4);return value; }
static void floating(unsigned at,float value) { std::memcpy(reinterpret_cast<void*>(at),&value,4); }
static unsigned current(unsigned row) { return (row<3?bars:gems)+(row%3)*0x2c+(row<3?0x28:0x1c); }
static void run(const std::array<int,6>& values,const std::array<CharacterCostCoefficients,4>& coefficients) {
    std::memset(reinterpret_cast<void*>(menu),0,0x120);std::memset(reinterpret_cast<void*>(bars),0,0x200);
    global(menu+0x7f,bars);global(menu+0x83,gems);global(0x6fa208,1);
    for(unsigned row=0;row<6;++row)global(current(row),unsigned(values[row]));
    for(unsigned row=0;row<4;++row) {
        const auto& c=coefficients[row];const unsigned p=menu+0xab+row*0x10;
        global(p,unsigned(c.increment));floating(p+4,c.initial);floating(p+8,c.linear);floating(p+12,c.exponential);
    }
    unsigned short cw=0x27f;
    asm volatile("fninit; fldcw %0" : : "m"(cw));
    using Calculator=void (__attribute__((thiscall)) *)(void*);
    reinterpret_cast<Calculator>(0x4b5d10)(reinterpret_cast<void*>(menu));
    unsigned short after=0;asm volatile("fnstcw %0" : "=m"(after));check(after==cw,"original FPU control word changed");
    for(unsigned row=0;row<6;++row) {
        const auto expected=characterCosts(values[row],coefficients[row<3?row:3]);
        check(bool(expected),"valid model input refused");
        if(word(menu+0xeb+row*8)!=unsigned(expected->decrease)||word(menu+0xef+row*8)!=unsigned(expected->increase)) {
            std::cerr<<"row "<<row<<" value "<<values[row]<<" original "<<word(menu+0xeb+row*8)<<'/'<<word(menu+0xef+row*8)
                     <<" model "<<expected->decrease<<'/'<<expected->increase<<'\n';std::exit(1);
        }
    }
}
}
int main(int argc,char** argv) try {
    check(argc==2,"Pinned executable required");map_image(read(argv[1]));
    const auto* begin=reinterpret_cast<const unsigned char*>(0x4b5d10);
    const Bytes bytes(begin,reinterpret_cast<const unsigned char*>(0x4b5f88));
    const auto* mathBegin=reinterpret_cast<const unsigned char*>(0x59bcc0);
    const Bytes mathBytes(mathBegin,reinterpret_cast<const unsigned char*>(0x59bf07));
    unsigned cases=0;
    // Every value, including nonmultiples of configured increments and extrema.
    for(int value=0;value<=800;++value) {
        run({value%201,value,value%41,value%8,(value+3)%8,(value+6)%8},installed);++cases;
    }
    const std::array<CharacterCostCoefficients,4> synthetic{{{5,5,2,1.1f},{20,10,4,1.07f},{1,20,1,1.15f},{1,100,2,1.5f}}};
    for(int value=0;value<=800;++value) {
        run({value%201,value,value%41,value%8,(value+3)%8,(value+6)%8},synthetic);++cases;
    }
    const std::array<CharacterCostCoefficients,4> exact{{{5,0,0,1},{20,0.75f,1,1},{1,0.25f,0,2},{1,0,1,2}}};
    for(int value=0;value<=40;++value) {
        run({value,value,value,value%8,(value+1)%8,(value+2)%8},exact);++cases;
    }
    unsigned refusals=0;const CharacterCostCoefficients valid{5,5,2,1.1f};
    for(int value:{-1,1000001}) {check(!characterCosts(value,valid),"invalid value accepted");++refusals;}
    for(const auto& c:std::array<CharacterCostCoefficients,10>{{
        {0,5,2,1.1f},{-1,5,2,1.1f},{1000001,5,2,1.1f},{5,-1,2,1.1f},{5,5,-1,1.1f},{5,5,2,0.5f},
        {5,std::numeric_limits<float>::infinity(),2,1.1f},{5,5,std::numeric_limits<float>::quiet_NaN(),1.1f},
        {5,5,2,std::numeric_limits<float>::infinity()},{1,0,1000,1000}}}) {
        check(!characterCosts(40,c),"invalid coefficient/output accepted");++refusals;
    }
    check(!std::memcmp(begin,bytes.data(),bytes.size()),"original calculator mutated");
    check(!std::memcmp(mathBegin,mathBytes.data(),mathBytes.size()),"original math helpers mutated");
    std::cout<<"{\"success\":true,\"calculator_cases\":"<<cases<<",\"price_comparisons\":"<<cases*12
             <<",\"refusals\":"<<refusals<<",\"calculator_and_math_unchanged\":true}\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}

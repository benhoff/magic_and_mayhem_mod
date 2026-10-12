// Original Character action callback in a private complete PE32 mapping.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "character_menu_fixture_bytes.h"
#include <array>
#include <cstdlib>

namespace {
constexpr unsigned menu=0x6c5148, bars=0x6e8000, gems=0x6e8100;
static unsigned wizardIndex, fades, refreshes;
static void check(bool ok,const char* text) {
    if (!ok) { std::cerr << text << '\n'; std::exit(1); }
}
static unsigned word(unsigned at) { unsigned value; std::memcpy(&value,reinterpret_cast<void*>(at),4); return value; }
static int __attribute__((thiscall)) selected(void*) { return int(wizardIndex); }
static void __attribute__((thiscall)) fade(void* self) { check(self==reinterpret_cast<void*>(menu),"fade receiver"); ++fades; }
static void __attribute__((thiscall)) refresh(void* self) { check(self==reinterpret_cast<void*>(menu),"refresh receiver"); ++refreshes; }
static unsigned cell(unsigned stat) { return (stat<3?bars:gems)+(stat%3)*0x2c; }
static unsigned baseline(unsigned stat) { return cell(stat)+(stat<3?0x24:0x18); }
static unsigned current(unsigned stat) { return cell(stat)+(stat<3?0x28:0x1c); }
static unsigned maximum(unsigned stat) { return cell(stat)+(stat<3?0x20:0x14); }
static const std::array<unsigned,6> steps{5,20,1,1,1,1};
static const std::array<unsigned,6> limits{200,800,40,7,7,7};
static const std::array<unsigned,6> baseValues{40,160,4,2,2,2};
static void reset(unsigned wizard) {
    wizardIndex=wizard;fades=refreshes=0;
    std::memset(reinterpret_cast<void*>(menu),0,0x120);
    std::memset(reinterpret_cast<void*>(bars),0,0x200);
    global(menu+0x7f,bars);global(menu+0x83,gems);global(menu+0xa7,50);
    const unsigned player=0x65b250+wizard*0x93a;
    for(unsigned i=0;i<6;++i) {
        global(baseline(i),baseValues[i]);global(current(i),baseValues[i]);global(maximum(i),limits[i]);
        global(menu+(i<3?0xab+i*0x10:0xdb),steps[i]);
        global(menu+0xeb+i*8,7);global(menu+0xef+i*8,11);
        global(player+(i<3?0x3c+i*4:0xf0+(i-3)*4),baseValues[i]);
    }
    global(player+0x58,50);
}
static void action(unsigned index) {
    using Action=int (__attribute__((thiscall)) *)(void*,unsigned);
    check(reinterpret_cast<Action>(0x4b5470)(reinterpret_cast<void*>(menu),index)==0,"callback result");
    check(refreshes==1,"one original refresh request");
}
}
int main(int argc,char** argv) try {
    check(argc==2,"Pinned PE required");map_image(read(argv[1]));
    const auto* first=reinterpret_cast<const unsigned char*>(0x4b5470);
    const Bytes original(first,reinterpret_cast<const unsigned char*>(0x4b59d5));
    auto stub=[](unsigned at,const unsigned char* expected,unsigned n,void* target) {
        check(n>=5&&!std::memcmp(reinterpret_cast<void*>(at),expected,n),"dependency original bytes");
        *reinterpret_cast<unsigned char*>(at)=0xe9;global(at+1,unsigned(reinterpret_cast<std::uintptr_t>(target)-at-5));
    };
#define STUB(name,target) stub(name##_address,name##_bytes,sizeof(name##_bytes),reinterpret_cast<void*>(&target))
    STUB(selected,selected);STUB(fade,fade);STUB(refresh,refresh);
    unsigned editCases=0,acceptCases=0,cancelCases=0,invalidCases=0;
    for(unsigned wizard:{0u,2u}) for(unsigned stat=0;stat<6;++stat)
    for(unsigned value:{baseValues[stat],baseValues[stat]+steps[stat],limits[stat]})
    for(unsigned experience:{0u,10u,11u,50u}) for(bool increase:{false,true}) {
        reset(wizard);global(current(stat),value);global(menu+0xa7,experience);
        const int next=int(value)+(increase?int(steps[stat]):-int(steps[stat]));
        const bool admitted=next>=int(baseValues[stat])&&next<=int(limits[stat])&&(!increase||experience>=11);
        action(25+stat*2+unsigned(increase));
        check(word(current(stat))==(admitted?unsigned(next):value),"stat bounds/step");
        check(word(menu+0xa7)==(admitted?(increase?experience-11:experience+7):experience),"draft charge/refund");
        check(!fades&&!word(menu+0x43),"edits must not close");
        const unsigned player=0x65b250+wizard*0x93a;
        check(word(player+0x58)==50,"edits must not persist wizard experience");
        for(unsigned i=0;i<6;++i)
            check(word(player+(i<3?0x3c+i*4:0xf0+(i-3)*4))==baseValues[i],"edits must not persist wizard stats");
        ++editCases;
    }
    for(unsigned wizard:{0u,2u}) for(int experience:{-1,0,39,50,51}) {
        reset(wizard);global(menu+0xa7,unsigned(experience));
        for(unsigned i=0;i<6;++i)global(current(i),baseValues[i]+steps[i]);
        action(0);const bool accepted=experience>=0;
        const unsigned player=0x65b250+wizard*0x93a;
        check(word(player+0x58)==(accepted?unsigned(experience):50),"OK experience commit");
        check(fades==unsigned(accepted)&&word(menu+0x43)==unsigned(accepted),"OK admission/return");
        for(unsigned i=0;i<6;++i) {
            check(word(player+(i<3?0x3c+i*4:0xf0+(i-3)*4))==(accepted?baseValues[i]+steps[i]:baseValues[i]),"OK stat commit");
            check(word(baseline(i))==(accepted?baseValues[i]+steps[i]:baseValues[i]),"OK baseline advances");
        }
        ++acceptCases;
    }
    for(unsigned wizard:{0u,2u}) for(unsigned experience:{0u,39u,50u}) {
        reset(wizard);global(menu+0xa7,experience);
        for(unsigned i=0;i<6;++i)global(current(i),baseValues[i]+steps[i]);
        action(1);const unsigned player=0x65b250+wizard*0x93a;
        check(word(menu+0xa7)==50&&word(player+0x58)==50&&fades==1&&word(menu+0x43)==1,"Cancel restores experience/return");
        for(unsigned i=0;i<6;++i) {
            check(word(current(i))==baseValues[i],"Cancel restores draft baseline");
            check(word(player+(i<3?0x3c+i*4:0xf0+(i-3)*4))==baseValues[i],"Cancel wizard unchanged");
        }
        ++cancelCases;
    }
    for(unsigned index=2;index<=24;++index) {
        reset(0);action(index);check(word(menu+0xa7)==50&&!fades&&!word(menu+0x43),"unmapped action ignored");++invalidCases;
    }
    for(unsigned index:{37u,0xffffffffu}) {
        reset(0);action(index);check(word(menu+0xa7)==50&&!fades&&!word(menu+0x43),"out-of-range action ignored");++invalidCases;
    }
    check(!std::memcmp(first,original.data(),original.size()),"original Character callback mutated");
    std::cout<<"{\"success\":true,\"edit_cases\":"<<editCases<<",\"accept_cases\":"<<acceptCases
             <<",\"cancel_cases\":"<<cancelCases<<",\"invalid_cases\":"<<invalidCases
             <<",\"original_callback_unchanged\":true}\n";
    return 0;
} catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }

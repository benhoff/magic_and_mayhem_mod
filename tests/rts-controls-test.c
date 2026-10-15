#include "../game/input/rts_controls.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(v) do {assert(v);++checks;} while(0)
static MnmRtsSnapshot s;
static MnmRtsState state;
static MnmRtsResult out;
static void fixture(void) {
    memset(&s,0,sizeof(s));mnm_rts_reset(&state);
    s.count=8;s.owner=0;s.session=1;
    for(unsigned i=0;i<s.count;++i) {
        s.actors[i]=(MnmRtsActor){.slot=i,.generation=1,.type=i%3,
            .owner=0,.group=UINT32_MAX,.health=100,.active=1,.idle=1};
    }
    s.actors[0].wizard=1;s.actors[6].owner=1;s.actors[7].health=0;
}
static void apply(unsigned action,unsigned arg,unsigned now) {
    CHECK(mnm_rts_apply(&state,&s,action,arg,0,now,400,0,&out));
    for(unsigned i=0;i<s.count;++i) {
        if(out.selection_write)s.actors[i].selected=out.selection[i];
        if(out.group_write[i])s.actors[i].group=out.groups[i];
    }
}
int main(void) {
    fixture();s.actors[1].selected=1;s.actors[2].group=3;s.actors[6].group=3;
    apply(MNM_RTS_GROUP_ADD,3,0);
    CHECK(s.actors[1].group==3&&s.actors[2].group==3&&s.actors[6].group==3);
    apply(MNM_RTS_GROUP_REMOVE,3,1);
    CHECK(s.actors[1].group==UINT32_MAX&&s.actors[2].group==3);
    apply(MNM_RTS_GROUP_SAVE,3,2);
    CHECK(s.actors[1].group==3&&s.actors[2].group==UINT32_MAX&&s.actors[6].group==3);
    apply(MNM_RTS_GROUP_SAVE,4,3);CHECK(s.actors[1].group==4);
    apply(MNM_RTS_GROUP_RECALL,8,4);CHECK(!out.selection_write&&s.actors[1].selected);
    apply(MNM_RTS_GROUP_RECALL,4,5);CHECK(out.focus_slot==UINT32_MAX);
    apply(MNM_RTS_GROUP_RECALL,4,300);CHECK(out.focus_slot==1);
    CHECK(mnm_rts_apply(&state,&s,MNM_RTS_GROUP_RECALL,4,0,301,400,1,&out)&&!out.accepted);
    fixture();apply(MNM_RTS_WIZARD,0,UINT32_MAX-100);CHECK(s.actors[0].selected);
    apply(MNM_RTS_WIZARD,0,50);CHECK(out.focus_slot==0); /* clock wrap */
    fixture();s.actors[1].selected=s.actors[2].selected=s.actors[4].selected=1;
    apply(MNM_RTS_CYCLE_TYPE,0,0);CHECK(s.actors[1].selected&&s.actors[4].selected&&!s.actors[2].selected);
    apply(MNM_RTS_CYCLE_TYPE,0,1);CHECK(s.actors[2].selected&&!s.actors[1].selected);
    apply(MNM_RTS_CYCLE_TYPE,0,2);CHECK(s.actors[1].selected&&s.actors[4].selected);
    s.actors[2].generation=2;
    apply(MNM_RTS_CYCLE_TYPE,0,3);CHECK(!s.actors[2].selected); /* replacement is not retained */
    fixture();s.actors[1].selected=1;apply(MNM_RTS_SAME_TYPE,0,0);
    CHECK(s.actors[1].selected&&s.actors[4].selected&&!s.actors[7].selected);
    fixture();s.actors[2].idle=0;apply(MNM_RTS_IDLE,0,0);CHECK(s.actors[1].selected);
    apply(MNM_RTS_IDLE,0,1);CHECK(s.actors[3].selected);
    apply(MNM_RTS_IDLE,1,2);CHECK(s.actors[1].selected&&s.actors[3].selected&&!s.actors[0].selected&&!s.actors[2].selected);
    fixture();s.actors[1].group=1;apply(MNM_RTS_UNGROUPED,0,0);CHECK(s.actors[2].selected);
    apply(MNM_RTS_UNGROUPED,1,1);CHECK(s.actors[2].selected&&s.actors[5].selected&&!s.actors[0].selected&&!s.actors[1].selected);
    fixture();apply(MNM_RTS_FOCUS,0,0);CHECK(out.focus_slot==UINT32_MAX);
    s.actors[1].selected=1;apply(MNM_RTS_FOCUS,0,1);CHECK(out.focus_slot==1);
    MnmRtsState before=state;s.count=257;
    CHECK(!mnm_rts_apply(&state,&s,MNM_RTS_WIZARD,0,0,2,400,0,&out));
    CHECK(!memcmp(&state,&before,sizeof(state))&&!out.selection_write);
    fixture();s.actors[2].slot=1;
    CHECK(!mnm_rts_apply(&state,&s,MNM_RTS_WIZARD,0,0,0,400,0,&out));
    fixture();s.actors[1].group=10;
    CHECK(!mnm_rts_apply(&state,&s,MNM_RTS_WIZARD,0,0,0,400,0,&out));
    fixture();s.actors[1].selected=1;apply(MNM_RTS_GROUP_RECALL,0,0);
    s.session=2;apply(MNM_RTS_WIZARD,0,1);CHECK(out.focus_slot==UINT32_MAX&&state.session==2);
    /* All slots, every action and team: no dead/foreign selection or group writes. */
    for(unsigned owner=0;owner<4;++owner)for(unsigned action=1;action<=10;++action)
        for(unsigned group=0;group<10;++group) {
            fixture();s.count=256;s.owner=owner;
            for(unsigned i=0;i<256;++i)
                s.actors[i]=(MnmRtsActor){.slot=i,.generation=i+1,.type=i%28,
                    .owner=i%4,.group=i%11==10?UINT32_MAX:i%11,
                    .health=i%7?100:0,.active=i%5!=0,.selected=i%2,.idle=i%3==0,.wizard=i==owner};
            CHECK(mnm_rts_apply(&state,&s,action,group,0,0,400,0,&out));
            for(unsigned i=0;i<256;++i) {
                int valid=s.actors[i].active&&s.actors[i].health>0&&s.actors[i].owner==owner;
                CHECK(valid||(!out.selection[i]&&!out.group_write[i]));
            }
        }
    printf("{\"success\":true,\"checks\":%u}\n",checks);
}

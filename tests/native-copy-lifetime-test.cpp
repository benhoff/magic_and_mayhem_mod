/* Exercise the production copy-owner header with provider admission. The older
 * contention fixture also runs, covering timeout/reentry/recovery behavior. */
#include "copy-timing-test.cpp"
static u32 nativeDepth,nativeBegins,nativeEnds,nativeDrains,expectedOwner;
static u32 nativeBegin(){
    require(nativeDepth==0 || game_copy_owner==threadId,"provider precedes copy admission or is recursive");
    ++nativeDepth;++nativeBegins;return 1;
}
static void nativeEnd(){
    require(nativeDepth&&game_copy_owner==expectedOwner,"copy owner releases before provider");
    --nativeDepth;++nativeEnds;
}
static void nativeDrain(){require(nativeDepth&&game_copy_owner==threadId,"physical retirement drains under provider then copy owner");++nativeDrains;}
static void ownedReset(){reset(false);nativeDepth=nativeBegins=nativeEnds=nativeDrains=expectedOwner=0;game_copy_order_enabled=1;game_copy_native_begin=nativeBegin;game_copy_native_end=nativeEnd;game_copy_native_drain=nativeDrain;}
static void ownedTests(){
    (void)game_copy_native_backing_live;
    for(u32 method=0;method<4;++method){
        ownedReset();auto outer=method==0?game_copy_enter_at(nullptr,17):method==1?game_copy_bind_enter():method==2?game_copy_observation_enter_at(nullptr,17):game_copy_shutdown_enter();
        require(outer.held&&nativeDepth==1&&nativeDrains==1,"every admission kind takes provider and retirement drain");
        auto inner=game_copy_enter_at(nullptr,18);require(!inner.held&&inner.observe&&nativeDepth==2,"same-thread original reentry keeps provider");
        expectedOwner=threadId;game_copy_leave(&inner);require(nativeDepth==1&&game_copy_owner==threadId,"nested leave retains outer owner");
        auto refused=game_copy_shutdown_enter();require(!refused.held&&!refused.observe&&nativeDepth==2,"reentrant shutdown refuses without losing ownership");
        game_copy_leave(&refused);expectedOwner=0;game_copy_leave(&outer);
        require(!nativeDepth&&nativeBegins==nativeEnds&&nativeDrains==1,"all provider borrows retired exactly once");
        game_copy_leave(&outer);require(nativeBegins==nativeEnds,"provider cleanup is idempotent");
    }
    ownedReset();game_copy_order_enabled=0;auto disabled=game_copy_enter_at(nullptr,17);
    require(!disabled.held&&disabled.observe&&nativeDepth==1,"disabled observation still protects original native lifetime");game_copy_leave(&disabled);require(!nativeDepth&&nativeBegins==nativeEnds,"disabled admission releases provider");
    game_copy_native_begin=nullptr;game_copy_native_end=nullptr;game_copy_native_drain=nullptr;game_copy_order_enabled=1;
    std::puts("{\"success\":true,\"native_admission_kinds\":4,\"nested_borrows\":8,\"reentrant_shutdown_refusals\":4,\"disabled_mode_borrow\":true}");
}

struct OwnedLifetimeTests {OwnedLifetimeTests(){ownedTests();}};
static OwnedLifetimeTests ownedLifetimeTests;

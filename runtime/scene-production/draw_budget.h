/* Native admission policy; never changes the original pacing target. */
#define DRAW_BUDGET_WARM 16u
#define DRAW_BUDGET_WINDOW_MS 60000u
#define DRAW_BUDGET_CASES 2416u
#define DRAW_BUDGET_RESERVE_MS 3u
#define DRAW_BUDGET_WORDS 152u
static int draw_budget_fits(u32 elapsed,u32 target,u32 prediction,u32 reserve){
 if(target!=50||elapsed>=target||!prediction||prediction>target)return 0;
 u32 remaining=target-elapsed;
 return reserve<=remaining&&prediction<=remaining-reserve;
}

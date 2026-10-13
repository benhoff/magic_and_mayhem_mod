#pragma once
#include "menu_v19.h"
#define MNM_MENU_V20_MAGIC "MNMMCM20"
#define MNM_MENU_V20_SIZE 393216
#define MNM_MENU_REGION_CHARACTER 50
#define MNM_MENU_REGION_SPELLBOX 51
#define MNM_MENU_HANDOFF_CAMPAIGN_RETURN 4
/* Existing offsets unchanged. V7 Region word7 becomes enterHandoff:
 * zero if Enter unavailable,3 original battle,4 original campaign-menu return.
 * Region actions additionally offer Spellbox8 and Character16. Caller4/depth4
 * remains fixed; settled original realm names/region counts and different0/1
 * are admitted separately from the preserved fresh first-region path.
 * Experimental only; no invented campaign progress or arbitrary caller ingress.
 */

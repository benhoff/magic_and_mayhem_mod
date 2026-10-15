#pragma once
#include "../../reconstruction/rendering/panel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
/* PUSHAD order: EDI,ESI,EBP,ESP,EBX,EDX,ECX,EAX,EFLAGS. Called after
 * original Lock succeeds; original Unlock and wrapper epilogue remain active. */
typedef int (*MnmPanelResolve)(uint32_t *, unsigned, struct MnmPanelPlane *,
                               struct MnmPanelRequest *);
extern MnmPanelResolve mnm_panel_resolve;
extern uintptr_t mnm_panel_original[3], mnm_panel_resume[3];
extern uint32_t mnm_panel_native[3], mnm_panel_forwarded[3];
extern void (*mnm_panel_notice)(unsigned,int);
int mnm_panel_route(uint32_t *, unsigned);
#ifdef __cplusplus
}
#endif

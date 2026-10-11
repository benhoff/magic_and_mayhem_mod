/* Link the production shared producer entry into the independent Linux oracle.
 * Win32 admission is validated separately; this tests the real RET16/FX ABI. */
#include "../runtime/scene/glyph_route.h"
extern void mnm_glyph_counted_original(void);
void (*_producer_trampolines[52])(void)={[10]=mnm_glyph_counted_original};
int _producer_enter_observe(uint32_t* saved,uint32_t tag,uint8_t* fx){return tag==10?mnm_glyph_route(saved,fx):0;}
void _producer_leave_observe(uint32_t* saved){(void)saved;}

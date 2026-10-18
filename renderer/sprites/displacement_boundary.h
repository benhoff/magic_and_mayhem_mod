#pragma once
#include <stdint.h>
/* Native finite-canvas policy, matching SpriteComposite's explicit
 * retainOutsideDisplacement option. No pixel ownership is enlarged. */
static inline int mnm_retain_outside_displacement(void *context,uint32_t destination,
                                                 uint32_t offset,int write){
 (void)destination;(void)offset;
 if(write&&context)++*(uint32_t*)context;
 return 1;
}

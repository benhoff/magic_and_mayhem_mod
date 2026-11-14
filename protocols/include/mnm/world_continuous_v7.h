#ifndef MNM_WORLD_CONTINUOUS_V7_H
#define MNM_WORLD_CONTINUOUS_V7_H
#include "world_continuous_v6.h"
/* V7 combines V5 owned World receipts with V6 apply-before-ACK source chunks.
 * World generations/ACK identity and source-only ordinals remain separate.
 * Same bounded slots; no World credit or GPU-completion claim from source ACK.
 * Earlier peers reject this envelope. */
#define MNM_CONT_OWNED_PREFIX_MAGIC "MNMCON07"
#define MNM_CONT_OWNED_PREFIX_VERSION 7u
#define MNM_CONT_OWNED_PREFIX_MODE 4u
#endif

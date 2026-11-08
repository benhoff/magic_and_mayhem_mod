#pragma once
/* Passive private file. No pointer values cross this contract.
 * Each actor owns 12 raw producer roles; frame tokens expire every generation.
 * v1 remains a historical observation format, including its missing-role probe.
 */
#define MNM_SAO_MAGIC "MNMSAO02"
#define MNM_SAO_END "MNMSAOE2"
#define MNM_SAO_VERSION 2u
#define MNM_SAO_BUILD 0x40209ca7u
#define MNM_SAO_HEADER 64u
#define MNM_SAO_FRAME 128u
#define MNM_SAO_ACTOR 800u
#define MNM_SAO_DRAW 80u
#define MNM_SAO_ACTORS 256u
#define MNM_SAO_DRAWS 4096u
#define MNM_SAO_FRAMES 8192u
#define MNM_SAO_BYTES (256u*1024u*1024u)
#define MNM_SAO_ROLES 12u
#define MNM_SAO_TOKENS (MNM_SAO_ACTORS*MNM_SAO_ROLES)
#define MNM_SAO_ANI 28u
#define MNM_SAO_BOUNDS 160u
#define MNM_SAO_INDICES 172u
#define MNM_SAO_GATES 184u

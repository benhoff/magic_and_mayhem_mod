#ifndef MNM_REQUIRED_BOUNDARIES_V1_H
#define MNM_REQUIRED_BOUNDARIES_V1_H
/* Serialized little-endian diagnostics, never shared host pointers/objects.
 * Guest addresses are execution-local identities only. No pixel payloads. */
#define MNM_BOUNDARY_MAGIC "MNMBND01"
#define MNM_BOUNDARY_VERSION 1u
#define MNM_BOUNDARY_HEADER 64u
#define MNM_BOUNDARY_ROW 512u
#define MNM_BOUNDARY_ROWS 8192u
#define MNM_BOUNDARY_HOOKS 23u
#define MNM_BOUNDARY_ENTER 1u
#define MNM_BOUNDARY_RETURN 2u
#define MNM_BOUNDARY_WORLD 3u
#define MNM_BOUNDARY_END 4u
#endif

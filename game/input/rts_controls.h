#pragma once
#include <stdint.h>

/* Intentional input policy. Values are owned snapshots, never game pointers. */
#define MNM_RTS_MAX_ACTORS 256u
#define MNM_RTS_NO_GROUP UINT32_MAX
enum MnmRtsAction {
    MNM_RTS_WIZARD = 1, MNM_RTS_FOCUS, MNM_RTS_IDLE, MNM_RTS_UNGROUPED,
    MNM_RTS_SAME_TYPE, MNM_RTS_CYCLE_TYPE, MNM_RTS_GROUP_RECALL,
    MNM_RTS_GROUP_SAVE, MNM_RTS_GROUP_ADD, MNM_RTS_GROUP_REMOVE
};
typedef struct {
    uint32_t slot, generation, type, owner, group;
    int32_t health;
    uint32_t active, selected, wizard, idle;
} MnmRtsActor;
typedef struct {
    MnmRtsActor actors[MNM_RTS_MAX_ACTORS];
    uint32_t count, owner, session;
} MnmRtsSnapshot;
typedef struct {
    uint8_t selection[MNM_RTS_MAX_ACTORS], group_write[MNM_RTS_MAX_ACTORS];
    uint32_t groups[MNM_RTS_MAX_ACTORS], focus_slot;
    uint32_t selection_write, accepted;
} MnmRtsResult;
typedef struct {
    uint32_t session, owner, cycle_type, cycle_valid;
    uint32_t generations[MNM_RTS_MAX_ACTORS];
    uint8_t cycle_members[MNM_RTS_MAX_ACTORS], last_selection[MNM_RTS_MAX_ACTORS];
    uint32_t last_action, last_argument, last_time, last_valid;
} MnmRtsState;
void mnm_rts_reset(MnmRtsState*);
/* additive affects selection only. repeat never advances a cycle/double tap. */
int mnm_rts_apply(MnmRtsState*, const MnmRtsSnapshot*, uint32_t action,
                  uint32_t argument, int additive, uint32_t now,
                  uint32_t double_tap_ms, int repeat, MnmRtsResult*);

#include "rts_controls.h"

static int eligible(const MnmRtsSnapshot* s, uint32_t i) {
    const MnmRtsActor* a = s->actors + i;
    return a->active && a->health > 0 && a->owner == s->owner;
}
void mnm_rts_reset(MnmRtsState* state) {
    unsigned char* p = (unsigned char*)state;
    for (uint32_t i = 0; i < sizeof(*state); ++i) p[i] = 0;
}
int mnm_rts_apply(MnmRtsState* state, const MnmRtsSnapshot* s,
                  uint32_t action, uint32_t argument, int additive,
                  uint32_t now, uint32_t double_tap_ms, int repeat,
                  MnmRtsResult* out) {
    unsigned char* bytes = (unsigned char*)out;
    for (uint32_t i = 0; i < sizeof(*out); ++i) bytes[i] = 0;
    out->focus_slot = UINT32_MAX;
    if (s->count > MNM_RTS_MAX_ACTORS || s->owner >= 80 || !s->session ||
        action < MNM_RTS_WIZARD || action > MNM_RTS_GROUP_REMOVE ||
        double_tap_ms > 1000 ||
        (action >= MNM_RTS_GROUP_RECALL && argument > 9)) return 0;
    for (uint32_t i = 0; i < s->count; ++i)
        if (s->actors[i].slot != i || !s->actors[i].generation ||
            (s->actors[i].group > 9 && s->actors[i].group != MNM_RTS_NO_GROUP)) return 0;
    if (state->session != s->session || state->owner != s->owner) {
        mnm_rts_reset(state); state->session = s->session; state->owner = s->owner;
    }
    if (repeat) return 1;
    int cycle_matches = state->cycle_valid;
    for (uint32_t i = 0; i < MNM_RTS_MAX_ACTORS; ++i) {
        if (i < s->count) {
            out->selection[i] = eligible(s, i) && s->actors[i].selected;
            out->groups[i] = s->actors[i].group;
            if (state->last_selection[i] != out->selection[i] ||
                (state->cycle_members[i] &&
                 (!eligible(s, i) || state->generations[i] != s->actors[i].generation)))
                cycle_matches = 0;
        } else if (state->cycle_members[i] || state->last_selection[i]) cycle_matches = 0;
    }
    uint32_t first = UINT32_MAX;
    for (uint32_t i = 0; i < s->count; ++i)
        if (out->selection[i]) { first = i; break; }
    int double_tap = state->last_valid && state->last_action == action &&
        state->last_argument == argument && double_tap_ms &&
        (uint32_t)(now - state->last_time) <= double_tap_ms;
    if (action >= MNM_RTS_GROUP_SAVE) {
        for (uint32_t i = 0; i < s->count; ++i) {
            if (!eligible(s, i)) continue;
            int selected = out->selection[i];
            if ((action == MNM_RTS_GROUP_SAVE &&
                 (selected || s->actors[i].group == argument)) ||
                (action == MNM_RTS_GROUP_ADD && selected) ||
                (action == MNM_RTS_GROUP_REMOVE && selected && s->actors[i].group == argument)) {
                out->group_write[i] = 1;
                out->groups[i] = selected && action != MNM_RTS_GROUP_REMOVE ? argument : MNM_RTS_NO_GROUP;
            }
        }
    } else if (action == MNM_RTS_FOCUS) {
        out->focus_slot = first;
    } else {
        uint32_t desired_type = UINT32_MAX;
        if (action == MNM_RTS_SAME_TYPE && first != UINT32_MAX)
            desired_type = s->actors[first].type;
        if (action == MNM_RTS_CYCLE_TYPE) {
            if (!cycle_matches) {
                for (uint32_t i = 0; i < MNM_RTS_MAX_ACTORS; ++i) {
                    state->cycle_members[i] = i < s->count && out->selection[i];
                    state->generations[i] = i < s->count ? s->actors[i].generation : 0;
                }
                state->cycle_type = UINT32_MAX;
            }
            uint32_t smallest = UINT32_MAX, next = UINT32_MAX;
            for (uint32_t i = 0; i < s->count; ++i) if (state->cycle_members[i]) {
                uint32_t t = s->actors[i].type;
                if (t < smallest) smallest = t;
                if (t > state->cycle_type && t < next) next = t;
            }
            desired_type = next == UINT32_MAX ? smallest : next;
            state->cycle_type = desired_type; state->cycle_valid = desired_type != UINT32_MAX;
        } else state->cycle_valid = 0;
        uint32_t matches = 0;
        uint32_t next_actor = UINT32_MAX, first_actor = UINT32_MAX;
        for (uint32_t i = 0; i < s->count; ++i) if (eligible(s, i)) {
            const MnmRtsActor* a = s->actors + i;
            int match = action == MNM_RTS_WIZARD ? a->wizard != 0 :
                action == MNM_RTS_IDLE ? a->idle && !a->wizard :
                action == MNM_RTS_UNGROUPED ? a->group == MNM_RTS_NO_GROUP && !a->wizard :
                action == MNM_RTS_GROUP_RECALL ? a->group == argument :
                a->type == desired_type && (action != MNM_RTS_CYCLE_TYPE || state->cycle_members[i]);
            if (match) {
                ++matches;
                if (first_actor == UINT32_MAX) first_actor = i;
                if (next_actor == UINT32_MAX && (first == UINT32_MAX || i > first)) next_actor = i;
            }
        }
        /* Empty searches retain selection; an empty saved team can be saved. */
        if (matches) {
            if (next_actor == UINT32_MAX) next_actor = first_actor;
            out->selection_write = 1;
            for (uint32_t i = 0; i < s->count; ++i) {
                const MnmRtsActor* a = s->actors + i;
                int match = eligible(s, i) && (action == MNM_RTS_WIZARD ? a->wizard != 0 :
                    action == MNM_RTS_IDLE ? a->idle && !a->wizard :
                    action == MNM_RTS_UNGROUPED ? a->group == MNM_RTS_NO_GROUP && !a->wizard :
                    action == MNM_RTS_GROUP_RECALL ? a->group == argument :
                    a->type == desired_type && (action != MNM_RTS_CYCLE_TYPE || state->cycle_members[i]));
                if ((action == MNM_RTS_IDLE || action == MNM_RTS_UNGROUPED) && !argument)
                    match = match && i == next_actor;
                out->selection[i] = match || (additive && out->selection[i]);
                if (match && out->focus_slot == UINT32_MAX && double_tap &&
                    (action == MNM_RTS_WIZARD || action == MNM_RTS_GROUP_RECALL))
                    out->focus_slot = i;
            }
        }
    }
    state->last_action = action; state->last_argument = argument;
    state->last_time = now; state->last_valid = !double_tap;
    for (uint32_t i = 0; i < MNM_RTS_MAX_ACTORS; ++i) state->last_selection[i] = out->selection[i];
    out->accepted = 1;
    return 1;
}

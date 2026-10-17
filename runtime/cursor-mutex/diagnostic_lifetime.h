#pragma once
/* Native diagnostic model. Caller serializes every operation; callbacks happen
 * outside this model. Handles stay private to the owning module. */
typedef struct {
    void* original;
    void* duplicate;
    unsigned identity;
    unsigned leases;
    unsigned retired;
    unsigned closing;
} MnmCursorLifetime;
static int mnm_cursor_lifetime_publish(MnmCursorLifetime* cell, void* original,
                                       void* duplicate, unsigned identity) {
    if (cell->original || cell->duplicate || cell->identity || cell->leases || cell->closing || !original ||
        !duplicate || original == duplicate || !identity) return 0;
    cell->original=original;cell->duplicate=duplicate;cell->identity=identity;
    cell->retired=0;
    return 1;
}
static int mnm_cursor_lifetime_reserve(MnmCursorLifetime* cell, void* original,
                                       unsigned identity) {
    if (!cell->duplicate || cell->retired || cell->closing ||
        cell->original!=original || cell->identity!=identity ||
        cell->leases==~0u) return 0;
    ++cell->leases;
    return 1;
}
static int mnm_cursor_lifetime_retire(MnmCursorLifetime* cell, void* original,
                                      unsigned identity) {
    if (!cell->duplicate || cell->original!=original || cell->identity!=identity)
        return 0;
    cell->retired=1;
    return 1;
}
static int mnm_cursor_lifetime_return(MnmCursorLifetime* cell, unsigned identity) {
    if (!cell->duplicate || cell->identity!=identity || !cell->leases) return 0;
    --cell->leases;
    return 1;
}
/* Claim one close attempt after leases drain; no new reservations after retire.
 * Leave duplicate resident on failed Close, preventing reuse/publication. */
static void* mnm_cursor_lifetime_close_begin(MnmCursorLifetime* cell) {
    if (!cell->duplicate || !cell->retired || cell->leases || cell->closing)
        return 0;
    cell->closing=1;
    return cell->duplicate;
}
static int mnm_cursor_lifetime_close_end(MnmCursorLifetime* cell, void* duplicate,
                                        unsigned identity, unsigned succeeded) {
    if (!cell->closing || cell->duplicate!=duplicate || cell->identity!=identity)
        return 0;
    cell->closing=0;
    if (succeeded) {cell->duplicate=0;cell->original=0;cell->identity=0;}
    return 1;
}

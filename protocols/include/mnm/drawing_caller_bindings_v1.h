#pragma once
/* Optional state companion for opaque-wrapper context2 rows. No expected state
 * or target admission is supplied. Header64: magic, version, header bytes, row
 * bytes, maximum rows, original build, nine zero DWORDs. Each32-byte row holds
 * state ID, thread, caller, tag, queue, incoming sequence, actual6903dc target,
 * actual6c4e3c target. Targets remain raw even when unknown. Terminal row has
 * ID0, binding row count, all completed state count, active count, failures,
 * three zero DWORDs. Every context2 state must have exactly one matching row.
 * The final evidence package binds both files and their source/session identity. */
#define MNM_CALLER_BINDINGS_MAGIC "MNMCSB01"
#define MNM_CALLER_BINDINGS_VERSION 1u
#define MNM_CALLER_BINDINGS_ROW_BYTES 32u

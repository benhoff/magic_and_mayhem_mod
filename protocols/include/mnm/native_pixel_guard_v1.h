#pragma once
#define MNM_PIXEL_GUARD_FILE "native-pixel-forward-refusal.bin"
#define MNM_PIXEL_GUARD_VERSION 1u
#define MNM_PIXEL_GUARD_BYTES 128u
/* Refusal DWORDs: magic MNMPGRD1 (two), version, bytes, reason,
 * family, route, event, enabled, installed masks[6], expected masks[6],
 * zero[11]. Reasons1 selector,2 counters absent,3 unexpected route,
 * 4 duplicate install,5 missing install,6 original forwarding attempted,
 * 7 route not installed. Event0 install,1 native success,2 forward attempt.
 * A refusal is emitted before the callback can return to an original fallback.
 * This guard covers these six families only, not every original pixel writer.
 */

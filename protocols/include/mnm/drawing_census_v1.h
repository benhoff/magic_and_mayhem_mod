#pragma once
/* Little-endian words; process addresses are diagnostic identities, never host
 * pointers. Header/terminal records are 64 bytes, events are 128 bytes. */
#define MNM_DRAW_CENSUS_MAGIC "MNMDCN01"
#define MNM_DRAW_CENSUS_END "MNMDCEND"
#define MNM_DRAW_CENSUS_CONTROL "MNMDCTL1"
#define MNM_DRAW_CENSUS_RECORD 128u
#define MNM_DRAW_CENSUS_MAX_RECORDS 4194304u
enum { MNM_DRAW_CENSUS_ENTER=1, MNM_DRAW_CENSUS_RETURN=2, MNM_DRAW_CENSUS_SITE=3 };
/* Header words: magic[2], version, header size, event size, runtime module base,
 * hook count, max events, original SHA256[8].
 * Event words: seq,type,call ID,tag,phase,thread,entry,caller,SP,ECX,EDX,EAX,
 * EFLAGS,last error,tick,args[6],Video global,draw binding,pixel format,
 * screen stack depth,queue count/capacity/view,live frames,reserved[3].
 * End words: magic[2],version,events,calls,lost events,open calls,IO failure,
 * reserved[8]. A successful terminal marker proves journal integrity only.
 * Control words: magic[2],version,size,host sequence,phase,alive,reserved[9].
 * Phase 0 is unassigned;1..14 follow drawing-inventory.json scenario order.
 * Host writes odd sequence, phase/alive, then even sequence. No engine writes.
 */

#pragma once
/* Separate PE32 diagnostic wire contract: little-endian words, never host
 * pointers or a declaration that an indirect target is a particular API. */
#define MNM_DRAW_API_MAGIC "MNMDAPI1"
#define MNM_DRAW_API_END "MNMDAEND"
#define MNM_DRAW_API_RECORD 128u
#define MNM_DRAW_API_MAX_RECORDS 4194304u
enum { MNM_DRAW_API_BEFORE=1, MNM_DRAW_API_AFTER=2 };
enum { MNM_DRAW_API_REGISTER=1, MNM_DRAW_API_SLOT_READ=2,
       MNM_DRAW_API_SLOT_UNREADABLE=3, MNM_DRAW_API_DIRECT=4 };
/* Header (64 bytes): magic[2],version=1,size=64,row size=128,original base,
 * hook count,max records,original executable SHA256[8].
 * Row (32 words): seq,type,call ID,tag,phase,thread,site,actual ESP,ECX,EDX,EAX,
 * EFLAGS,LastError,tick,target state,target slot,target value,stack state,
 * pre-CALL stack words[8],active frames,pre-CALL ESP,EBX,EBP,ESI,EDI.
 * BEFORE stack state is 0 unreadable or 1 readable; unreadable values are zero.
 * AFTER stack state/words are zero. Target state/slot/value and pre-CALL ESP
 * repeat the matched BEFORE identity; AFTER never dereferences a released object.
 * Register/direct targets have slot=0; unreadable slots have value=0.
 * Actual cleanup is AFTER ESP minus BEFORE ESP, not a guessed calling convention.
 * End (64 bytes): magic[2],version,events,calls,lost,open,IO failure,reserved[8].
 * Loss, open calls, IO failure, missing/invalid terminal invalidate the journal.
 * Integrity does not prove all drawing writers, API outputs or resource lifetimes.
 */

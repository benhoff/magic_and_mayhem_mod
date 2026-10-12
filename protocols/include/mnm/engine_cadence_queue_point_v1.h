#pragma once
/* Explicit DWORD point, never host pointers or compiler object layout. */
#define MNM_CADENCE_QUEUE_POINT_VERSION 1u
#define MNM_CADENCE_QUEUE_POINT_BYTES 64u
#define MNM_CADENCE_QUEUE_POINT_WORDS 16u
/* size,version,eventOrdinal,counter,acceptedSequence,modeByte,ownerThread,
 * callerID(1=004fd4b0,2=004fd559),QPClo,QPChi,frequencyLo,frequencyHi,
 * eventType8,activeWorldBoolean,reserved0,reserved0. */

#pragma once
/* Optional passive final owned-source prefix. Header64 bytes: magic[8],
 * version1, header64, body bytes, World generation low/high, next source
 * ordinal low/high, session nonce low/high, queue0, World-open0, sequence,
 * slot state WRITING1, complete1. Body is the exact final CPSv3 prefix, at most
 *16MiB. It contains source inputs only, no destination planes. A reader must
 * reject World/failure records, torn bytes and a foreign/noncontiguous epoch.
 * This dump neither publishes source commands nor changes the live channel. */
#define MNM_CALLER_TAIL_MAGIC "MNMCST01"
#define MNM_CALLER_TAIL_VERSION 1u
#define MNM_CALLER_TAIL_HEADER_BYTES 64u
#define MNM_CALLER_TAIL_MAX_BYTES 16777216u

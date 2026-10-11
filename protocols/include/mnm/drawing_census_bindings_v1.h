#pragma once
/* Separate startup metadata; the core call journal remains MNMDCN01. */
#define MNM_DRAW_CENSUS_BINDINGS "MNMDCB01"
#define MNM_DRAW_CENSUS_BINDINGS_END "MNMDCBEN"
/* Header:128 bytes (32 DWORDs): magic[2], version1, header128, row96,
 * hook count, original base, core module base/size, named census_leave,
 * reserved[6], original SHA256[8], reserved[8].
 * Row:96 bytes (24 DWORDs): tag, original entry, prefix length, RET pop,
 * SITE flag, allocation base/size, entry stub, relocation count,
 * relative instruction offsets[3], original prefix[16 bytes], actual
 * relocated prefix plus continuation[24 bytes], reserved[2].
 * All unused bytes/offsets are zero. Terminal:64 bytes:magic[2],version1,
 * hook count,reserved[12]. Prepared allocations are not installed-hook proof.
 * Exact E8 return identities can be mapped to original instruction addresses;
 * E9 transfers and module/range membership never acquire CALL ownership.
 */

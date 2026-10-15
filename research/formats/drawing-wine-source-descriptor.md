# MNMWSD01 public source descriptor

Little-endian64-byte header,160-byte rows. Header words2–7 are version1,
row size160, original build40209ca7,16384record limit,2048resource-ID limit and
at most two extra public queries per sample; words8–15 are reserved zero.

Each sample row contains ordinal/type1, submission/API/thread/tick, source
texture/generation/subresource, portable resource ID, status/query result/error,
lifetime prefixes before/after, source generation after, ten public descriptor
words and the extra-query count. Words27–39 remain zero. Descriptor fields are
format, multisample type/quality, usage, bind/access flags, width/height/depth/size.
No host pointers or pixel bytes are exported.

Status0 is a successful query with the same known generation. Status1 is an
untrusted or absent source with no queries;2 is a null resource after one getter;
3 is a failed descriptor query with no descriptor payload;4 is a changed source
generation. These statuses do not prove storage ownership. Terminal type4 names
stop reason1or2 and the exact sample count; all other terminal fields are zero.
Duplicates, reserved fields, malformed status/query shapes and missing closure
refuse. Original blit outcomes remain in the independently joined submission
journal; this file records extra diagnostic queries only.

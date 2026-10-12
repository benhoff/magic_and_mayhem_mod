#ifndef MNM_RESIDENT_FRAME_BUDGET_H
#define MNM_RESIDENT_FRAME_BUDGET_H
/* Native V5 public policy only. No new wire field or original allocation.
 * Producer source slots and renderer-private preparation share the existing
 * renderer64handle/16Mpixel cap. Default V4/nonprepared residency is unchanged.
 */
#define MNM_RESIDENT_SOURCE_HANDLES 32u
#define MNM_RESIDENT_PREPARED_HANDLES 32u
#define MNM_RESIDENT_SOURCE_PIXELS (5u*1024u*1024u)
#define MNM_RESIDENT_PREPARED_PIXELS (11u*1024u*1024u)
static inline unsigned mnm_resident_source_pixel_limit(unsigned prepared) {
 return prepared?MNM_RESIDENT_SOURCE_PIXELS:16u*1024u*1024u;
}
#endif

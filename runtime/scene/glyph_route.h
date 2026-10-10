#ifndef MNM_GLYPH_ROUTE_H
#define MNM_GLYPH_ROUTE_H
#include "../../reconstruction/rendering/glyph_backend.h"
/* Explicit single-threaded ownership supplied by the embedding adapter. The
 * default NULL context forwards. No process addresses or hooks are installed
 * by this isolated adapter; live discovery/lifetime checks remain pending. */
typedef struct MnmGlyphRouteContext {
    MnmGlyphBackend backend;
    uint32_t handled,forwarded;
} MnmGlyphRouteContext;
extern MnmGlyphRouteContext *mnm_glyph_route_context;
int mnm_glyph_route(uint32_t *saved,uint8_t *fx);
#endif

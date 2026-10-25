#define _GNU_SOURCE
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef BACKEND
unsigned forwarding_calls;
xcb_get_geometry_cookie_t xcb_get_geometry(xcb_connection_t *c, xcb_drawable_t d) {
    (void)c; (void)d; errno = 33; return (xcb_get_geometry_cookie_t){9};
}
xcb_get_geometry_reply_t *xcb_get_geometry_reply(xcb_connection_t *c,
        xcb_get_geometry_cookie_t cookie, xcb_generic_error_t **error) {
    (void)c; (void)cookie; *error = NULL;
    xcb_get_geometry_reply_t *r = calloc(1, sizeof(*r)); if (!r) exit(8);
    r->width = r->height = 4; errno = 44; return r;
}
#define IMPL(name, incoming, outgoing, result) \
xcb_void_cookie_t name(xcb_connection_t *c, xcb_drawable_t d, xcb_gcontext_t gc, \
    uint16_t tw, uint16_t th, uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, \
    int16_t dx, int16_t dy, uint8_t depth, uint8_t format, uint8_t event, xcb_shm_seg_t segment, uint32_t offset) { \
    if (c != (void *)(uintptr_t)1 || d != 55 || gc != 99 || tw != 4 || th != 4 || sx || sy || \
        sw != 4 || sh != 4 || dx || dy || depth != 24 || format != 2 || event || segment != 77 || offset || errno != incoming) exit(9); \
    ++forwarding_calls; errno = outgoing; return (xcb_void_cookie_t){result}; \
}
IMPL(xcb_shm_put_image,101,119,0x12345678u)
IMPL(xcb_shm_put_image_checked,102,120,0x89abcdefu)
#else
extern unsigned forwarding_calls;
int main(void) {
    errno = 101;
    xcb_void_cookie_t a = xcb_shm_put_image((void *)(uintptr_t)1,55,99,4,4,0,0,4,4,0,0,24,2,0,77,0);
    if (a.sequence != 0x12345678u || errno != 119 || forwarding_calls != 1) return 1;
    errno = 102;
    xcb_void_cookie_t b = xcb_shm_put_image_checked((void *)(uintptr_t)1,55,99,4,4,0,0,4,4,0,0,24,2,0,77,0);
    if (b.sequence != 0x89abcdefu || errno != 120 || forwarding_calls != 2) return 2;
    puts("{\"success\":true,\"original_calls\":2,\"cookie_and_errno_preserved\":true}");
    return 0;
}
#endif

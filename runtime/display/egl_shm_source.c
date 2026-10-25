#define _GNU_SOURCE
/* Diagnostic-only dependency of the explicitly selected storage observer.
 * Public SysV/XCB interfaces only. Samples are external comparator oracles. */
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_SAMPLE (8u * 1024u * 1024u)
#define MAX_SAMPLES 16u
typedef struct { xcb_connection_t *connection; uint32_t xid, sysv, generation, connection_id; } Segment;
typedef struct { xcb_connection_t *connection; uint32_t xid, width, height, image_width, image_height, follow; } Drawable;
static Segment segments[128];
static Drawable drawables[32];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t generations, connections, samples, exhausted;

static void *next(const char *name) { return dlsym(RTLD_NEXT, name); }
static uint64_t now(void) {
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value)) return 0;
    return (uint64_t)value.tv_sec * 1000000000u + (uint64_t)value.tv_nsec;
}
static int enabled(void) { return getenv("MNM_DRAW_X11_DIRECTORY") != NULL; }
static void put32(unsigned char *out, uint32_t word) {
    for (unsigned i = 0; i < 4; ++i) out[i] = (unsigned char)(word >> (i * 8));
}
static void publish(const char *suffix, uint32_t sequence, const void *data, size_t size) {
    const char *directory = getenv("MNM_DRAW_X11_DIRECTORY");
    char path[4096];
    if (!directory || snprintf(path, sizeof(path), "%s/shm-source-%u-%06u.%s",
            directory, (unsigned)getpid(), sequence, suffix) >= (int)sizeof(path)) _exit(122);
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) _exit(122);
    const unsigned char *bytes = data;
    while (size) {
        ssize_t written = write(fd, bytes, size);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) _exit(122);
        bytes += written; size -= (size_t)written;
    }
    if (close(fd)) _exit(122);
}
static void remember(xcb_connection_t *c, uint32_t xid, uint32_t sysv) {
    if (!enabled()) return;
    pthread_mutex_lock(&lock);
    Segment *free_slot = NULL; uint32_t connection_id = 0;
    for (unsigned i = 0; i < 128; ++i) {
        if (segments[i].connection == c) connection_id = segments[i].connection_id;
        if (segments[i].generation && segments[i].connection == c && segments[i].xid == xid)
            free_slot = &segments[i];
        else if (!segments[i].generation && !free_slot) free_slot = &segments[i];
    }
    if (free_slot) *free_slot = (Segment){c, xid, sysv, ++generations, connection_id ? connection_id : ++connections};
    pthread_mutex_unlock(&lock);
}
#define ATTACH(name) \
xcb_void_cookie_t name(xcb_connection_t *c, xcb_shm_seg_t xid, uint32_t sysv, uint8_t read_only) { \
    typedef xcb_void_cookie_t (*Fn)(xcb_connection_t *, xcb_shm_seg_t, uint32_t, uint8_t); \
    int entry = errno; Fn real; void *p = next(#name); memcpy(&real, &p, sizeof(real)); \
    if (!real) { _exit(122); } errno = entry; xcb_void_cookie_t result = real(c, xid, sysv, read_only); \
    int saved = errno; remember(c, xid, sysv); errno = saved; return result; \
}
ATTACH(xcb_shm_attach)
ATTACH(xcb_shm_attach_checked)
#define DETACH(name) \
xcb_void_cookie_t name(xcb_connection_t *c, xcb_shm_seg_t xid) { \
    typedef xcb_void_cookie_t (*Fn)(xcb_connection_t *, xcb_shm_seg_t); \
    int entry = errno; Fn real; void *p = next(#name); memcpy(&real, &p, sizeof(real)); \
    if (!real) { _exit(122); } errno = entry; xcb_void_cookie_t result = real(c, xid); int saved = errno; \
    if (enabled()) { pthread_mutex_lock(&lock); for (unsigned i = 0; i < 128; ++i) { \
        if (segments[i].connection == c && segments[i].xid == xid) segments[i].generation = 0; } \
        pthread_mutex_unlock(&lock); } errno = saved; return result; \
}
DETACH(xcb_shm_detach)
DETACH(xcb_shm_detach_checked)

void xcb_disconnect(xcb_connection_t *c) {
    typedef void (*Fn)(xcb_connection_t *);
    int entry = errno; Fn real; void *p = next("xcb_disconnect"); memcpy(&real, &p, sizeof(real));
    if (!real) _exit(122);
    if (enabled()) {
        pthread_mutex_lock(&lock);
        for (unsigned i = 0; i < 128; ++i) if (segments[i].connection == c) memset(&segments[i], 0, sizeof(segments[i]));
        for (unsigned i = 0; i < 32; ++i) if (drawables[i].connection == c) memset(&drawables[i], 0, sizeof(drawables[i]));
        pthread_mutex_unlock(&lock);
    }
    errno = entry; real(c);
}

typedef struct { uint32_t words[46]; unsigned char *bytes, *second; void *mapped; size_t size; } Sample;
static Sample begin_sample(xcb_connection_t *c, const uint32_t args[16], int entry) {
    Sample result = {0}; result.mapped = (void *)-1;
    if (!enabled()) return result;
    xcb_generic_error_t *error = NULL;
    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(c, xcb_get_geometry(c, args[0]), &error);
    if (!geometry || error) { free(geometry); free(error); return result; }
    uint32_t width = geometry->width, height = geometry->height; free(geometry);
    Segment segment = {0}; uint32_t reason = 0, oldw = 0, oldh = 0, imagew = 0, imageh = 0;
    pthread_mutex_lock(&lock);
    for (unsigned i = 0; i < 128; ++i)
        if (segments[i].generation && segments[i].connection == c && segments[i].xid == args[14]) segment = segments[i];
    Drawable *d = NULL;
    for (unsigned i = 0; i < 32; ++i)
        if (drawables[i].connection == c && drawables[i].xid == args[0]) d = &drawables[i];
    if (!d) for (unsigned i = 0; i < 32; ++i) if (!drawables[i].connection) {
        d = &drawables[i]; *d = (Drawable){c, args[0], 0, 0, 0, 0, 0}; break;
    }
    if (d) {
        oldw = d->width; oldh = d->height; imagew = d->image_width; imageh = d->image_height;
        reason = !oldw ? 1u : oldw != width || oldh != height ? 2u : d->follow ? 4u : 0u;
        d->follow = reason == 1 || reason == 2; d->width = width; d->height = height;
        d->image_width = args[1]; d->image_height = args[2];
    }
    uint32_t sequence = reason && samples < MAX_SAMPLES ? ++samples : 0;
    if (reason && !sequence) exhausted++;
    pthread_mutex_unlock(&lock);
    if (!sequence) return result;
    uint32_t *w = result.words;
    w[0] = 1; w[1] = 192; w[2] = (uint32_t)getpid(); w[3] = (uint32_t)syscall(SYS_gettid);
    w[4] = sequence; w[5] = 1; w[6] = segment.connection_id; w[7] = segment.generation;
    w[8] = args[14]; w[9] = segment.sysv; w[10] = width; w[11] = height;
    w[12] = oldw; w[13] = oldh; w[18] = reason; w[40] = imagew; w[41] = imageh;
    w[43] = (uint32_t)entry; w[45] = 2; /* public XCB geometry, not an EGL allocation query */
    uint64_t time = now(); w[20] = (uint32_t)time; w[21] = (uint32_t)(time >> 32);
    memcpy(w + 24, args, 64);
    if (!segment.generation) { w[5] = 2; return result; }
    const xcb_setup_t *setup = xcb_get_setup(c); int layout = 0;
    for (xcb_format_iterator_t f = xcb_setup_pixmap_formats_iterator(setup); f.rem; xcb_format_next(&f))
        if (f.data->depth == 24 && f.data->bits_per_pixel == 32 && f.data->scanline_pad == 32) layout = 1;
    if (!layout || setup->image_byte_order != XCB_IMAGE_ORDER_LSB_FIRST || args[9] != 24 || args[10] != 2 ||
            args[11] || args[12] || args[13] || args[7] || args[8] || args[3] + width > args[1] ||
            args[4] + height > args[2] || args[5] < width || args[6] < height) { w[5] = 3; return result; }
    uint64_t size = (uint64_t)args[1] * args[2] * 4;
    struct shmid_ds info;
    if (!size || size > MAX_SAMPLE || shmctl((int)segment.sysv, IPC_STAT, &info) ||
            info.shm_segsz > UINT32_MAX || args[15] > info.shm_segsz || size > info.shm_segsz - args[15]) {
        w[5] = 4; return result;
    }
    w[15] = (uint32_t)info.shm_segsz; result.size = (size_t)size;
    result.mapped = shmat((int)segment.sysv, NULL, SHM_RDONLY);
    if (result.mapped == (void *)-1) { w[5] = 6; return result; }
    result.bytes = malloc(result.size); result.second = malloc(result.size);
    if (!result.bytes || !result.second) { w[5] = 4; return result; }
    const unsigned char *source = (const unsigned char *)result.mapped + args[15];
    memcpy(result.bytes, source, result.size); memcpy(result.second, source, result.size);
    w[42] = !memcmp(result.bytes, result.second, result.size);
    if (!w[42]) w[5] = 5;
    w[14] = (uint32_t)size;
    return result;
}
static void end_sample(Sample *s, uint32_t cookie, int saved) {
    if (!s->words[4]) return;
    uint32_t *w = s->words;
    if (w[5] == 1) {
        memcpy(s->second, (unsigned char *)s->mapped + w[39], s->size);
        w[17] = !memcmp(s->bytes, s->second, s->size);
        if (!w[17]) w[5] = 5;
    }
    if (s->mapped != (void *)-1) shmdt(s->mapped);
    w[16] = cookie; w[44] = (uint32_t)saved;
    uint64_t time = now(); w[22] = (uint32_t)time; w[23] = (uint32_t)(time >> 32);
    if (s->bytes && w[14]) publish("bgra", w[4], s->bytes, w[14]);
    unsigned char metadata[192] = {0}; memcpy(metadata, "MNMESH01", 8);
    for (unsigned i = 0; i < 46; ++i) put32(metadata + 8 + i * 4, w[i]);
    publish("bin", w[4], metadata, sizeof(metadata)); free(s->bytes); free(s->second);
}
#define PUT(name) \
xcb_void_cookie_t name(xcb_connection_t *c, xcb_drawable_t drawable, xcb_gcontext_t gc, \
    uint16_t tw, uint16_t th, uint16_t sx, uint16_t sy, uint16_t sw, uint16_t sh, \
    int16_t dx, int16_t dy, uint8_t depth, uint8_t format, uint8_t event, xcb_shm_seg_t segment, uint32_t offset) { \
    typedef xcb_void_cookie_t (*Fn)(xcb_connection_t *,xcb_drawable_t,xcb_gcontext_t,uint16_t,uint16_t, \
        uint16_t,uint16_t,uint16_t,uint16_t,int16_t,int16_t,uint8_t,uint8_t,uint8_t,xcb_shm_seg_t,uint32_t); \
    int entry = errno; Fn real; void *p = next(#name); memcpy(&real,&p,sizeof(real)); if (!real) _exit(122); \
    uint32_t args[16] = {drawable,tw,th,sx,sy,sw,sh,(uint32_t)(int32_t)dx,(uint32_t)(int32_t)dy, \
        depth,format,event,0,0,segment,offset}; Sample sample = begin_sample(c,args,entry); \
    errno = entry; xcb_void_cookie_t result = real(c,drawable,gc,tw,th,sx,sy,sw,sh,dx,dy,depth,format,event,segment,offset); \
    int saved = errno; end_sample(&sample,result.sequence,saved); errno = saved; return result; \
}
PUT(xcb_shm_put_image)
PUT(xcb_shm_put_image_checked)

__attribute__((destructor)) static void terminal(void) {
    if (!enabled() || !samples) return;
    uint32_t words[6] = {1, (uint32_t)getpid(), samples, exhausted, generations, connections};
    unsigned char data[24];
    for (unsigned i = 0; i < 6; ++i) put32(data + i * 4, words[i]);
    publish("terminal", 0, data, sizeof(data));
}

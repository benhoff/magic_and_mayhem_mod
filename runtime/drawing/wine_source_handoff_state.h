static int wine_handoff_transfer_writer(void*,u32);
#include "backing_lease_v1.h"
/* Reference-only per-COPY value ownership; no pixels enter the native app. */
static u32 wine_handoff_enabled,wine_handoff_snapshot,wine_handoff_records,wine_handoff_count;
static HANDLE wine_handoff_file;
static struct WineSourceHandoff {
 void* texture;void* resource;
 u32 submission,texture_id,generation,resource_id,width,height,pitch,size,crc;
 u32 old_client,old_owner,old_heap,old_heap_offset,old_heap_handle,old_data;
 u32 original_base,original_bytes,original_allocation,prior,thread,state,detached;
 u32 stage_base,stage_owner,epoch,provider,provider_object;
 struct MnmBackingRequestV1 provider_request;
 struct MnmBackingReadV1 provider_read;
} wine_handoffs[2048];
static void wine_handoff_wait_snapshot(void);
static int wine_handoff_defer_free(u32);
static int wine_handoff_prepare_restore(void*);
static void wine_handoff_heap_check(void*,void*,u32,u32);
static void wine_handoff_vm_check(void*,void**,u32*,u32);
static void wine_handoff_resource_guard(void*,u32);
static int wine_handoff_source_available(void*);
static int wine_handoff_source_active(void);
static int wine_handoff_fault_wait(u32);
static int wine_handoff_cs_allowed(u32,const u32*);

static void wine_handoff_retire(void*,void*,u32);

static u32 wine_source_ownership_required,wine_source_ownership_ready,wine_source_ownership_copies,wine_source_ownership_records;
static HANDLE wine_source_ownership_file;
static int wine_source_ownership_configure(void);
static int wine_source_ownership_open(void);
static int wine_source_ownership_install(void);
static void wine_source_ownership_require(void);
static void wine_source_ownership_close(void);

static int wine_handoff_stage_owned(u32);
static int wine_handoff_stage_writer(u32,void*,u32);

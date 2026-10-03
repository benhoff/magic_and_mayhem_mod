/* Standalone replay checkpoints for committed partial CPU writes. All pixels
 * are already owned; this path never reads an application/driver pointer. */
static u32 game_update_bytes;
static void game_update_commands(struct GameLock* lock,const struct Snapshot* after,u32 id){
    if(!lock->rectangle || !lock->base.data || !after->data)return;
    struct Snapshot* before=&lock->base;
    struct GameSurface* surface=game_surface_find(lock->object,0);
    if(before->bits==8 && (!surface || !game_surface_colors(surface,before->palette))){
        lock_diagnostic("update_palette_unobserved",lock->object,lock->kind,0,lock->flags,0,lock->owner,0);return;
    }
    u32 width=(u32)(lock->region.right-lock->region.left),height=(u32)(lock->region.bottom-lock->region.top);
    u32 row_bytes=width*(after->bits/8),stride=after->width*(after->bits/8);
    /* Exact envelope size, including one UPDATE record per normalized row. */
    u32 length=116+before->length+after->length+height*(32+row_bytes)+(before->bits==8?792:0);
    if(length>GAME_SURFACE_LIMIT-game_update_bytes){
        lock_diagnostic("update_limit",lock->object,lock->kind,0,lock->flags,0,lock->owner,0);return;
    }
    game_update_bytes+=length;
    char path[544];copy(path,lock_capture_path,lock_capture_path_length);char* tail=path+lock_capture_path_length;
    copy(tail,"\\update-",8);failure_hex(tail+8,id);copy(tail+16,".bin",5);
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);u32 sequence=0,header[4],surface_id=1;
    copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    int ok=file!=(HANDLE)-1 && write_all(file,header,16) && command_create(file,&sequence,surface_id,before);
    for(u32 y=0;ok && y<height;++y){
        u32 fields[5]={surface_id,(u32)lock->region.left,(u32)lock->region.top+y,width,1};
        const u8* row=after->data+fields[2]*stride+fields[1]*(after->bits/8);
        ok=command_record(file,&sequence,2,fields,20,row,row_bytes);
    }
    if(ok)ok=command_record(file,&sequence,5,&surface_id,4,after->data,after->length) &&
        command_record(file,&sequence,6,&surface_id,4,0,0) && command_record(file,&sequence,7,&surface_id,4,0,0) &&
        command_record(file,&sequence,8,0,0,0,0);
    if(file!=(HANDLE)-1)CloseHandle(file);
    lock_diagnostic(ok?"update_recorded":"update_file_failed",lock->object,lock->kind,0,lock->flags,0,lock->owner,0);
}

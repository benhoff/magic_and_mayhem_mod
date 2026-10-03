/* Bounded checkpoint session from a validated original-engine draw. IDs are
 * session-local, not COM pointers. This does not claim a complete live stream.
 * No additional observer calls or locks: reuse the accepted draw's snapshots.
 */
static int command_record(HANDLE file,u32* sequence,u32 operation,const void* fields,u32 field_length,
                          const void* bytes,u32 byte_length){
    u32 header[3]={operation,++*sequence,field_length+byte_length};
    return write_all(file,header,12) && (!field_length || write_all(file,fields,field_length)) &&
           (!byte_length || write_all(file,bytes,byte_length));
}
static int command_create(HANDLE file,u32* sequence,u32 id,const struct Snapshot* s){
    u32 fields[7]={id,s->width,s->height,s->bits,s->bits==8?0:s->r,s->bits==8?0:s->g,s->bits==8?0:s->b};
    if(!command_record(file,sequence,1,fields,28,s->data,s->length))return 0;
    if(s->bits==8){
        u32 p[3]={id,0,256};u8 rgb[768];
        for(u32 i=0;i<256;++i)copy(rgb+i*3,s->palette+i*4,3);
        if(!command_record(file,sequence,4,p,12,rgb,768))return 0;
    }
    return 1;
}
static void write_surface_commands(const struct DrawCapture* c){
    char path[512];u32 size=0,last=0;
    while(draw_path[size]){if(draw_path[size]=='\\')last=size;path[size]=draw_path[size];++size;}
    if(!last || last+20>=sizeof(path))return;
    copy(path+last,"\\commands-0001.bin",19);
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);if(file==(HANDLE)-1)return;
    u32 header[4],sequence=0;copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    u32 fields[10]={1,2,c->header[10],c->header[11],c->header[12],c->header[13],
                   c->header[14],c->header[15],c->header[6],c->header[6]?c->header[7]:0};
    u32 source=1,target=2;
    int ok=write_all(file,header,16) && command_create(file,&sequence,source,&c->src) &&
        command_create(file,&sequence,target,&c->before) && command_record(file,&sequence,3,fields,40,0,0) &&
        command_record(file,&sequence,5,&target,4,c->after.data,c->after.length) &&
        command_record(file,&sequence,6,&target,4,0,0) && command_record(file,&sequence,7,&source,4,0,0) &&
        command_record(file,&sequence,7,&target,4,0,0) && command_record(file,&sequence,8,0,0,0,0);
    (void)ok; /* Truncated sessions have no valid END and fail offline validation. */
    CloseHandle(file);
}

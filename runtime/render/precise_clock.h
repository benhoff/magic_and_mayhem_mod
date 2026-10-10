/* Intentional native policy: use an actual finer uptime API, never manufacture
 * ticks. Keep the original clock's LastError contract. Natural DWORD wrap. */
static PacerTickFn pacer_precise_tick;
static int precise_clock_epoch_matches(u32 coarse,u32 fine){
    i32 delta=(i32)(fine-coarse);return delta>=-32&&delta<=32;
}
static u32 precise_clock_read(PacerTickFn original,PacerTickFn fine){
    u32 entry=GetLastError(),value=original(),error=GetLastError();
    if(fine){SetLastError(entry);value=fine();}
    SetLastError(error);return value;
}

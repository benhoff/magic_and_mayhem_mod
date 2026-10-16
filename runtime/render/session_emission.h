/* Actual owned-session record emission, separated for independent byte/sequence
 * and bounded archive controls. Caller holds the tracker; state is host-local. */
static int session_emit(u32 op,const void* fields,u32 fl,const void* pixels,u32 length,u32 closing){
    if(!session_active)return 0;
    if(fl>0xffffffffu-12 || length>0xffffffffu-12-fl || session_sequence==0xffffffffu ||
       (!game_session_continuous && 12+fl+length>0xffffffffu-session_bytes)){game_session_gap(2);return 0;}
    u32 bytes=12+fl+length;
    /* Archive exhaustion is a diagnostic GAP, never a live transport GAP.
     * Reserve 32 DELETEs, END and GAP in the unchanged bounded archive. */
    if(session_file && !closing && (session_archive_sequence>=4096-session_cleanup_records() ||
       bytes>64*1024*1024-session_cleanup_bytes()-session_archive_bytes)){
        if(!game_session_continuous){game_session_gap(2);return 0;}
        session_archive_gap(2);
    }
    ++session_sequence;
    if(session_file && !command_record(session_file,&session_archive_sequence,op,fields,fl,pixels,length)){
        session_archive_close();lock_diagnostic("session_file_failed",0,0,0,0,0,0,0);
        if(!game_session_continuous){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);session_active=0;return 0;}
    }else if(session_file)session_archive_bytes+=bytes;
    if(!command_channel_record(session_sequence,op,fields,fl,pixels,length)){
        command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);session_archive_close();session_active=0;
        lock_diagnostic("session_channel_failed",0,0,0,0,0,0,0);return 0;
    }
    if(!game_session_continuous)session_bytes+=bytes;
    return 1;
}
static int session_record(u32 op,const void* fields,u32 fl,const void* pixels,u32 length){
    if(!session_active)return 0;
    /* Sequence identities always reserve orderly cleanup. Byte accounting is
     * cumulative only for bounded files; continuous storage is queue-bounded. */
    u32 limit=game_session_continuous?0xffffffffu:64*1024*1024;
    if(session_sequence>=(game_session_continuous?0xffffffffu-session_cleanup_records():4096-session_cleanup_records()) ||
       fl>limit-session_cleanup_bytes()-12 || length>limit-session_cleanup_bytes()-12-fl || (!game_session_continuous && (session_bytes>limit-session_cleanup_bytes() ||
       12+fl+length>limit-session_cleanup_bytes()-session_bytes))){
        game_session_gap(2);return 0;
    }
    return session_emit(op,fields,fl,pixels,length,0);
}

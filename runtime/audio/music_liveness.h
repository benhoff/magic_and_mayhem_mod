#pragma once
/* Thread-confined monotonic32 watch; use the same PE owner lock as commands. */
struct MnmMusicWatchdog {unsigned heartbeat,last_tick,started,retired;};
static int mnm_music_alive(struct MnmMusicWatchdog* watch,unsigned now,unsigned heartbeat,unsigned ready,unsigned deadline){
    if(watch->retired)return 0;
    if(ready!=1 || (!watch->started && !heartbeat)){watch->retired=1;return 0;}
    if(!watch->started || heartbeat!=watch->heartbeat){watch->started=1;watch->heartbeat=heartbeat;watch->last_tick=now;}
    else if(now-watch->last_tick>deadline){watch->retired=1;return 0;}
    return 1;
}

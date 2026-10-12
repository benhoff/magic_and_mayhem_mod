#include "../runtime/audio/music_liveness.h"
#include <assert.h>
#include <limits.h>
int main(void){
    struct MnmMusicWatchdog watch={0};
    assert(mnm_music_alive(&watch,10,1,1,5000));
    assert(mnm_music_alive(&watch,5010,1,1,5000));
    assert(!mnm_music_alive(&watch,5011,1,1,5000));
    assert(!mnm_music_alive(&watch,5012,2,1,5000)); /* never replay after retirement */
    watch=(struct MnmMusicWatchdog){0};
    assert(!mnm_music_alive(&watch,0,0,1,5000));
    watch=(struct MnmMusicWatchdog){0};
    assert(!mnm_music_alive(&watch,0,1,0,5000));
    watch=(struct MnmMusicWatchdog){0};
    assert(mnm_music_alive(&watch,UINT_MAX-10,1,1,5000));
    assert(mnm_music_alive(&watch,10,1,1,5000));
    assert(mnm_music_alive(&watch,20,2,1,5000));
    assert(mnm_music_alive(&watch,5020,2,1,5000));
    assert(!mnm_music_alive(&watch,5021,2,1,5000));
    watch=(struct MnmMusicWatchdog){0};
    assert(mnm_music_alive(&watch,10,UINT_MAX,1,5000));
    assert(mnm_music_alive(&watch,11,0,1,5000)); /* heartbeat rollover */
    assert(!mnm_music_alive(&watch,12,0,2,5000));
    return 0;
}

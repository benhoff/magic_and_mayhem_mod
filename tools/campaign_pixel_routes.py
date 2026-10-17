"""Bounded physical rotation scenarios observed through production counters."""
import time

from native_pixel_counters import read_counters


def camera_scenarios(game, key, record, ensure_ready=None):
    """Give each rotation time to settle instead of cancelling it with another.

    Counters prove the production camera bodies ran. Key presses alone cannot
    prove an orientation, and a selected orientation with no draw cannot pass.
    No game state or route counters are written by this helper.
    """
    def snapshot():
        counters = read_counters(game)
        if counters['forwarded'] or counters['history_refused']:
            raise ValueError('Pixel camera scenario observed a forward or history refusal')
        return {r['route']: r['native'] for r in counters['families']['minimap'] if r['route'].startswith('camera')}

    if ensure_ready:ensure_ready()
    initial = snapshot();epoch_initial = initial
    seen = set()
    epoch_turns = 0;interruptions = 0
    for turn in range(8):
        before = snapshot()
        key(ord('.'), .18)
        time.sleep(1.5)
        after = snapshot()
        if any(after[r] < before[r] for r in before):
            raise ValueError('Pixel camera counter reset during scenario')
        advanced = [r for r in before if after[r] > before[r]]
        seen.update(advanced)
        epoch_turns += 1
        record('pixel-camera-sample',turn=turn,before=before,after=after,advanced=advanced)
        # Original switch 555e08 maps cases 0..3 to camera0,2,0,1.
        # Camera3 has no branch in this dispatcher. Preserve that baseline.
        if {'camera0','camera1','camera2'}<=seen and epoch_turns>=4:
            result = dict(initial=initial, final=after, observed=sorted(seen), turns=turn+1,
                          ready_epoch_initial=epoch_initial,ready_epoch_turns=epoch_turns,dialogue_interruptions=interruptions)
            record('pixel-camera-complete',**result)
            return result
        if epoch_turns>=4 and ensure_ready and ensure_ready():
            # Dialogue can recenter/freeze the original camera. Require four
            # fresh turns and all bodies after ordinary dismissal, within the
            # original eight-key budget; interrupted progress cannot pass.
            seen.clear();epoch_turns=0;interruptions+=1;epoch_initial=snapshot()
            record('pixel-camera-readiness-reset',turn=turn,initial=epoch_initial)
    raise ValueError('Physical rotation did not execute the three selected minimap camera routes')


def validate_camera_progress(inputs):
    samples = [a for a in inputs['actions'] if a['action']=='pixel-camera-sample']
    complete = [a for a in inputs['actions'] if a['action']=='pixel-camera-complete']
    if len(complete)!=1 or not 4<=len(samples)<=8 or [a['turn'] for a in samples]!=list(range(len(samples))):
        raise ValueError('Missing bounded settled camera sequence')
    result = complete[0];required = {'camera0','camera1','camera2'}
    if result['turns']!=len(samples) or set(result['observed'])!=required or not 4<=result['ready_epoch_turns']<=len(samples):
        raise ValueError('Incomplete fresh camera readiness epoch')
    for row in samples:
        if set(row['before'])!={'camera0','camera1','camera2','camera3'} or set(row['after'])!=set(row['before']):
            raise ValueError('Incomplete camera counter snapshot')
        if any(row['after'][r]<row['before'][r] for r in row['before']) or set(row['advanced'])!={r for r in row['before'] if row['after'][r]>row['before'][r]}:
            raise ValueError('Camera counter regression or forged progress')
    if any(result['final'][r]<=result['ready_epoch_initial'][r] for r in required):
        raise ValueError('Interrupted camera progress cannot prove readiness')
    return dict(verified=True,turns=result['turns'],ready_epoch_turns=result['ready_epoch_turns'],
                dialogue_interruptions=result['dialogue_interruptions'],observed=sorted(required))

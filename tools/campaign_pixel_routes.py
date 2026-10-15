"""Bounded physical rotation scenarios observed through production counters."""
import time

from native_pixel_counters import read_counters


def camera_scenarios(game, key, record):
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

    initial = snapshot()
    seen = set()
    for turn in range(8):
        before = snapshot()
        key(ord('.'), .18)
        time.sleep(1.5)
        after = snapshot()
        if any(after[r] < before[r] for r in before):
            raise ValueError('Pixel camera counter reset during scenario')
        advanced = [r for r in before if after[r] > before[r]]
        seen.update(advanced)
        record('pixel-camera-sample',turn=turn,before=before,after=after,advanced=advanced)
        # Original switch 555e08 maps cases 0..3 to camera0,2,0,1.
        # Camera3 has no branch in this dispatcher. Preserve that baseline.
        if {'camera0','camera1','camera2'}<=seen and turn>=3:
            result = dict(initial=initial, final=after, observed=sorted(seen), turns=turn+1)
            record('pixel-camera-complete',**result)
            return result
    raise ValueError('Physical rotation did not execute the three selected minimap camera routes')

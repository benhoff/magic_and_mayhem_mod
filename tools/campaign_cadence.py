"""Phase-local World imports, separate from presentation and whole-session FPS."""
import math


CLOCK = 'linux-clock-monotonic'


def number(value):
    if type(value) not in (float, int) or not math.isfinite(value):
        raise ValueError('Invalid cadence number')
    return value


def counter(value):
    if type(value) is str and value.isascii() and value.isdecimal() and len(value) <= 20:
        value = int(value)
    if type(value) is not int or not 0 <= value <= 0xffffffffffffffff:
        raise ValueError('Invalid cadence counter')
    return value


def analyze_cadence(flow, inputs, required_seconds, minimum_fps):
    required_seconds, minimum_fps = number(required_seconds), number(minimum_fps)
    if not 1 <= required_seconds <= 120 or not 1 <= minimum_fps <= 120:
        raise ValueError("Invalid cadence target")
    phases = []
    previous_phase_end = None
    between = 0.
    if len(inputs) < 2:
        raise ValueError('Cadence requires pre/post Mini phases')
    rows = flow['gameplay_rates']
    if any(type(r['cycle']) is not int or not 0 <= r['cycle'] < len(inputs) for r in rows):
        raise ValueError('Unexpected cadence cycle')
    for cycle, helper in enumerate(inputs):
        if helper.get('success') is not True or helper.get('host_clock') != CLOCK:
            raise ValueError('Missing physical helper monotonic clock')
        begin = number(helper['monotonic_start_seconds'])
        end = begin + number(helper['seconds'])
        intervals = helper['camera_dialogue']['intervals']
        if not 2 <= len(intervals) <= 32 or end <= begin:
            raise ValueError('Missing bounded readiness intervals')
        last = begin
        overhead = 0.
        for interval in intervals:
            lo, hi = number(interval['begin']), number(interval['end'])
            if not begin <= last <= lo < hi <= end:
                raise ValueError('Unordered/outside readiness interval')
            overhead += hi-lo
            last = hi
        if abs(overhead-number(helper['camera_dialogue']['overhead_seconds'])) > .0001:
            raise ValueError('Readiness timing differs from active-stress accounting')
        if end-begin-overhead < required_seconds:
            raise ValueError('Cadence helper shortened active stress')
        samples = [r for r in rows if r['cycle'] == cycle]
        if not samples:
            raise ValueError('Missing phase cadence samples')
        selected = []
        readiness_windows = outside_windows = 0
        last_end = previous_world = None
        for row in samples:
            if row.get('host_clock') != CLOCK:
                raise ValueError('Cadence clocks differ')
            lo, hi = counter(row['host_begin_us'])/1e6, counter(row['host_end_us'])/1e6
            duration = number(row['seconds'])
            first, final = counter(row['world_imports_begin']), counter(row['world_imports_end'])
            worlds = counter(row['world_imports'])
            if hi <= lo or duration <= 0 or abs(hi-lo-duration) > .03:
                raise ValueError('Cadence window duration differs')
            if final < first or worlds != final-first or (previous_world is not None and previous_world != first):
                raise ValueError('Cadence World import counter gap/regression')
            if last_end is not None and abs(lo-last_end) > .000002:
                raise ValueError('Overlapping/gapped cadence windows')
            previous_world, last_end = final, hi
            counter(row['native_frames']);counter(row['painted_frames'])
            if lo < begin or hi > end:
                outside_windows += 1
            elif any(lo < interval['end'] and hi > interval['begin'] for interval in intervals):
                readiness_windows += 1
            else:
                selected.append((row, hi-lo))
        if previous_phase_end is not None:
            gap = counter(samples[0]['host_begin_us'])/1e6-previous_phase_end
            if gap < 0:
                raise ValueError('Cadence phases overlap')
            between += gap
        previous_phase_end = last_end
        seconds = sum(duration for _, duration in selected)
        if seconds < max(1, required_seconds-3):
            raise ValueError('Insufficient complete active cadence windows')
        totals = {key: sum(counter(row[key]) for row, _ in selected)
                  for key in ('world_imports', 'native_frames', 'painted_frames')}
        rates = {key+'_fps': value/seconds for key, value in totals.items()}
        phases.append(dict(cycle=cycle, active_sample_seconds=seconds, complete_active_windows=len(selected),
                           readiness_seconds=overhead, readiness_windows=readiness_windows,
                           boundary_windows=outside_windows, long_active_windows=sum(d > 2 for _, d in selected),
                           totals=totals, rates=rates,
                           meets_world_floor=rates['world_imports_fps'] >= minimum_fps,
                           meets_paint_floor=rates['painted_frames_fps'] >= minimum_fps))
    return dict(verified=True, phases=phases, between_stress_phases_seconds=between,
                minimum_fps=minimum_fps, whole_journey_metric_unchanged=True,
                scope='Complete stress windows excluding only explicit readiness intervals and helper boundaries. '
                      'Counts actual World imports separately from command frames/paints. Other modal time is unclassified; '
                      'between-phase time is retained in whole-journey engine FPS. No FPS gate is bypassed, hardware/scanout '
                      'or original pixel equivalence asserted.')

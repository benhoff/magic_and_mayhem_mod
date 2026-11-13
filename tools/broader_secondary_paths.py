"""Rebind only unrequested continuation43 outputs and counters to continuation44."""
import copy
from pathlib import PurePosixPath
from rendering_pilot import require


def rebind(phase):
    q = copy.deepcopy(phase)
    old = q['collector_prefix']
    require(old.startswith('research/runtime/') and old.endswith('/continuation-43/')
            and '..' not in PurePosixPath(old).parts, 'Expected exact prior continuation namespace')
    new = old.removesuffix('continuation-43/') + 'continuation-44/'
    cases, outputs = q['broader_cases'], {}
    require(cases and all(v['reserved_original_games'] == 1 for v in cases), 'Only new single-start cases')
    for case in cases:
        for key in ('report', 'comparison', 'prepare_progression'):
            value = case.get(key)
            if value is None: continue
            require(isinstance(value, str) and value.startswith(old)
                    and PurePosixPath(value).parent == PurePosixPath(old), 'Output outside its collection')
            outputs[value] = new + PurePosixPath(value).name
    require(len({v['report'] for v in cases}) == len(cases), 'Duplicate prospective acquisition')

    def adapt(value):
        if isinstance(value, dict): return {k: adapt(v) for k, v in value.items()}
        if isinstance(value, list): return [adapt(v) for v in value]
        return outputs.get(value, value) if isinstance(value, str) else value
    q = adapt(q)
    q['collector_prefix'] = new
    require(all(q['broader_cases'][i].get('historical_primary') == case.get('historical_primary')
                for i, case in enumerate(cases)), 'Historical primary rewritten')
    counters = {}
    for origin, target in q.get('counter_bindings', {}).items():
        require(target.startswith('working/broader-player/bp43/') and '..' not in PurePosixPath(target).parts,
                'Unreviewed prior counter namespace')
        counters[origin] = target.replace('working/broader-player/bp43/', 'working/broader-player/bp44/', 1)
    require(counters and len(set(counters.values())) == len(counters), 'Missing or aliased counters')
    q['counter_bindings'] = counters
    require(q.get('bounds') == phase.get('bounds'), 'Inner technical limits changed')
    return q

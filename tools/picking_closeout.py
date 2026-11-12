"""Finite picking acceptance across original display recreation."""
from rendering_pilot import require

PREFIX = 'research/runtime/picking-closeout-results-20261109/'
BID = 'NP.picking-closeout'
SID = 'native.picking-closeout'
LEAF = 'journey.cursor-interaction.picking'
CONTROLS = {
    'tools/picking_closeout.py', 'tools/prepare-rendering-picking-closeout.py',
    'tools/run-rendering-picking-closeout.py',
    'tools/check-rendering-picking-closeout.py',
    'tools/close-rendering-picking-closeout.py', 'tests/test-picking-closeout.py',
}


def complete_cases(cases):
    require(len(cases) == 2 and all(type(c['software']) is bool for c in cases)
            and {c['software'] for c in cases} == {False, True},
            'Both actual picking profiles required')
    for case in cases:
        p = case['proof']
        require(case['success'] is True and case['sources_stable'] is True
                and case['comparison_processes_stopped'] is True
                and case['original_manifest_verified_before_after'] is True
                and case['comparison']['success'] is True
                and case['comparison']['mismatching_pixels'] == 0
                and p['verified'] is True and p['normal_quit'] is True
                and p['software'] is case['software']
                and p['windows'] == 48 and p['object_windows'] == 24
                and p['cell_windows'] == 24 and p['displayed_source_witnesses'] == 144
                and p['live_display_rebuilds'] == 2 and p['same_world_continuity'] is False,
                'Incomplete independently compared picking domain')
    return True


# Post-retirement host changes may qualify a preserved pilot only through a
# fresh retained-data comparison. Native/runtime/compiler inputs stay full-hash.
REVIEWABLE_CONTROLS = CONTROLS | {'tools/rendering_picking_resize_assessment.py'}


def preserved_pilot(acquired_sources, current_sources, changes, compared):
    actual = {n:dict(before=acquired_sources.get(n),after=h)
              for n,h in current_sources.items() if acquired_sources.get(n)!=h}
    require(actual == changes and set(actual) <= REVIEWABLE_CONTROLS
            and all(v['before'] and v['after'] for v in actual.values()),
            'Unreviewed execution source drift; native inputs cannot be waived')
    p=compared['proof']
    require(compared['success'] is True and compared['sources_stable'] is True
            and compared['comparison_processes_stopped'] is True
            and compared['original_manifest_verified_before_after'] is True
            and compared['comparison']['success'] is True
            and compared['comparison']['mismatching_pixels']==0
            and compared['retained_pixel_comparison']['new_original_reference_execution'] is False
            and compared['retained_pixel_comparison']['new_pixel_comparison'] is False
            and p['verified'] is True and p['normal_quit'] is True
            and p['same_world_continuity'] is False and p['windows']==48
            and p['object_windows']==p['cell_windows']==24
            and p['displayed_source_witnesses']==144 and p['live_display_rebuilds']==2,
            'Incomplete newly analyzed retained pilot')
    return True


def quiet_sources(samples):
    """Source-only readiness; no hover result or displayed pixels are inputs."""
    if len(samples)!=3:return False
    generations=[v['generation']for v in samples]
    if generations!=sorted(set(generations)):return False
    keys=[(v['nonce'],tuple(v['position']),tuple(v['anchor']),tuple(v['point']),v['view'])for v in samples]
    return keys[0]==keys[1]==keys[2]

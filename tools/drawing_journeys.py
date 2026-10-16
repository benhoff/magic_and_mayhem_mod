"""DRAW-02 journey requirements and independent reference/parity accounting.

A component canvas can contribute useful comparisons without closing a journey.
Coverage is derived from admitted frame/action contracts, never a stored pass bit.
"""
from drawing_corpus import SCENARIOS, BUILD, local, sha
import json

CATALOG = 'tests/fixtures/drawing-journeys/suite.json'
# These are acceptance requirements, not recovered engine behavior. Changes to
# this denominator require source review and cannot silently shrink a manifest.
REQUIREMENTS = {
    'enabled-movies': ('both-intros', 'movie-skip', 'return-to-menu'),
    'main-setup-menus': ('main-menu', 'setup-change', 'cancel-return'),
    'campaign-entry': ('campaign-selection', 'loading', 'first-composed-world'),
    'quick-battle-entry': ('setup-confirm', 'loading', 'first-composed-world'),
    'active-world': ('sustained-world', 'animation', 'visibility'),
    'camera-motion': ('pan', 'four-orientations', 'viewport-boundaries'),
    'creatures-effects': ('creature-animation', 'attachment', 'combat', 'lighting', 'effect-blending'),
    'hud-minimap-tooltips': ('hud-update', 'minimap-update', 'markers', 'tooltip-open-close'),
    'cursor-interaction': ('cursor-motion', 'hotspot', 'picking', 'selection'),
    'pause-preferences': ('pause', 'preferences-change', 'cancel', 'resume'),
    'save-load': ('save', 'load', 'restored-world'),
    'map-transitions': ('leave-map', 'loading', 'next-map'),
    'battle-results': ('victory', 'defeat', 'return-to-menu'),
    'shutdown': ('world-exit', 'menu-exit', 'cleanup'),
}
OPERATIONS = ('copy-opaque', 'copy-keyed', 'copy-clipped', 'copy-stretched',
              'copy-alias-overlap', 'fill-full', 'fill-clipped', 'partial-write',
              'cpu-write-readback', 'text-font-dc-lifetime', 'image-cropping',
              'sprite-indexed', 'sprite-direct', 'sprite-auxiliary', 'palette-remap',
              'palette-cycling', 'destination-dependency', 'flags-and-return-values',
              'surface-loss-restore', 'transition-recovery', 'failure-cleanup')
BOUNDARIES = ('operation', 'world_canvas', 'composed_canvas', 'observed_primary_blt', 'observed_egl_session', 'observed_egl_action_session', 'presented_frame')


def token(value):
    import re
    if not isinstance(value, str) or not re.fullmatch(r'[A-Za-z0-9_-]+', value):
        raise ValueError('Invalid journey identifier')
    return value


def admit_suite(root, catalog):
    if catalog['schema'] != 1 or catalog['source_sha256'] != BUILD:
        raise ValueError('Journey suite schema/build mismatch')
    if catalog['requirements'] != {s: list(REQUIREMENTS[s]) for s in SCENARIOS}:
        raise ValueError('Journey acceptance denominator changed')
    if catalog['operation_requirements'] != list(OPERATIONS):
        raise ValueError('Operation acceptance denominator changed')
    seen = set()
    for case in catalog['cases']:
        token(case['id'])
        if case['id'] in seen:
            raise ValueError('Duplicate suite case')
        seen.add(case['id'])
        if case['adapter'] not in ('components', 'scene', 'producer_sequence', 'publication', 'recorded_session', 'recorded_action_session', 'session'):
            raise ValueError('Unsupported journey adapter')
        path = local(root, case['catalog']['path'])
        if sha(path.read_bytes()) != case['catalog']['sha256']:
            raise ValueError('Journey catalog identity changed')
        if not isinstance(case['scope'], str) or not case['scope'].strip():
            raise ValueError('Missing suite case scope')
    if not seen:
        raise ValueError('Empty journey suite')
    return catalog


def coverage(observations):
    """Summarize only validated reference identities and completed comparisons.

    Callers must derive observations from admitted artifacts. Plain suite labels
    and successful component results do not constitute presented-frame evidence.
    Both coverage axes are reported even if the native implementation refuses.
    """
    references = {s: set() for s in SCENARIOS}
    parity = {s: set() for s in SCENARIOS}
    operations_reference, operations_parity = set(), set()
    partial, failures = [], []
    seen = set()
    for row in observations:
        token(row['id'])
        if row['id'] in seen:
            raise ValueError('Duplicate comparison observation')
        seen.add(row['id'])
        scenario = row['scenario']
        if scenario not in (*SCENARIOS, 'unattributed-surface'):
            raise ValueError('Unknown journey scenario')
        if row['boundary'] not in BOUNDARIES or row['outcome'] not in ('matching', 'mismatch', 'unsupported', 'error', 'pending'):
            raise ValueError('Invalid comparison boundary/outcome')
        gates = row.get('requirements', [])
        if len(gates) != len(set(gates)) or any(g not in REQUIREMENTS.get(scenario, ()) for g in gates):
            raise ValueError('Unknown/duplicate journey requirement')
        ops = row.get('operations', [])
        if len(ops) != len(set(ops)) or any(o not in OPERATIONS for o in ops):
            raise ValueError('Unknown/duplicate operation requirement')
        independent = row.get('independent_reference') is True
        # Explicit original presented-frame and ordered action/frame evidence is
        # mandatory. Empty output, sampled/discontinuous sequences or synthetic
        # references cannot close acceptance requirements.
        eligible = (independent and row['boundary'] == 'presented_frame' and
                    row.get('contiguous') is True and row.get('actions_bound') is True and
                    type(row.get('frames')) is int and row['frames'] > 0)
        if gates and not eligible:
            partial.append({'id': row['id'], 'scenario': scenario,
                            'reason': 'Journey requirements need contiguous, action-bound independent presented frames'})
        if eligible:
            references[scenario].update(gates)
            if row['outcome'] == 'matching':
                parity[scenario].update(gates)
        if independent:
            operations_reference.update(ops)
            if row['outcome'] == 'matching':
                operations_parity.update(ops)
        if row['boundary'] != 'presented_frame':
            partial.append({'id': row['id'], 'scenario': scenario,
                            'reason': 'Observed opaque-route publication does not establish complete route or input-action coverage' if row['boundary']=='observed_primary_blt' else row['boundary'] + ' comparison does not establish presentation'})
        if row['outcome'] in ('mismatch', 'unsupported', 'error', 'pending'):
            failures.append({'id': row['id'], 'outcome': row['outcome']})
    scenarios = []
    for s in SCENARIOS:
        required = REQUIREMENTS[s]
        scenarios.append({'id': s, 'required': list(required),
                          'reference_covered': [g for g in required if g in references[s]],
                          'native_matching': [g for g in required if g in parity[s]],
                          'reference_remaining': [g for g in required if g not in references[s]],
                          'parity_remaining': [g for g in required if g not in parity[s]],
                          'reference_complete': set(required) <= references[s],
                          'parity_complete': set(required) <= parity[s]})
    return {'scenarios': scenarios,
            'operations': {'required': list(OPERATIONS),
                           'reference_remaining': [o for o in OPERATIONS if o not in operations_reference],
                           'parity_remaining': [o for o in OPERATIONS if o not in operations_parity]},
            'reference_complete': all(s['reference_complete'] for s in scenarios) and set(OPERATIONS) <= operations_reference,
            'draw02_complete': all(s['parity_complete'] for s in scenarios) and set(OPERATIONS) <= operations_parity and not failures,
            'partial_boundaries': partial, 'nonmatching': failures}


def inventory(root, catalog_path=CATALOG):
    catalog = admit_suite(root, json.loads(local(root, catalog_path).read_text()))
    # Inventory reports requirements, never fabricates execution observations.
    return {'schema': 1, 'source_executable_sha256': BUILD,
            'suite_sha256': sha(local(root, catalog_path).read_bytes()),
            'cases': catalog['cases'], **coverage([])}

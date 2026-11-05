"""Finite ordinary player cohort; evidence applicability never grants acceptance."""
from pathlib import Path

from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate, verify_evidence
from rendering_normal_acceptance import admit, receipt
from rendering_pilot import require
from rendering_storage_acceptance import LEAVES as STORAGE

CONFIGURATIONS = ('player-default', 'player-software')
SCENARIOS = ('normal-storage', 'normal-quick')
SIZES = {(640, 480), (800, 600)}
LEAVES = dict(STORAGE, **{
    'journey.quick-battle-entry.setup-confirm': 'Original acknowledged map/setup and actual native configuration.',
    'journey.shutdown.menu-exit': 'Original Main Quit, stopped private process and zero graphics owners/forwarding.',
    'journey.campaign-entry.campaign-selection': 'Original Main campaign and New Campaign confirmation for the default initial region.',
    'journey.pause-preferences.preferences-change': 'Actual original Preferences OK and both display rebuild/rebind paths; resolution setting only.'})


def applicability(root, reader, register, plan):
    """Verify retained candidates once, retaining missing/invalid evidence as gaps."""
    rows = requirements(plan)
    candidates = {e for row in rows for e in row['candidate_evidence']}
    # Recent finite ordinary/fault proofs may postdate the frozen gap inventory.
    candidates.update(e['id'] for e in register['evidence']
                      if e['id'].startswith(('SO04.', 'PR04.', 'WR04.', 'SM01.')))
    verified = {}
    for eid in sorted(candidates):
        try:
            verified[eid] = verify_evidence(Path(root), reader, register, eid)
        except (ValueError, KeyError, OSError) as error:
            verified[eid] = dict(id=eid, payload_and_assertions_verified=False,
                                 error=str(error), closes_requirement=False)
    return dict(schema=1, kind='public_player_evidence_applicability', success=True,
                candidates=list(verified.values()), requirements=[dict(row, state='pending',
                applicability=[verified[e] for e in row['candidate_evidence']]) for row in rows],
                decisions={'supported-player': 'pending', 'original-recovery': 'pending'},
                scope='Source matching and retained assertions are candidate facts. Configuration and full requirement scope still need collector admission; no pass follows from this inventory.')


def first_presented(observed, comparison, reference):
    """Bind the first actual swapped World; coalesced source generations are allowed."""
    paint = next(p for p in observed['swaps'] if int(observed['paints'][p][0]['world']) > 0)
    row = observed['paints'][paint][0]
    matches = [r for r in comparison['comparisons'] if r['paint'] == paint]
    require(len(matches) == 1, 'First presented World comparison is ambiguous')
    match = matches[0]
    require(match['world'] == int(row['world']) and match['mismatching_pixels'] == 0
            and match['actual_sha256'] == match['expected_sha256']
            and match['pixels'] == row['width'] * row['height']
            and reference['continuous_from_owned_initial_history'] is True
            and reference['worlds'] >= match['world'], 'First presented World history is incomplete')
    return dict(verified=True, paint=paint, generation=match['world'], pixels=match['pixels'],
                sha256=match['actual_sha256'], complete_caller_preparation_proved=False,
                scope='Complete first swapped native viewport and continuous independent original history. This does not identify every preparation side effect or caller state.')


def progress(plan, reader, cohorts):
    validate(plan)
    require(len(cohorts) == 8, 'Frozen cohort requires exactly eight executions')
    runs, sources, identities, domains = [], {}, set(), set()
    for configuration, pair in cohorts:
        require(configuration in CONFIGURATIONS, 'Unknown player configuration')
        run = admit(reader, pair, configuration)
        require(run['scenario'] in SCENARIOS, 'Unknown ordinary journey')
        acquired = receipt(reader, pair['acquisition'])
        compared = receipt(reader, pair['comparison'])
        identity = acquired['session_identity']
        require(type(identity) is str and len(identity) == 32
                and all(c in '0123456789abcdef' for c in identity)
                and identity not in identities, 'Invalid or reused actual session')
        identities.add(identity)
        sizes = {tuple(p) for p in run['world_extents']}
        require(len(sizes) == 1 and sizes <= SIZES, 'Ambiguous actual World domain')
        domain = (configuration, run['scenario'], next(iter(sizes)))
        require(domain not in domains, 'Duplicate cohort domain')
        domains.add(domain)
        require(compared['first_presented_world']['verified'] is True, 'Missing first presented comparison')
        if run['scenario'] == 'normal-storage':
            menu = compared['original_menu_actions']
            require(menu['campaign_selection']['verified'] is True
                    and menu['campaign_selection']['callbacks'] == [[3, 0], [18, 65537], [18, 0]],
                    'Original campaign selection/confirmation missing')
            display = menu['display_rebuilds']
            expected = [[640, 480], [800, 600]] if sizes == {(800, 600)} else [[800, 600], [640, 480]]
            require(display['verified'] is True and display['dimensions'] == expected
                    and len(set(display['callback_sequences'])) == 2, 'Both original display changes required')
            cpu = compared['cpu_delivery']
            require(cpu['request_count'] > 0 and cpu['request_count'] == cpu['complete_reply_matches']
                    == cpu['counters']['completed'] and cpu['aggregate_completion_reconciles'] is True
                    and cpu['terminal_reply_matches'] is True, 'CPU reply/completion join incomplete')
        for name, digest in run['sources'].items():
            require(name not in sources or sources[name] == digest, 'Different current player source phases')
            sources[name] = digest
        runs.append(dict(run, configuration=configuration))
    require(domains == {(c, s, z) for c in CONFIGURATIONS for s in SCENARIOS for z in SIZES},
            'Missing frozen cohort domain')
    require(all(reader.digest(n) == h for n, h in sources.items()), 'Player evidence sources are stale')
    rows = []
    for row in requirements(plan):
        proof = []
        if row['id'] in LEAVES:
            scenario = 'normal-quick' if row['id'] in ('journey.quick-battle-entry.setup-confirm', 'journey.shutdown.menu-exit') else 'normal-storage'
            proof = [dict(reason=LEAVES[row['id']], execution=r['pair'], provider=r['provider'],
                          actual_world_extents=r['world_extents']) for r in runs
                     if r['configuration'] == row['configuration'] and r['scenario'] == scenario]
        rows.append(dict(row, state='passed' if proof else 'pending', admitted_execution_evidence=proof))
    require(len(rows) == 156 and sum(r['state'] == 'passed' for r in rows) == 16,
            'Finite player scope or denominator changed')
    return dict(schema=1, kind='public_player_cohort_progress', success=True,
                dossier_identity=fingerprint(requirements(plan)), sources=sources, executions=runs,
                requirements=rows, passed_requirements=16, pending_requirements=140,
                decisions={'supported-player': 'pending', 'original-recovery': 'pending'},
                historical_failures_retired=False,
                scope='Eight named finite leaves per profile, both sizes. Preferences credit covers resolution rebuilds only. First-World caller preparation, camera/picking/HUD/media, whole operations/gates, unknown branches and both final decisions remain pending.')


def main():
    import argparse
    import json
    from coverage_evidence import Reader, write_report
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--reconcile', action='store_true')
    mode.add_argument('--execution', nargs=3, action='append', metavar=('CONFIGURATION', 'ACQUISITION', 'COMPARISON'))
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    path = Path(args.report)
    require(path.as_posix().startswith('research/runtime/rendering-player-acceptance-results-20261104/')
            and '..' not in path.parts and not (root/path).exists(), 'New owned immutable report required')
    with Reader(root) as reader:
        plan = reader.json('docs/native-rendering-closure.json')
        if args.reconcile:
            result = applicability(root, reader, reader.json('research/runtime/coverage/register.json'), plan)
        else:
            cohorts = [(cfg, {k: dict(record=n, sha256=reader.digest(n)) for k, n in
                              [('acquisition', a), ('comparison', c)]}) for cfg, a, c in args.execution]
            result = progress(plan, reader, cohorts)
        result.setdefault('sources', {}).update({n: reader.digest(n) for n in
            ['tools/rendering_player_acceptance.py', 'tools/rendering_acceptance.py',
             'tools/rendering_storage_acceptance.py', 'tools/rendering_normal_acceptance.py',
             'tools/coverage_claims.py', 'docs/native-rendering-closure.json']})
        digest = write_report(root, path, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], record=str(path), sha256=digest,
                         passed=result.get('passed_requirements'), pending=result.get('pending_requirements'))))


if __name__ == '__main__': main()

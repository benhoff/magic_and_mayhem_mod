"""Account for current public-player coverage without widening historical proofs."""
import hashlib
from pathlib import Path

from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require

PREFIX = 'research/runtime/broader-player-acceptance-results-20261110/'
BID = 'NP.broader-player-acceptance'
SID = 'native.broader-player-acceptance'
PARENTS = (
    'PH09.pixels-history01', 'JG10.journeys02', 'OG09.merged-operations01',
    'FL09.fault-lifetime01', 'TG10.merged-transitions01', 'WP10.cohort03',
)
NATIVE_ROOTS = ('apps/', 'renderer/', 'runtime/', 'reconstruction/', 'assets/', 'audio/', 'protocols/')


def checked(reader, ref):
    require(reader.digest(ref['record']) == ref['sha256'], 'Retained input identity changed')
    value = reader.json(ref['record'])
    require(value['success'] is True, 'Unaccepted retained input')
    return value


def fingerprints(acquired):
    """Keep incomplete provenance visible; never infer an absent source hash."""
    result = {}
    build = acquired.get('player_build', {})
    for values in (acquired.get('sources', {}), acquired.get('compiler_sources', {}),
                   build.get('compiler_sources', {})):
        require(isinstance(values, dict), 'Unrecognized compiler/source schema')
        for name, value in values.items():
            if isinstance(value, dict):
                value = value.get('sha256')
            require(isinstance(value, str), 'Source fingerprint is not a hash')
            require(name not in result or result[name] == value, 'Conflicting source fingerprints')
            result[name] = value
    return result


def native_phase(root, reader, acquired, current_binary):
    """Full native file hashes remain dependencies, including inactive branches."""
    root = Path(root)
    values = fingerprints(acquired)
    native = {n: h for n, h in values.items() if n.startswith(NATIVE_ROOTS)}
    changed = {n: dict(recorded=h, current=reader.digest(n) if (root/n).is_file() else None)
               for n, h in native.items() if not (root/n).is_file() or reader.digest(n) != h}
    missing = sorted(native_runtime_source_paths(root) - values.keys())
    binding = acquired.get('player_binary', acquired.get('player_build', {}).get('binary', {}))
    require(isinstance(binding, dict), 'Malformed original player binary binding')
    current = bool(native) and not changed and not missing and binding.get('sha256') == current_binary
    return dict(recorded_native_files=len(native), changed_native_sources=changed,
                missing_runtime_sources=missing, recorded_player_binary=binding,
                exact_current_binary=binding.get('sha256') == current_binary,
                current_native_source_candidate=current,
                current_application_acceptance_established=False)


def parents(reader, register):
    evidence = {v['id']: v for v in register['evidence']}
    result = {}
    for eid in PARENTS:
        entry = evidence[eid]
        ref = dict(record=entry['record'], sha256=entry['sha256'])
        value = checked(reader, ref)
        proof = checked(reader, value['proof']) if 'proof' in value else value
        result[eid] = dict(reference=ref, admission=value, proof=proof)
    return result


def inventory(root, reader):
    register = reader.json('research/runtime/coverage/register.json')
    accepted = parents(reader, register)
    build_name = 'research/runtime/whole-player-validation-results-20261110/build-02.json'
    build = reader.json(build_name)
    require(build['success'] is True, 'Missing accepted current build')
    current_binary = build['player']['sha256']
    current_file = Path(root)/'working/build/qt-shell/mnm-qt-shell'
    with current_file.open('rb') as executable:
        require(hashlib.file_digest(executable, 'sha256').hexdigest() == current_binary,
                'Current player differs from retained tested build')
    ph = accepted['PH09.pixels-history01']['proof']
    references = {}

    def add(ref, role, pair=None):
        item = references.setdefault(ref['record'], dict(acquisition=ref, roles=[], pairs=[]))
        require(item['acquisition'] == ref, 'Conflicting acquisition identity')
        if role not in item['roles']:
            item['roles'].append(role)
        if pair and pair not in item['pairs']:
            item['pairs'].append(pair)

    for case in ph['cases'] + ph['additional_presented_cases']:
        add(case['pair']['acquisition'], 'historical-complete-World-output', case['pair'])
    for case in ph['component_cases']:
        add(case['pair']['acquisition'], 'historical-operation-specific-selected-plane', case['pair'])
    for case in ph['semantic_primary_cases']:
        add(case['execution'], 'historical-semantic-menu-or-movie')
    for case in accepted['WP10.cohort03']['proof']['executions']:
        add(case['pair']['acquisition'], 'current-WP10-finite-session', case['pair'])
    faults = accepted['FL09.fault-lifetime01']['proof']['cases']
    for case in faults:
        pair = case.get('pair')
        require(pair and set(pair) >= {'acquisition', 'comparison'}, 'Unrecognized accepted fault pair')
        add(pair['acquisition'], 'historical-failed-and-fresh-epochs', pair)

    cases = []
    for item in references.values():
        acquired = checked(reader, item['acquisition'])
        cfg = acquired['configuration']
        software = cfg.get('software_flag', cfg.get('software'))
        require(type(software) is bool, 'Untyped profile flag')
        item.update(kind=acquired['kind'], profile='player-software' if software else 'player-default',
                    identity=acquired.get('session_identity', acquired.get('session')),
                    phase=native_phase(root, reader, acquired, current_binary))
        cases.append(item)
    closure = reader.json('docs/native-rendering-closure.json')
    require(len(closure['journeys']) == 14 and len(closure['operations']) == 21, 'Acceptance denominator changed')
    return dict(schema=1, kind='broader_player_acceptance_source_inventory', success=True,
                planning_only=True, original_game_runs=0, current_whole_player_proved=False,
                supported_player_accepted=False, original_recovery_complete=False,
                parents={eid: value['reference'] for eid, value in accepted.items()},
                current_build=dict(record=build_name, sha256=reader.digest(build_name)),
                current_player_binary_sha256=current_binary,
                journeys=[dict(id=j['id'], requirements=j['requirements']) for j in closure['journeys']],
                operation_ids=[o['id'] for o in closure['operations']],
                cases=cases, historical_fault_pairs=len(faults),
                scope='Planning only. Full native/compiler/input provenance and actual public execution remain required. Existing host joins preserve historical execution phases. Current-source candidates do not establish semantic adequacy, final player acceptance or complete original recovery.')

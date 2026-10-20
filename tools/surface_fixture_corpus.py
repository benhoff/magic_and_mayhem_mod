"""Strict admission and native replay of independently retained surface cohorts.

Adapters keep each historical wire format and oracle separate. A catalog cannot
choose a program, add arguments, supply expected pixels as native inputs, or
turn a represented operation family into full surface acceptance.
"""
from contextlib import nullcontext
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from terrain_validation_display import private_display
from coverage_claims import behavior_contract, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
DEFAULT = ROOT / 'tests/fixtures/surfaces/operation-corpus.json'
PREFIX = 'tests/fixtures/surfaces/'
RESEARCH = 'research/runtime/'
MAX_JSON = 16 * 1024 * 1024


def pair(stem):
    return {PREFIX + stem + '-corpus.json', PREFIX + stem + '.json.gz'}


# These are admission contracts, not catalog-controlled command templates.
ADAPTERS = {
    'backend': {
        'checker': 'check-native-surface-backend.py',
        'binary': 'render-surface-backend-test',
        'args': ['--combined'],
        'matrix_ids': {'SURF.copy.opaque', 'SURF.format.rgb565', 'SURF.fill',
                       'SURF.key.exact_source', 'SURF.copy.self_overlap',
                       'SURF.clip.rectangles', 'SURF.clip.regions'},
        'oracle': 'isolated_original_wrapper_and_standalone_driver',
        'artifacts': (pair('original-rgb565-fills') | pair('original-rgb565-keyed') |
                      pair('original-rgb565-overlap') | pair('original-rgb565-keyed-overlap') |
                      pair('original-sentinel') | {
                          PREFIX + 'driver-clipping-surface2-corpus.json',
                          PREFIX + 'driver-clipping-surface2-original.json.gz',
                          PREFIX + 'surface-dc-reuse.json.gz',
                          RESEARCH + 'surface-dc-reuse-capture-20261006.json',
                          PREFIX + 'surface-access.json.gz',
                          RESEARCH + 'surface-access-capture-20261006.json',
                          PREFIX + 'surface-borrowed-draw.json.gz',
                          RESEARCH + 'surface-borrowed-draw-capture-20261006.json'}),
        'metrics': {'draws': 5114, 'hresult_checks': 5114, 'pixel_checks': 538272,
                    'reuse_cases': 192, 'reuse_phases': 576, 'access_cases': 96},
    },
    'formats': {
        'checker': 'check-native-surface-formats.py',
        'binary': 'render-surface-formats-test',
        'args': [],
        'matrix_ids': {'SURF.format.other', 'SURF.copy.opaque', 'SURF.fill',
                       'SURF.key.exact_source', 'SURF.copy.self_overlap',
                       'SURF.clip.rectangles'},
        'oracle': 'standalone_directdraw2_surface2_driver',
        'artifacts': {RESEARCH + 'surface-formats-final-capture-20261006.json',
                      PREFIX + 'surface-formats-final.json.gz'},
        'metrics': {'driver_draws': 1400, 'signed_row_draws': 2800,
                    'admitted_draws': 1400, 'rejected_layout_cases': 2100},
    },
    'ownership': {
        'checker': 'check-native-surface-ownership.py',
        'binary': 'render-surface-ownership-test',
        'args': [],
        'matrix_ids': {'SURF.palette', 'SURF.flip', 'SURF.restore', 'SURF.lifetime_updates'},
        'oracle': 'standalone_driver_and_recovered_original_scheduler',
        'artifacts': (pair('original-retry') | {
            RESEARCH + 'surface-ownership-final-capture-20261006.json',
            PREFIX + 'surface-ownership-final.json.gz',
            RESEARCH + 'surface-palette-restore-capture-20261006.json',
            PREFIX + 'surface-palette-restore.json.gz'}),
        'metrics': {'flip_results': 11, 'retry_traces': 4480,
                    'bounded_retry_stops': 1536, 'owned_recovery_cases': 16},
    },
    'clipping': {
        'checker': 'check-native-clipping-stream.py',
        'binary': 'render-clipping-stream-test',
        'args': [],
        'matrix_ids': {'SURF.clip.rectangles', 'SURF.clip.regions'},
        'oracle': 'standalone_surface2_driver',
        'artifacts': {PREFIX + 'driver-clipping-surface2-corpus.json',
                      PREFIX + 'driver-clipping-surface2-original.json.gz'},
        'metrics': {'driver_cases': 408, 'hresult_checks': 408, 'pixel_checks': 816},
    },
}


def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def source_path(name):
    require(isinstance(name, str), 'Source path must be a string')
    path = Path(name)
    target = ROOT / path
    require(not path.is_absolute() and path.as_posix() == name and '..' not in path.parts
            and path.parts and path.parts[0] in {'tools', 'tests', 'research', 'docs', 'renderer',
                'reconstruction', 'runtime', 'protocols', 'apps', 'compat', 'assets', 'audio', 'game'}
            and target.resolve().is_relative_to(ROOT), 'Unsafe corpus source path: ' + name)
    require(target.is_file() and not target.is_symlink(), 'Missing or linked corpus source: ' + name)
    return target


def read_json(path):
    with path.open('rb') as stream:
        raw = stream.read(MAX_JSON + 1)
    require(len(raw) <= MAX_JSON, 'Corpus/report exceeds JSON size limit')
    return json.loads(raw)


def stable_sources(sources):
    try:
        return all(sha(source_path(n)) == h for n, h in sources.items())
    except (OSError, ValueError):
        return False


def validate_catalog(catalog):
    require(catalog.get('schema') == 2 and isinstance(catalog.get('cohorts'), list)
            and catalog['cohorts'], 'Unsupported or empty operation corpus')
    require(isinstance(catalog.get('scope'), str) and catalog['scope'].strip(), 'Corpus scope required')
    require(isinstance(catalog.get('remaining_contracts'), list) and catalog['remaining_contracts']
            and all(isinstance(s, str) and s.strip() for s in catalog['remaining_contracts']),
            'Wider acceptance boundaries required')
    seen, pinned = set(), {}
    for row in catalog['cohorts']:
        require(isinstance(row, dict) and isinstance(row.get('id'), str), 'Invalid cohort')
        key = row['id']
        require(key in ADAPTERS and key not in seen, 'Unknown or duplicate cohort')
        seen.add(key)
        adapter = ADAPTERS[key]
        require(set(row) == {'id', 'matrix_ids', 'oracle', 'scope', 'artifacts'},
                'Unsupported cohort fields')
        ids = row['matrix_ids']
        require(isinstance(ids, list) and all(isinstance(i, str) for i in ids)
                and len(ids) == len(set(ids)) and set(ids) == adapter['matrix_ids'],
                'Cohort cannot establish the claimed operation semantics')
        require(row['oracle'] == adapter['oracle'], 'Cohort oracle relabeling')
        require(isinstance(row['scope'], str) and row['scope'].strip(), 'Cohort scope required')
        artifacts = row['artifacts']
        require(isinstance(artifacts, dict) and set(artifacts) == adapter['artifacts'],
                'Missing or substituted cohort artifacts')
        for name, digest in artifacts.items():
            require(isinstance(digest, str) and len(digest) == 64
                    and all(c in '0123456789abcdef' for c in digest), 'Invalid artifact hash')
            require(name not in pinned or pinned[name] == digest, 'Conflicting shared artifact hashes')
            require(sha(source_path(name)) == digest, 'Cohort artifact hash mismatch: ' + name)
            pinned[name] = digest
    return pinned


def admit_result(key, result, pinned, binary_hashes):
    """Require actual bounded comparisons, not merely a successful child exit."""
    adapter = ADAPTERS[key]
    require(isinstance(result, dict) and result.get('schema') == 1
            and result.get('success') is True, 'Missing successful cohort report')
    for name, expected in adapter['metrics'].items():
        require(type(result.get(name)) is int and result[name] == expected,
                'Incomplete cohort comparison: ' + name)
    if key == 'backend':
        require(result.get('combined') is True, 'Combined overlap/borrowed comparisons required')
    if key == 'ownership':
        require(result.get('ownership_driver_comparison') is True,
                'Guard-only ownership cannot establish driver comparison')
    sources = result.get('sources')
    require(isinstance(sources, dict) and adapter['artifacts'] <= sources.keys(),
            'Cohort report omits admitted artifact fingerprints')
    require('tools/' + adapter['checker'] in sources, 'Cohort checker identity missing')
    for name, digest in sources.items():
        require(sha(source_path(name)) == digest, 'Changed cohort source: ' + name)
        if name in pinned:
            require(digest == pinned[name], 'Cohort consumed a different artifact')
    if key == 'clipping':
        require(result.get('binaries') == {n: binary_hashes[n] for n in
                ('mnm-render-commands', 'render-clipping-stream-test')},
                'Clipping executable identity differs')
    else:
        require(result.get('binary_sha256') == binary_hashes[adapter['binary']]
                and result.get('library_sha256') == binary_hashes['libmnm-renderer.a'],
                'Native executable/library identity differs')
    return sources


def summarize(catalog, rows, required):
    expected = {row['id'] for row in catalog['cohorts']}
    require(len(rows) == len(expected) and {row['id'] for row in rows} == expected,
            'Missing or duplicate cohort result')
    passed = {row['id'] for row in rows if row.get('success') is True and row.get('returncode') == 0}
    covered = set().union(*(ADAPTERS[key]['matrix_ids'] for key in passed)) if passed else set()
    return dict(success=passed == expected, covered_matrix_ids=sorted(covered),
                missing_required_matrix_ids=sorted(required - covered),
                required_fixture_cells_complete=passed == expected and required <= covered,
                # Operation representation does not close every acceptance case in a row.
                milestone_complete=False, full_surface_acceptance_complete=False,
                remaining_contracts=catalog['remaining_contracts'])


def check(corpus_path, build, headless=False):
    catalog = read_json(corpus_path)
    pinned = validate_catalog(catalog)
    require(build is not None, 'Operation corpus requires --build with native replay executables')
    build = build.resolve()
    names = {'libmnm-renderer.a'} | {ADAPTERS[r['id']]['binary'] for r in catalog['cohorts']}
    if any(r['id'] == 'clipping' for r in catalog['cohorts']):
        names.add('mnm-render-commands')
    binaries = {name: sha(build / name) for name in sorted(names)}
    matrix = source_path('research/runtime/native-surface-operation-matrix.json')
    required = {r['id'] for r in read_json(matrix)['operations'] if r['requirement'] == 'required'}
    sources = dict(pinned)
    # The reviewed central links declare the transitive replay source closure.
    # Pin it before execution as well as checking every adapter's actual report.
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'NR.surface-fixture-corpus')
    for role in ('implementation', 'tests', 'validation_dependencies'):
        for name in behavior.get(role, []):
            sources[name] = sha(source_path(name))
    for name in ('tools/surface_fixture_corpus.py', 'tools/check-surface-fixtures.py',
                 'tools/coverage_claims.py',
                 'tools/terrain_validation_display.py', 'tests/test-surface-operation-corpus.py',
                 'research/runtime/native-surface-operation-matrix.json'):
        sources[name] = sha(source_path(name))
    # Catalogs are portable repository artifacts; no absolute historical paths are followed.
    catalog_name = corpus_path.resolve().relative_to(ROOT).as_posix()
    sources[catalog_name] = sha(source_path(catalog_name))
    parent = ROOT / 'working/tests/surface-operation-corpus'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    (run / '.retain').write_text('Operation corpus native comparisons, including failures.\n')
    rows = []
    scenario = next((s for s in register['scenarios']
                     if s['id'] == 'SURFCORPUS.operations.headless'), None)
    claim = dict(behavior=behavior['id'], contract_sha256=behavior_contract(
        behavior, {b['id']: b for b in register['builds']}))
    if headless and scenario:
        claim['scenarios'] = {scenario['id']: scenario_contract(scenario)}
        for name in scenario['tests']:
            sources[name] = sha(source_path(name))
    report = dict(schema=2, backend='opengl', scope=catalog['scope'], cases=rows,
                  sources=sources, claims=[claim], binaries=binaries, original_game_executed=False,
                  original_artifacts_consumed=False, original_pixels_used_as_native_inputs=False,
                  artifacts=run.relative_to(ROOT).as_posix())
    display = private_display(run / 'xvfb.log') if headless else nullcontext(dict(os.environ))
    try:
        with display as env:
            env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
            # Disable Python -O so assertions in the historical adapters remain enforced.
            env.pop('PYTHONOPTIMIZE', None)
            for cohort in catalog['cohorts']:
                key = cohort['id']
                adapter = ADAPTERS[key]
                output = run / (key + '.json')
                command = [sys.executable, '-B', str(ROOT / 'tools' / adapter['checker']),
                           '--build', str(build), '--report', str(output), *adapter['args']]
                if key != 'clipping':
                    command += ['--executable', str(build / adapter['binary'])]
                row = dict(id=key, oracle=cohort['oracle'], scope=cohort['scope'],
                           matrix_ids=cohort['matrix_ids'], success=False, returncode=None)
                rows.append(row)
                try:
                    with (run / (key + '.log')).open('x') as log:
                        child = subprocess.run(command, cwd=ROOT, env=env, stdout=log,
                                               stderr=subprocess.STDOUT, timeout=600)
                    row['returncode'] = child.returncode
                    require(child.returncode == 0, 'Native cohort failed: ' + key)
                    child_report = read_json(output)
                    fingerprints = admit_result(key, child_report, pinned, binaries)
                    for name, digest in fingerprints.items():
                        require(name not in sources or sources[name] == digest,
                                'Sources changed between cohorts: ' + name)
                        sources[name] = digest
                    row.update(success=True, metrics={n: child_report[n] for n in adapter['metrics']})
                except (ValueError, OSError, KeyError, TypeError, subprocess.SubprocessError) as error:
                    row['error'] = str(error)
                if output.exists():
                    row.update(report=output.relative_to(ROOT).as_posix(), report_sha256=sha(output))
    except (ValueError, OSError, RuntimeError) as error:
        report['error'] = str(error)
    finally:
        present = {r['id'] for r in rows}
        for cohort in catalog['cohorts']:
            if cohort['id'] not in present:
                rows.append(dict(id=cohort['id'], success=False, returncode=None,
                                 error='Cohort not executed'))
        report.update(summarize(catalog, rows, required))
        report['success'] = report['success'] and 'error' not in report
        report['sources_stable'] = stable_sources(sources)
        report['binaries_stable'] = all((build / n).is_file() and sha(build / n) == h
                                        for n, h in binaries.items())
        if not report['sources_stable'] or not report['binaries_stable']:
            report.update(success=False, covered_matrix_ids=[],
                          missing_required_matrix_ids=sorted(required),
                          required_fixture_cells_complete=False)
        (run / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report

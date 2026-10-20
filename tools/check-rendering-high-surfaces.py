#!/usr/bin/env python3
"""Validate retained surface operation cohorts without widening their provenance."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources
from terrain_validation_display import private_display

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / 'tests/fixtures/surfaces/high-validation-suite.json'
COMPONENTS = {
    'copies': ('check-surface-fixtures.py', {'SURF.copy.opaque', 'SURF.format.rgb565'}),
    'backend': ('check-native-surface-backend.py', {
        'SURF.copy.opaque', 'SURF.format.rgb565', 'SURF.key.exact_source',
        'SURF.clip.rectangles', 'SURF.clip.regions', 'SURF.fill', 'SURF.copy.self_overlap'}),
    'formats': ('check-native-surface-formats.py', {
        'SURF.format.other', 'SURF.copy.opaque', 'SURF.key.exact_source',
        'SURF.fill', 'SURF.clip.rectangles', 'SURF.copy.self_overlap'}),
    'ownership': ('check-native-surface-ownership.py', {
        'SURF.palette', 'SURF.flip', 'SURF.restore', 'SURF.lifetime_updates'}),
    'clipping': ('check-native-clipping-stream.py', {'SURF.clip.rectangles', 'SURF.clip.regions'}),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_catalog(catalog, required):
    if catalog.get('schema') != 1 or not catalog.get('remaining_contracts'):
        raise ValueError('Explicit schema and wider remaining contracts required')
    seen = set()
    for row in catalog['components']:
        key = row['id']
        if key not in COMPONENTS or key in seen:
            raise ValueError('Unknown or duplicate validation component')
        seen.add(key)
        if (len(row['matrix_ids']) != len(set(row['matrix_ids'])) or
                set(row['matrix_ids']) != COMPONENTS[key][1]):
            raise ValueError('Component cannot establish the requested operation cells')
        if not row.get('oracle') or not row.get('scope'):
            raise ValueError('Independent oracle and bounded scope required')
    if seen != COMPONENTS.keys():
        raise ValueError('Missing validation component')
    if set().union(*(set(r['matrix_ids']) for r in catalog['components'])) != required:
        raise ValueError('Surface matrix changed; review the operation mapping')


def summarize(catalog, rows, required):
    validate_catalog(catalog, required)
    by_id = {r['id']: r for r in rows}
    if len(by_id) != len(rows) or set(by_id) != COMPONENTS.keys():
        raise ValueError('Missing or duplicate component result')
    passed = {key for key, row in by_id.items()
              if row.get('returncode') == 0 and row.get('success') is True}
    covered = set().union(*(COMPONENTS[key][1] for key in passed)) if passed else set()
    success = passed == COMPONENTS.keys()
    return dict(success=success, validated_fixture_cells=sorted(covered),
                missing_required_fixture_cells=sorted(required-covered),
                required_fixture_cells_complete=success and required <= covered,
                full_surface_acceptance_complete=False,
                remaining_contracts=catalog['remaining_contracts'])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--catalog', type=Path, default=CATALOG)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--build', type=Path, required=True, help='Renderer build directory')
    p.add_argument('--output', type=Path, required=True, help='New retained output directory')
    a = p.parse_args()
    out = a.output.resolve()
    if not out.is_relative_to(ROOT/'working'):
        p.error('Output must be task-owned working storage')
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('Retained independent surface cohorts, including failures.\n')
    catalog = json.loads(a.catalog.read_text())
    matrix = json.loads((ROOT/'research/runtime/native-surface-operation-matrix.json').read_text())
    required = {r['id'] for r in matrix['operations'] if r['requirement'] == 'required'}
    validate_catalog(catalog, required)
    declaration = json.loads(a.claims.read_text())
    reg = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = {b['id']: b for b in reg['behaviors']}
    builds = {b['id']: b for b in reg['builds']}
    for claim in declaration['claims']:
        bid = claim['behavior']
        if bid not in {'NR.surface-fixture-corpus', 'NR.surfaces'}:
            raise ValueError('Unexecuted behavior claim in surface suite')
        if claim['contract_sha256'] != behavior_contract(behaviors[bid], builds):
            raise ValueError('Changed prospective behavior scope')
        if not required_sources(behaviors[bid]) <= declaration['sources'].keys():
            raise ValueError('Incomplete declared source closure')
    if not declaration['claims']:
        raise ValueError('Prospective behavior claims required')
    sources = declaration['sources']
    for name, digest in sources.items():
        target = (ROOT/name).resolve()
        if (Path(name).is_absolute() or not target.is_relative_to(ROOT) or
                target.relative_to(ROOT).parts[0] in {'original', 'working', '.git'} or
                sha(target) != digest):
            raise ValueError('Unsafe or changed declared source: '+name)
    binaries = {n: sha(a.build/n) for n in ('mnm-render-replay', 'render-surface-backend-test',
                'render-surface-formats-test', 'render-surface-ownership-test',
                'render-clipping-stream-test')}
    report = dict(success=False, claims=declaration['claims'], sources=sources,
                  catalog_sha256=sha(a.catalog), binaries=binaries, components=[],
                  original_game_executed=False, original_artifacts_consumed=False,
                  scope='Fresh replay of separately retained game/isolated-wrapper/standalone-driver '
                        'surface cohorts. Native import/presentation policies stay separate. '
                        'Operation cell representation is not complete original-game or live acceptance.')
    try:
        with private_display(out/'xvfb.log') as env:
            env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
            for row in catalog['components']:
                key = row['id']
                result = out/(key+'.json')
                command = [sys.executable, '-B', str(ROOT/'tools'/COMPONENTS[key][0]),
                           '--report', str(result)]
                if key == 'copies':
                    command += ['--backend', 'opengl', '--gl-executable', str(a.build/'mnm-render-replay')]
                else:
                    command += ['--build', str(a.build)]
                    if key == 'backend':
                        command += ['--combined', '--executable', str(a.build/'render-surface-backend-test')]
                    elif key in {'formats', 'ownership'}:
                        command += ['--executable', str(a.build/('render-surface-'+key+'-test'))]
                with (out/(key+'.log')).open('x') as log:
                    child = subprocess.run(command, cwd=ROOT, env=env, stdout=log,
                                           stderr=subprocess.STDOUT, timeout=600)
                data = json.loads(result.read_text()) if result.exists() else {}
                report['components'].append(dict(id=key, command=command,
                    returncode=child.returncode, success=data.get('success') is True,
                    report=str(result.relative_to(ROOT)),
                    report_sha256=sha(result) if result.exists() else None,
                    oracle=row['oracle'], scope=row['scope']))
        report.update(summarize(catalog, report['components'], required))
    except Exception as error:
        report.update(success=False, error=str(error))
    finally:
        report['sources_stable'] = all(sha(ROOT/n) == h for n, h in sources.items())
        report['binaries_stable'] = all(sha(a.build/n) == h for n, h in binaries.items())
        report['success'] &= report['sources_stable'] and report['binaries_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({k: report.get(k) for k in ('success', 'sources_stable',
                     'required_fixture_cells_complete', 'missing_required_fixture_cells', 'error')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())

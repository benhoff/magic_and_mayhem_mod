#!/usr/bin/env python3
"""Validate original overwrite dialogs and native deletion on a private new save."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import secrets
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PROSPECTIVE = {}
CLAIMS = []


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def validate(root, args):
    report = json.loads((root / 'report.json').read_text())
    required = ['success', 'overwrite_no_preserved', 'delete_no_preserved',
                'delete_rows_refreshed', 'load_excludes_deleted']
    if not all(report.get(key) is True for key in required):
        raise ValueError('Incomplete confirmation/file/list evidence')
    if not report['before_sha256'] or not report['after_sha256']:
        raise ValueError('Missing original save fingerprints')
    if (report['before_sha256'] == report['after_sha256'] and
            report['after_mtime_ms'] <= report['before_mtime_ms']):
        raise ValueError('Overwrite Yes has no observed replacement write')
    lines = (root / 'shell.log').read_text().splitlines()
    if not any(line.startswith('Menu launcher exited with status 0.') for line in lines):
        raise ValueError('Missing normal Quit')
    experiment = Path(next(line[20:] for line in lines if line.startswith('Evidence directory: ')))
    metadata = json.loads((experiment / 'manifest.json').read_text())
    decoded = module('confirmation_decoder', 'tools/test-menu-observer.py').decode((experiment / 'events.bin').read_bytes())
    actions = [(row['menu_id'], row['argument']) for row in decoded if row['event'] == 3]
    expected = [(3, 0), (18, 0), (17, 1), (13, 0), (17, 1), (13, 0), (13, 0),
                (17, 1), (13, 2), (13, 1), (17, 0), (12, 1), (17, 3), (6, 21), (3, 4)]
    if actions != expected:
        raise ValueError('Original callback sequence mismatch: ' + str(actions))
    if len({row['thread'] for row in decoded if row['event'] in (2, 3)}) != 1:
        raise ValueError('Original engine thread changed')
    name = report['saved_name'] + '.sav'
    result = json.loads((experiment / 'save-store-result.json').read_text())
    if not result.get('merged') or result['changed'] or result['files'] != metadata['save_store']['baseline']:
        raise ValueError('Confirmation journey changed pre-existing owned saves')
    if (experiment / 'game/Save' / name).exists() or (Path(metadata['save_store']['store']) / name).exists():
        raise ValueError('Deleted test save survived or entered owned store')
    current = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in PROSPECTIVE}
    if current != PROSPECTIVE:
        raise ValueError('Sources changed during test')
    built = json.loads((experiment / 'bridge-build.json').read_text())
    for p, digest in current.items():
        if (p.startswith('runtime/menu/') or (p.startswith('protocols/') and Path(p).name.startswith('menu_v'))) and built['sources'].get(Path(p).name) != digest:
            raise ValueError('Different compiled menu source: ' + p)
    if metadata.get('native_render'):
        raise ValueError('This scenario requires the Wine-window embedding route')
    report.update(sources=current, claims=CLAIMS, source_stable=True, source_sha256=metadata['source_sha256'],
                  original_callbacks=actions, normal_quit=True, owned_store_unchanged=True,
                  deleted_save_absent=True, native_render=False, experiment=str(experiment))
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Live overwrite No/Yes and delete No/Yes passed; existing owned saves unchanged')


def main():
    global PROSPECTIVE, CLAIMS
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behaviors = [b for b in register['behaviors'] if b['id'] in
                 ('HOST.native-storage-confirmations', 'NP.storage-menu-ownership')]
    scenario = next(s for s in register['scenarios'] if s['id'] == 'UI.storage-confirmations')
    claims = module('confirmation_claims', 'tools/coverage_claims.py')
    CLAIMS = [dict(behavior=b['id'], contract_sha256=claims.behavior_contract(b, {v['id']: v for v in register['builds']}),
                  scenarios={scenario['id']: claims.scenario_contract(scenario)}) for b in behaviors]
    paths = set().union(*(set(b['implementation']) | set(b['tests']) | set(b.get('validation_dependencies', [])) for b in behaviors))
    paths.update(module('confirmation_compiler', 'tools/test-native-campaign.py').linked_compiler_sources(ROOT / 'working/build/qt-shell', ROOT))
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT / 'runtime/menu').iterdir() if p.is_file())
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT / 'protocols/include/mnm').glob('menu_v*.h'))
    PROSPECTIVE = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sorted(paths)}
    parent = ROOT / 'working/tests/storage-confirmations-prospective'; parent.mkdir(parents=True, exist_ok=True)
    snapshot = Path(tempfile.mkdtemp(prefix='run-', dir=parent)) / 'prospective.json'
    snapshot.write_text(json.dumps(dict(sources=PROSPECTIVE, claims=CLAIMS), indent=2) + '\n')
    print(snapshot, flush=True)
    os.environ.update(MNM_CAMPAIGN_SMOKE_TEST='1', MNM_CAMPAIGN_STORAGE='1', MNM_CAMPAIGN_STORAGE_BOUNDARIES='1',
                      MNM_CAMPAIGN_STORAGE_NAME='native-confirm-' + secrets.token_hex(4), LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4')
    harness = module('confirmation_harness', 'tools/test-live-menus.py'); harness.validate = validate
    sys.argv = [sys.argv[0], '--exit-from', 'main', *sys.argv[1:]]
    harness.main()


if __name__ == '__main__':
    main()

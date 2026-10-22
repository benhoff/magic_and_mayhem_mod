#!/usr/bin/env python3
"""Replay an explicit phase plan and retain every full-plane SHA without duplicate dumps."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from coverage_evidence import read_bytes as evidence_bytes,read_json,evidence_digest,write_report
from drawing_api_session import sha
from drawing_copy_phase import select, transpose
from drawing_scene_corpus import asset_name

ROOT = Path(__file__).resolve().parents[1]


def compare_checkpoints(expected, actual):
    """Reject missing identities; retain every failing hash with unknown pixel count."""
    def keyed(rows):
        for r in rows:
            if not isinstance(r,dict) or any(type(r.get(k)) is not int or not 1<=r[k]<=(2048 if k in ('width','height') else 0xffffffff) for k in ('ordinal','sequence','canvas','width','height')):
                raise ValueError('Typed checkpoint identity/extent required')
        keyed_rows = {r['ordinal']: r for r in rows}
        if len(keyed_rows) != len(rows):
            raise ValueError('Duplicate checkpoint identity')
        return keyed_rows
    originals, predictions = keyed(expected), keyed(actual)
    if originals.keys() != predictions.keys():
        raise ValueError('Checkpoint denominator changed')
    compared = []
    for ordinal, old in originals.items():
        new = predictions[ordinal]
        if any(old[k] != new[k] for k in ('sequence', 'canvas', 'width', 'height')):
            raise ValueError('Checkpoint sequence/canvas/extent changed')
        for digest in (old['original_sha256'], new['sha256']):
            if not isinstance(digest,str) or len(digest) != 64 or any(c not in '0123456789abcdef' for c in digest):
                raise ValueError('Invalid full-plane SHA256')
        matched = old['original_sha256'] == new['sha256']
        compared.append(dict(ordinal=ordinal, **{k: old[k] for k in ('sequence', 'canvas', 'width', 'height')},
                             original_sha256=old['original_sha256'], native_sha256=new['sha256'],
                             full_plane_hash_matches=matched, different_pixels=0 if matched else None))
    return compared


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('reference', 'template', 'plan', 'executable', 'claims', 'report'):
        p.add_argument('--' + name, type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw, template_raw, plan_raw = (evidence_bytes(p) for p in (a.reference,a.template,a.plan))
    ref, template, plan = json.loads(raw), json.loads(template_raw), json.loads(plan_raw)
    claims = read_json(a.claims)
    if (not ref['success'] or not ref['normal_original_quit_verified'] or template['native_exit'] or
            template['reference']['sha256'] != sha(raw) or template['replay_order_changed'] or
            template['original_pixels_used_as_native_inputs'] or not plan['success'] or
            plan['reference']['sha256'] != sha(raw) or plan['original_pixels_used_as_native_inputs']):
        raise ValueError('Completed default template and exact metadata-only plan required')
    sources = claims['sources']
    if any(evidence_digest(ROOT / n) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    binding_path = Path(plan['bindings']['path'])
    binding_raw = evidence_bytes(binding_path if binding_path.is_absolute() else ROOT / binding_path)
    if sha(binding_raw) != plan['bindings']['sha256']:
        raise ValueError('Plan binding changed')
    bindings = json.loads(binding_raw)
    if not bindings['success'] or bindings['reference']['sha256'] != sha(raw):
        raise ValueError('Plan reference binding changed')
    executable_hash = sha(a.executable.read_bytes())
    if executable_hash != template['executable_sha256']:
        raise ValueError('Native executable differs from pinned default template')
    parent = ROOT / 'working/tests/drawing-copy-phase-hashes'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    inputs = run / 'inputs'
    (inputs / 'assets').mkdir(parents=True)
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        capture = Path(ref['experiment']) / 'capture'
        wire = (capture / 'canvas-producers.bin').read_bytes()
        if sha(wire) != plan['producer_sha256'] or sha(wire) != ref['producer_observation']['producer_journal_sha256']:
            raise ValueError('Original producer journal changed')
        spec = importlib.util.spec_from_file_location('hash_phase_producers', ROOT / 'tools/inspect-canvas-producers.py')
        m = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(m)
        _, rows = m.decode(wire, complete=True)
        choices = select(rows, ref['copy_order_observation'], bindings)
        if choices != {k: plan[k] for k in ('selected', 'refused')}:
            raise ValueError('Current policy differs from plan')
        source = ROOT / template['artifacts'] / 'inputs'
        native_source = (source / 'producers.bin').read_bytes()
        if sha(native_source) != template['native_input_sha256']:
            raise ValueError('Default parameter input changed')
        native_source = transpose(native_source, choices['selected'])
        (inputs / 'producers.bin').write_bytes(native_source)
        for name, digest in template['source_assets'].items():
            relative = asset_name(name).casefold()
            src, dst = source / 'assets' / relative, inputs / 'assets' / relative
            if sha(src.read_bytes()) != digest:
                raise ValueError('Encoded source asset changed')
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, dst)
        checkpoints = [r for r, _ in rows if r[2] == 10]
        expected = template['comparisons']
        if len(checkpoints) != len(expected) or len(expected) != ref['source_completion']['oracles']:
            raise ValueError('Original checkpoint denominator changed')
        originals = {r['ordinal']: r for r in expected}
        if len(originals) != len(expected):
            raise ValueError('Duplicate original checkpoint')
        for r in checkpoints:
            old = originals[r[14]]
            if ((r[1], r[3], r[5], r[6]) != tuple(old[k] for k in ('sequence', 'canvas', 'width', 'height')) or
                    sha((capture / f'producer-oracle-{r[14]:04d}.565').read_bytes()) != old['original_sha256']):
                raise ValueError('External original checkpoint identity/hash changed')
        final = json.loads((source / 'scene.json').read_bytes())['boundary']
        (inputs / 'scene.json').write_text(json.dumps(dict(schema=1, boundary=final,
                                          assets=template['source_assets'], retain_checkpoints=[]), indent=2) + '\n')
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        result = subprocess.run(['xvfb-run', '-a', str(a.executable), str(inputs / 'producers.bin'),
                                 str(inputs / 'assets'), str(inputs / 'scene.json'), str(run / 'native')],
                                env=env, capture_output=True, timeout=600)
        (run / 'native.log').write_bytes(result.stdout + result.stderr)
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    native_path = run / 'native/scene-report.json'
    native = json.loads(native_path.read_bytes()) if native_path.exists() else None
    comparisons = []
    if result.returncode == 0:
        if (not native['success'] or native['boundary'] != final or native['remaining_surfaces'] or
                any(native[k] for k in ('original_pixels_used_as_native_inputs', 'original_oracles_read',
                                       'original_raster_results_used_as_native_inputs', 'live_replacement'))):
            raise ValueError('Native closure/input provenance changed')
        comparisons = compare_checkpoints(expected, native['checkpoints'])
    if sha(a.executable.read_bytes()) != executable_hash or any(evidence_digest(ROOT / n) != h for n, h in sources.items()):
        raise ValueError('Source/executable changed during execution')
    report = dict(schema=1, success=result.returncode == 0 and len(comparisons) == len(expected) and
                  all(c['full_plane_hash_matches'] for c in comparisons), sources=sources, claims=claims['claims'],
                  reference=dict(path=str(a.reference), sha256=sha(raw)),
                  template=dict(path=str(a.template), sha256=sha(template_raw)),
                  plan=dict(path=str(a.plan), sha256=sha(plan_raw)), native_exit=result.returncode, native=native,
                  comparisons=comparisons, selected=choices['selected'], refused=choices['refused'],
                  artifacts=str(run.relative_to(ROOT)), native_input_sha256=sha(native_source),
                  executable_sha256=executable_hash, original_pixels_used_as_native_inputs=False,
                  original_game_reexecuted=False, original_manifest_verified_before_after=True,
                  replay_order_changed=True, source_assets=template['source_assets'],
                  scope='Explicit experimental metadata-only COPY phase policy; compare every original checkpoint identity and freshly computed native full-plane SHA256. Original planes externally rehashed, never native inputs. No duplicate native checkpoint plane dumps; hash mismatches remain failures with pixel count unknown, never zero. All checkpoint denominators retained, no general ordering/ownership/live replacement or whole journey closure.')
    write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], compared=len(comparisons),
                         failed=sum(not c['full_plane_hash_matches'] for c in comparisons), native_exit=result.returncode)))


if __name__ == '__main__':
    main()

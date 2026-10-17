#!/usr/bin/env python3
"""Compare external auxiliary mapped-view witnesses to unchanged native buffers."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_aux_save_readback import analyze
from drawing_scene_corpus import asset_name

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ['reference', 'template', 'bindings', 'executable', 'claims', 'report']:
        p.add_argument('--' + name, type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw, template_raw, binding_raw = a.reference.read_bytes(), a.template.read_bytes(), a.bindings.read_bytes()
    ref, template, bindings = json.loads(raw), json.loads(template_raw), json.loads(binding_raw)
    if (not ref['success'] or not ref['normal_original_quit_verified'] or template['native_exit']
            or template['reference']['sha256'] != sha(raw) or bindings['reference']['sha256'] != sha(raw)
            or not bindings['success'] or template['original_pixels_used_as_native_inputs']):
        raise ValueError('Completed exact original/native/request provenance required')
    capture = Path(ref['experiment']) / 'capture'
    observed = analyze(capture)
    if observed != ref['auxiliary_save_readback_observation'] or not observed['original_driver_state_restored']:
        raise ValueError('Original auxiliary mapped-view evidence changed')
    spec = importlib.util.spec_from_file_location('aux_producers', ROOT / 'tools/inspect-canvas-producers.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    wire = (capture / 'canvas-producers.bin').read_bytes()
    if sha(wire) != ref['producer_observation']['producer_journal_sha256']:
        raise ValueError('Original producer stream changed')
    _, decoded = module.decode(wire)
    rows, active, queues, complete, in_world = {}, False, {}, 0, {}
    for r, payload in decoded:
        rows[r[1]] = r
        if r[2] == 11:
            active = True
        elif r[2] == 12:
            active, complete = False, r[14]
        queues[r[1]], in_world[r[1]] = complete, active
    by_api = {b['api_entry']: b for b in bindings['bindings']}
    policy = template.get('copy_phase_policy')
    moved = {v['save_sequence']: v['full_sequence'] for v in policy['selected']} if policy else {}
    images = {i['ordinal']: i for i in observed['images']}
    selected, excluded, frames = [], [], []
    for sample in observed['samples']:
        if not sample['oracle']:
            excluded.append(dict(api_entry=sample['api_entry'], reason='diagnostic-refused', detail=sample['reason']))
            continue
        b = by_api.get(sample['api_entry'])
        if b is None or b['owner'] != sample['owner']:
            raise ValueError('Auxiliary sample lacks exact owner/request binding')
        seq = b['producer_return_sequence']
        r = rows[seq]
        if r[2] != 6 or r[3:7] != (5, 1, 40, 40) or r[8:10] != (0, 0) or r[14] != 0xffffffff:
            raise ValueError('Auxiliary return geometry changed')
        before, after = sample['point_pair'][3], sample['after_point'][1]
        if not b['producer_entry_prefix'] <= before <= after < seq:
            raise ValueError('Auxiliary point/return temporal bounds changed')
        reason = 'inside-active-world' if in_world[seq] else None
        if any(q[3] == 5 and q[2] not in (3, 4, 10, 11, 12, 25, 27)
               for q, payload in decoded[before:seq - 1]):
            reason = 'other-observed-auxiliary-write-or-lifetime'
        if reason:
            excluded.append(dict(api_entry=sample['api_entry'], request=b['request'], sequence=seq,
                                 oracle=sample['oracle'], reason=reason))
            continue
        frame = dict(sequence=moved.get(seq, seq), queues=queues[seq], canvas=5, width=40, height=40)
        frames.append(frame)
        selected.append(dict(api_entry=sample['api_entry'], request=b['request'], frame=frame,
                             original_sequence=seq, source_box=list(r[10:14]), oracle=images[sample['oracle']], sample=sample))
    if not selected or len({f['sequence'] for f in frames}) != len(frames):
        raise ValueError('Missing/duplicate admitted auxiliary publication')
    claims = json.loads(a.claims.read_bytes())
    sources = claims['sources']
    if any(sha((ROOT / n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    source = ROOT / template['artifacts'] / 'inputs'
    native_source = (source / 'producers.bin').read_bytes()
    if sha(native_source) != template['native_input_sha256']:
        raise ValueError('Pinned native parameter input changed')
    parent = ROOT / 'working/tests/drawing-aux-readback-comparison'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    inputs = run / 'inputs'
    (inputs / 'assets').mkdir(parents=True)
    (inputs / 'producers.bin').write_bytes(native_source)
    for name, digest in template['source_assets'].items():
        relative = asset_name(name).casefold()
        src = source / 'assets' / relative
        if sha(src.read_bytes()) != digest:
            raise ValueError('Encoded source asset changed')
        dst = inputs / 'assets' / relative
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
    final = json.loads((source / 'scene.json').read_bytes())['boundary']
    frames.append(final)
    frames.sort(key=lambda f: f['sequence'])
    (inputs / 'scene.json').write_text(json.dumps(dict(schema=2, boundary=final, frames=frames,
                        assets=template['source_assets'], retain_checkpoints=[]), indent=2) + '\n')
    executable_sha = sha(a.executable.read_bytes())
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        result = subprocess.run(['xvfb-run', '-a', str(a.executable), str(inputs / 'producers.bin'),
                                str(inputs / 'assets'), str(inputs / 'scene.json'), str(run / 'native')],
                               env=env, capture_output=True, timeout=600)
        (run / 'native.log').write_bytes(result.stdout + result.stderr)
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    native = json.loads((run / 'native/scene-report.json').read_bytes())
    comparisons = []
    if result.returncode == 0:
        if (not native['success'] or native['checkpoints'] != template['native']['checkpoints']
                or any(native[k] for k in ['original_pixels_used_as_native_inputs', 'original_oracles_read',
                                          'original_raster_results_used_as_native_inputs', 'live_replacement'])):
            raise ValueError('Diagnostic changed native history or oracle/input separation')
        positions = {f['sequence']: i for i, f in enumerate(frames, 1)}
        for item in selected:
            index = positions[item['frame']['sequence']]
            data = (run / ('native/frame-%04d.565' % index)).read_bytes()
            original = (capture / item['oracle']['path']).read_bytes()
            if (len(data) != 3200 or len(original) != 3200 or sha(original) != item['oracle']['sha256']
                    or sha(data) != native['frames'][index - 1]['sha256']):
                raise ValueError('Auxiliary native/original plane changed')
            different = sum(data[i:i + 2] != original[i:i + 2] for i in range(0, 3200, 2))
            comparisons.append(dict(item, native_sha256=sha(data), different_pixels=different))
    if sha(a.executable.read_bytes()) != executable_sha or any(sha((ROOT / n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Source/executable changed during comparison')
    report = dict(schema=1, success=result.returncode == 0 and len(comparisons) == len(selected) and
                  not any(c['different_pixels'] for c in comparisons), sources=sources, claims=claims['claims'],
                  reference=dict(path=str(a.reference), sha256=sha(raw)), template=dict(path=str(a.template), sha256=sha(template_raw)),
                  bindings=dict(path=str(a.bindings), sha256=sha(binding_raw)), native_exit=result.returncode,
                  native=native, comparisons=comparisons, excluded=excluded, artifacts=str(run.relative_to(ROOT)),
                  native_input_sha256=sha(native_source), executable_sha256=executable_sha,
                  checkpoints_unchanged=result.returncode == 0, template_comparison_succeeded=template['success'],
                  original_pixels_used_as_native_inputs=False, original_game_reexecuted=False,
                  replay_order_changed=policy is not None, copy_phase_policy=policy,
                  original_manifest_verified_before_after=True,
                  scope=('External auxiliary pixels compared against an explicit experimental metadata-selected COPY policy template. Save publication follows its transposed COPY; original request/oracle association and all comparison denominators/exclusions remain. No additional input/order/checkpoint changes relative to that template. Extra original Lock may perturb timing/cache; no general schedule or ownership/live replacement proof.' if policy else
                         'External post-save diagnostic mapped-view pixels to source-only native logical saved buffer at exact producer return. All input parameters/order/checkpoints unchanged; within-World/other observed writers excluded explicitly. Additional original Lock can affect cache/timing; no original backing/phase equivalence, ordering repair or whole presentation.'))
    with a.report.open('x') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(json.dumps(dict(success=report['success'], native_exit=result.returncode,
                         compared=len(comparisons), excluded=len(excluded),
                         failed=sum(bool(c['different_pixels']) for c in comparisons))))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Read native buffer values around retained copy-order inversions; never reorder."""
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_comparison import index, plane as complete_plane
from drawing_scene_corpus import asset_name

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--template', type=Path, required=True)
    group = p.add_mutually_exclusive_group(required=True)
    group.add_argument('--bindings', type=Path)
    group.add_argument('--diagnosis', type=Path)
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw, template_raw = read_bytes(a.reference), read_bytes(a.template)
    reference, template = json.loads(raw), json.loads(template_raw)
    association_path = a.bindings or a.diagnosis
    association_raw = read_bytes(association_path)
    associations = json.loads(association_raw)
    if (not reference['success'] or not reference['normal_original_quit_verified'] or
            template['native_exit'] or template['reference']['sha256'] != sha(raw) or
            template['original_pixels_used_as_native_inputs'] or
            not associations['success'] or associations['reference']['sha256'] != sha(raw)):
        raise ValueError('Completed native execution and exact original/association provenance required')
    source = ROOT / template['artifacts'] / 'inputs'
    native_source = (source / 'producers.bin').read_bytes()
    if sha(native_source) != template['native_input_sha256']:
        raise ValueError('Native source-only parameter input changed')
    wire = (Path(reference['experiment']) / 'capture/canvas-producers.bin').read_bytes()
    if sha(wire) != reference['producer_observation']['producer_journal_sha256']:
        raise ValueError('Original producer journal changed')
    spec = importlib.util.spec_from_file_location('order_value_source', ROOT / 'tools/inspect-canvas-producers.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    _, decoded = module.decode(wire, complete=True)
    records = {r[1]: r for r, payload in decoded}
    completed, active, queues, in_world = 0, False, {}, {}
    for r, payload in decoded:
        if r[2] == 11:
            active = True
        elif r[2] == 12:
            active = False
            completed = r[14]
        queues[r[1]], in_world[r[1]] = completed, active
    candidates = []
    if a.bindings:
        bindings = index(associations['bindings'], 'request', 'copy request')
        returned = reference['copy_order_observation']['returns']
        fulls = [q for q in returned if q['source_rectangle'] == [0, 0, 800, 600] and
                 q['destination_position'] == [0, 0] and q['raw_key'] == 0xffffffff]
        saves = [q for q in returned if q['destination_extent_stride'][:2] == [40, 40] and
                 q['destination_position'] == [0, 0] and q['raw_key'] == 0xffffffff and
                 q['source_rectangle'][2] - q['source_rectangle'][0] == 40 and
                 q['source_rectangle'][3] - q['source_rectangle'][1] == 40]
        for full in fulls:
            for save in saves:
                if full['destination'] != save['source'] or not 0 < full['producer_return_sequence'] - save['producer_return_sequence'] <= 20:
                    continue
                if full['request'] not in bindings or save['request'] not in bindings:
                    raise ValueError('Candidate copy request is unbound')
                fc, sc = bindings[full['request']]['commands'], bindings[save['request']]['commands']
                if max(c['worker_return_record'] for c in fc) < min(c['worker_entry_record'] for c in sc):
                    candidates.append(dict(full_sequence=full['producer_return_sequence'], save_sequence=save['producer_return_sequence'],
                                           full_request=full['request'], save_request=save['request'],
                                           full_commands=fc, save_commands=sc,
                                           association_kind='exact-request-api-worker'))
    else:
        if not associations['ordering_reversal_confirmed']:
            raise ValueError('Retained legacy local diagnosis required')
        full, save = associations['copies']['full'], associations['copies']['background_save']
        candidates.append(dict(full_sequence=full['producer_return_sequence'], save_sequence=save['producer_return_sequence'],
                               full_request=None, save_request=None, full_commands=full['commands'], save_commands=save['commands'],
                               association_kind='retained-legacy-local-diagnosis-no-request-ids'))
    frames, selected, refused, occupied = [], [], [], set()
    for candidate in candidates:
        f, s = records[candidate['full_sequence']], records[candidate['save_sequence']]
        if f[2] != 6 or s[2] != 6 or f[3] != s[4] or f[14] != 0xffffffff or s[14] != 0xffffffff:
            raise ValueError('Candidate copy descriptors changed')
        seqs = [s[1] - 1, s[1], f[1]]
        box = list(s[10:14])
        reason = ('nonadjacent-copy-returns' if f[1] != s[1] + 1 else
                  'inside-active-world' if any(in_world[q] for q in seqs) else
                  'overlapping-diagnostic-boundaries' if any(q in occupied for q in seqs) else
                  'unsupported-native-copy-geometry' if list(f[5:7]) != [800, 600] or list(f[10:14]) != [0, 0, 800, 600] or
                  list(f[8:10]) != [0, 0] or list(s[5:7]) != [40, 40] or list(s[8:10]) != [0, 0] or
                  box[2] - box[0] != 40 or box[3] - box[1] != 40 or
                  not 0 <= box[0] < box[2] <= 800 or not 0 <= box[1] < box[3] <= 600 or
                  len({f[3], f[4], s[3]}) != 3 else None)
        if reason:
            refused.append(dict(candidate, reason=reason))
            continue
        added = [dict(sequence=seqs[0], queues=queues[seqs[0]], canvas=f[3], width=800, height=600),
                 dict(sequence=seqs[1], queues=queues[seqs[1]], canvas=f[4], width=800, height=600),
                 dict(sequence=seqs[2], queues=queues[seqs[2]], canvas=s[3], width=40, height=40)]
        frames.extend(added)
        occupied.update(seqs)
        followup, checkpoint = None, None
        for later, payload in decoded[f[1]:]:
            if later[2] in (1, 2) and later[3] in (s[3], f[4]):
                followup = dict(kind='tracked-canvas-lifetime-boundary', sequence=later[1],
                                operation=module.NAMES[later[2]], canvas=later[3])
                break
            if later[2] == 6 and (later[3] == s[3] or
                                  (later[4] == s[3] and later[3] == f[4])):
                followup = dict(kind='copy-into-saved-buffer' if later[3] == s[3] else
                                'saved-buffer-copy-into-full-source', sequence=later[1],
                                source=later[4], destination=later[3],
                                source_rectangle=list(later[10:14]),
                                destination_position=list(later[8:10]), raw_key=later[14],
                                caller_address=later[22])
                break
        for later, payload in decoded[f[1]:]:
            if later[2] in (1, 2) and later[3] == f[4]:
                break
            if later[2] == 10 and later[3] == f[4]:
                checkpoint = dict(sequence=later[1], oracle=later[14], canvas=later[3])
                break
        selected.append(dict(candidate, source_box=box, diagnostic_boundaries=added,
                             full_caller_address=f[22], save_caller_address=s[22],
                             first_subsequent_tracked_copy_or_lifetime_boundary=followup,
                             next_full_source_checkpoint=checkpoint))
    if not selected:
        raise ValueError('No admitted native-only value diagnostic')
    claims = json.loads(read_bytes(a.claims))
    sources = claims['sources']
    if any(evidence_digest(ROOT/name, root=ROOT) != digest for name, digest in sources.items()):
        raise ValueError('Prospective source changed')
    executable_hash = sha(a.executable.read_bytes())
    parent = ROOT / 'working/tests/drawing-copy-order-values'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    inputs = run / 'inputs'
    (inputs / 'assets').mkdir(parents=True)
    (inputs / 'producers.bin').write_bytes(native_source)
    for name, digest in template['source_assets'].items():
        relative = asset_name(name).casefold()
        original = source / 'assets' / relative
        if sha(original.read_bytes()) != digest:
            raise ValueError('Previously pinned encoded input changed')
        target = inputs / 'assets' / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(original, target)
    final = json.loads((source / 'scene.json').read_bytes())['boundary']
    if final['sequence'] in occupied:
        raise ValueError('Diagnostic occupies terminal publication')
    frames.append(final)
    frames.sort(key=lambda f: f['sequence'])
    declaration = dict(schema=2, boundary=final, frames=frames, assets=template['source_assets'], retain_checkpoints=[])
    (inputs / 'scene.json').write_text(json.dumps(declaration, indent=2) + '\n')

    def verify(label):
        result = subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], capture_output=True, timeout=120)
        (run / (label + '.log')).write_bytes(result.stdout + result.stderr)
        result.check_returncode()

    verify('original-before')
    try:
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        execution = subprocess.run(['xvfb-run', '-a', str(a.executable.resolve()), str(inputs / 'producers.bin'),
                                    str(inputs / 'assets'), str(inputs / 'scene.json'), str(run / 'native')],
                                   capture_output=True, timeout=600, env=env)
        (run / 'native.log').write_bytes(execution.stdout + execution.stderr)
    finally:
        verify('original-after')
    native_path = run / 'native/scene-report.json'
    native = json.loads(native_path.read_bytes()) if native_path.exists() else None
    values = []
    if execution.returncode == 0:
        if (not native['success'] or native['frames_published'] != len(frames) or
                native['checkpoints'] != template['native']['checkpoints'] or
                any(native[k] for k in ['original_pixels_used_as_native_inputs', 'original_oracles_read',
                                       'original_raster_results_used_as_native_inputs', 'live_replacement'])):
            raise ValueError('Read-only diagnostic changed checkpoint history or input provenance')
        positions = {f['sequence']: i for i, f in enumerate(frames, 1)}
        def plane(sequence, width, height):
            data = (run / ('native/frame-%04d.565' % positions[sequence])).read_bytes()
            metadata=native['frames'][positions[sequence]-1]
            if (metadata['sequence'],metadata['width'],metadata['height'])!=(sequence,width,height):
                raise ValueError('Diagnostic publication identity differs')
            complete_plane(data,width,height,metadata['sha256'])
            if len(data) != width * height * 2 or sha(data) != native['frames'][positions[sequence] - 1]['sha256']:
                raise ValueError('Diagnostic output identity/extent changed')
            return data
        for item in selected:
            pre, cpu, saved = [plane(b['sequence'], b['width'], b['height']) for b in item['diagnostic_boundaries']]
            left, top, right, bottom = item['source_box']
            def region(data):
                return b''.join(data[(y * 800 + left) * 2:(y * 800 + right) * 2] for y in range(top, bottom))
            prior, current = region(pre), region(cpu)
            different = sum(prior[i:i + 2] != current[i:i + 2] for i in range(0, len(prior), 2))
            values.append(dict(item, native_primary_roi_sha256=sha(prior), native_current_source_roi_sha256=sha(current),
                               native_saved_background_sha256=sha(saved), different_pixels=different,
                               saved_equals_prior_primary=saved == prior, saved_equals_current_source=saved == current))
    if sha(a.executable.read_bytes()) != executable_hash or any(evidence_digest(ROOT/name, root=ROOT) != h for name, h in sources.items()):
        raise ValueError('Source/executable changed during diagnostic')
    report = dict(schema=1, success=execution.returncode == 0 and len(values) == len(selected) and all(v['saved_equals_prior_primary'] for v in values),
                  sources=sources, claims=claims['claims'], native_exit=execution.returncode, native=native,
                  reference=dict(path=str(a.reference), sha256=sha(raw)), template=dict(path=str(a.template), sha256=sha(template_raw)),
                  template_comparison_succeeded=template['success'],
                  associations=dict(path=str(association_path), sha256=sha(association_raw)),
                  executable_sha256=executable_hash, native_input_sha256=sha(native_source), artifacts=str(run.relative_to(ROOT)),
                  values=values, refused=refused, counts=dict(candidates=len(candidates), admitted=len(selected),
                  compared=len(values), different_native_regions=sum(v['different_pixels'] != 0 for v in values)),
                  original_game_reexecuted=False, original_pixels_used_as_native_inputs=False, replay_order_changed=False,
                  checkpoints_unchanged=execution.returncode == 0, original_manifest_verified_before_after=True,
                  scope='Native-only diagnostic publication before a background save, source surface after that save and saved buffer after the adjacent full-copy return. Input bytes, operation order and every checkpoint hash are unchanged. Retain caller addresses and first subsequent copy into the saved buffer or from it into the full source, stopping at tracked canvas creation/release; other writes, other destinations and backing aliases are not classified by this predicate. Compare owned native regions only; exact request/API/worker association or explicitly retained legacy local diagnosis stays separate. No original view/byte ownership, causal repair, general ordering or complete presentation claim.')
    write_report(ROOT, a.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], native_exit=execution.returncode, counts=report['counts'], refused=len(refused))))


if __name__ == '__main__':
    main()

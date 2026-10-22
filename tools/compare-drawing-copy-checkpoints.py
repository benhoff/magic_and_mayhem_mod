#!/usr/bin/env python3
"""Compare every source checkpoint in one completed original copy-journal interval."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
import shutil
import subprocess
import tempfile
from drawing_api_session import BUILD, sha
from drawing_comparison import index, plane
from drawing_scene_corpus import asset_name, raster_inputs
from drawing_copy_phase import select as select_phase, transpose

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--assets-reference', type=Path, required=True)
    parser.add_argument('--input-manifest', type=Path, required=True)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--copy-phase-plan', type=Path, help='Explicit experimental metadata-only offline COPY policy')
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    raw_reference = read_bytes(args.reference)
    reference = json.loads(raw_reference)
    if not reference['success'] or not reference['normal_original_quit_verified'] or reference['source_sha256'] != BUILD:
        raise ValueError('Completed original-active reference required')
    raw_assets = read_bytes(args.assets_reference)
    assets_reference = json.loads(raw_assets)
    if assets_reference['original_pixels_used_as_native_inputs'] or not assets_reference['original_manifest_verified_before_after']:
        raise ValueError('Previously isolated encoded assets required')
    pin_bytes = read_bytes(args.input_manifest)
    pin = json.loads(pin_bytes)
    if pin['executable_sha256'] != BUILD or not pin['original_manifest_verified_before_after']:
        raise ValueError('Pinned installed inputs required')
    sources = dict(json.loads(read_bytes(args.claims))['sources'])
    sources.update({name: evidence_digest(ROOT/name,root=ROOT) for name in
                    ['tools/compare-drawing-copy-checkpoints.py', 'tools/drawing_scene_corpus.py',
                     'tools/inspect-canvas-producers.py', 'tools/drawing_api_session.py']})
    if any(evidence_digest(ROOT/name,root=ROOT) != digest for name, digest in sources.items()):
        raise ValueError('Prospective sources changed')
    executable_hash = sha(args.executable.read_bytes())
    parent = ROOT / 'working/tests/drawing-copy-checkpoints'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))

    def verify(label):
        result = subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'],
                                capture_output=True, timeout=120)
        (run / (label + '.log')).write_bytes(result.stdout + result.stderr)
        result.check_returncode()

    verify('original-before')
    try:
        capture = Path(reference['experiment']) / 'capture'
        producer = (capture / 'canvas-producers.bin').read_bytes()
        if sha(producer) != reference['producer_observation']['producer_journal_sha256']:
            raise ValueError('Original source journal changed')
        spec = importlib.util.spec_from_file_location('copy_checkpoint_producers', ROOT / 'tools/inspect-canvas-producers.py')
        inspector = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(inspector)
        _, records = inspector.decode(producer, complete=True)
        # The renderer accepts drawing operations, while V4's final record is
        # a validated recorder update-end marker. Keep it in reference evidence
        # and remove only that exact payload-free terminal from native inputs.
        terminal = records[-1][0]
        if terminal[0] != 96 or terminal[2] != 27 or terminal[18] or records[-1][1]:
            raise ValueError('Exact V4 update-end metadata required')
        drawing_prefix = producer[:-96]
        checkpoints = [r for r, payload in records if r[2] == 10]
        if len(checkpoints) != reference['source_completion']['oracles']:
            raise ValueError('Missing source checkpoint')
        queues = reference['source_completion']['expected_queues']
        boundary = dict(sequence=records[-2][0][1], queues=queues, canvas=2, width=800, height=600)
        inputs = run / 'inputs'
        asset_root = inputs / 'assets'
        asset_root.mkdir(parents=True)
        native_source = raster_inputs(drawing_prefix)
        phase_policy = None
        if args.copy_phase_plan:
            plan_raw = read_bytes(args.copy_phase_plan, root=ROOT)
            plan = json.loads(plan_raw)
            binding_path = Path(plan['bindings']['path'])
            binding_raw = read_bytes(binding_path,root=ROOT)
            if (not plan['success'] or plan['reference']['sha256'] != sha(raw_reference) or
                    plan['producer_sha256'] != sha(producer) or
                    plan['bindings']['sha256'] != sha(binding_raw) or
                    plan['original_pixels_used_as_native_inputs'] or
                    any(evidence_digest(ROOT/n,root=ROOT) != h for n, h in plan['sources'].items())):
                raise ValueError('Exact current metadata-only phase plan required')
            bindings = json.loads(binding_raw)
            if not bindings['success'] or bindings['reference']['sha256'] != sha(raw_reference):
                raise ValueError('Phase plan bindings changed')
            choices = select_phase(records, reference['copy_order_observation'], bindings)
            if choices != {k: plan[k] for k in ('selected', 'refused')}:
                raise ValueError('Phase selection differs from current guarded policy')
            native_source = transpose(native_source, choices['selected'])
            sources.update(plan['sources'])
            phase_policy = dict(path=str(args.copy_phase_plan), sha256=sha(plan_raw),
                                selected=choices['selected'], refused=choices['refused'])
        (inputs / 'producers.bin').write_bytes(native_source)
        hashes = assets_reference['source_assets']
        if not hashes or len(hashes) > 256:
            raise ValueError('Encoded asset budget')
        pinned = {}
        for name, digest in pin['staged_files'].items():
            normalized = asset_name(name).casefold()
            if normalized in pinned:
                raise ValueError('Ambiguous installed-input spelling')
            pinned[normalized] = digest
        for name, digest in hashes.items():
            normalized = asset_name(name).casefold()
            source = ROOT / assets_reference['artifacts'] / 'inputs/assets' / normalized
            if pinned.get(normalized) != digest or sha(source.read_bytes()) != digest:
                raise ValueError('Encoded template asset differs from pinned installed input')
            target = asset_root / normalized
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists():
                if sha(target.read_bytes()) != digest:
                    raise ValueError('Conflicting asset alias')
            else:
                shutil.copyfile(source, target)
        declaration = dict(schema=1, boundary=boundary, assets=hashes,
                           retain_checkpoints=[r[14] for r in checkpoints])
        (inputs / 'scene.json').write_text(json.dumps(declaration, indent=2) + '\n')
        expected_hashes = {r[14]: sha((capture / f'producer-oracle-{r[14]:04d}.565').read_bytes()) for r in checkpoints}
        environment = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        environment.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        execution = subprocess.run(['xvfb-run', '-a', str(args.executable.resolve()),
                                    str(inputs / 'producers.bin'), str(asset_root),
                                    str(inputs / 'scene.json'), str(run / 'native')],
                                   capture_output=True, timeout=600, env=environment)
        (run / 'native.log').write_bytes(execution.stdout + execution.stderr)
        native_path = run / 'native/scene-report.json'
        native = json.loads(native_path.read_text()) if native_path.exists() else None
        comparisons = []
        if execution.returncode == 0:
            if not native['success'] or native['boundary'] != boundary or native['remaining_surfaces'] or any(native[k] for k in
                    ['original_pixels_used_as_native_inputs', 'original_oracles_read', 'original_raster_results_used_as_native_inputs', 'live_replacement']):
                raise ValueError('Native provenance/closure changed')
            actual = index(native['checkpoints'], 'ordinal', 'native checkpoint')
            if len(actual) != len(checkpoints):
                raise ValueError('Native checkpoint count changed')
            for record in checkpoints:
                sequence, canvas, width, height, ordinal = record[1], record[3], record[5], record[6], record[14]
                original = (capture / f'producer-oracle-{ordinal:04d}.565').read_bytes()
                predicted = (run / f'native/checkpoint-{ordinal:04d}.565').read_bytes()
                n = actual[ordinal]
                plane(original, width, height, expected_hashes[ordinal])
                plane(predicted, width, height, n['sha256'])
                if len(original) != width * height * 2 or len(predicted) != len(original) or sha(original) != expected_hashes[ordinal] or n['sha256'] != sha(predicted) or (n['sequence'], n['canvas'], n['width'], n['height']) != (sequence, canvas, width, height):
                    raise ValueError('Checkpoint identity/extent changed')
                different = 0 if original == predicted else sum(original[i:i + 2] != predicted[i:i + 2] for i in range(0, len(original), 2))
                comparisons.append(dict(ordinal=ordinal, sequence=sequence, canvas=canvas, width=width, height=height,
                                        original_sha256=sha(original), native_sha256=sha(predicted), different_pixels=different))
        if sha(args.executable.read_bytes()) != executable_hash or any(evidence_digest(ROOT/name,root=ROOT) != digest for name, digest in sources.items()):
            raise ValueError('Sources or executable changed during comparison')
    finally:
        verify('original-after')
    report = dict(schema=1, success=execution.returncode == 0 and len(comparisons) == len(checkpoints) and all(not c['different_pixels'] for c in comparisons),
                  sources=sources, sources_stable=True, claims=json.loads(read_bytes(args.claims))['claims'],
                  reference=dict(path=str(args.reference), sha256=sha(raw_reference)),
                  assets_reference=dict(path=str(args.assets_reference), sha256=sha(raw_assets)),
                  input_manifest=dict(path=str(args.input_manifest), sha256=sha(pin_bytes)),
                  executable_sha256=executable_hash, producer_sha256=sha(producer), native_input_sha256=sha(native_source),
                  artifacts=str(run.relative_to(ROOT)), native_exit=execution.returncode, native=native,
                  original_terminal_metadata=list(terminal), native_terminal_metadata_separately_validated=True,
                  comparisons=comparisons, source_assets=hashes, original_game_executed=False,
                  original_manifest_verified_before_after=True, original_pixels_used_as_native_inputs=False,
                  replay_order_changed=phase_policy is not None, copy_phase_policy=phase_policy,
                  complete_journey_coverage=False,
                  scope=('Explicit experimental metadata-selected adjacent full/save transpositions; all checkpoint identities and denominators remain, original pixels stay external. Only guarded COPY bodies are transposed; no all-writer/view/GPU/CPU ownership, general schedule, live replacement or whole journey closure.' if phase_policy is not None else
                         'Source-only unchanged return-order native replay of every checkpoint in a completed original copy-journal interval. The strict original V4 update-end bookkeeping marker is validated separately and retained; all drawing operations/checkpoints remain. Only parameters and independently pinned encoded assets enter native inputs; all original planes stay external. No general ordering repair, source/view ownership, physical presentation or whole journey closure.'))
    write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], compared=len(comparisons),
                          failed=sum(bool(c['different_pixels']) for c in comparisons), native_exit=execution.returncode)))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Recheck interaction semantics against a complete retained independent pixel proof."""
import argparse
import copy
import hashlib
import inspect
import json
from pathlib import Path
import subprocess
import sys
from coverage_evidence import Reader, write_report
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require
import rendering_interaction_assessment as semantics
from broader_interaction_source_chunks import RetainedCameraHistory, bind_assessor
ROOT = Path(__file__).resolve().parents[1]
SUPPORT = ('tools/broader_interaction_source_chunks.py', 'tools/analyze-broader-retained-interaction.py')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ('acquisition', 'pixels', 'controls', 'report'):
        parser.add_argument('--' + key, required=True)
    args = parser.parse_args()
    require(args.report.startswith('research/runtime/rendering-interaction-results-20261107/')
            and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh owned result required')
    out = ROOT / 'working/broader-player/retained-interaction-semantics' / Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').write_text('Current semantic analysis and original manifest logs; retain all failures.\n')
    with Reader(ROOT) as reader:
        ref = lambda n: dict(record=n, sha256=reader.digest(n))
        parent = reader.json(args.pixels)
        acquired = reader.json(args.acquisition)
        parent_ref, acquired_ref = ref(args.pixels), ref(args.acquisition)
        require(parent['acquisition'] == acquired_ref and parent['error'] == 'Camera packet crosses a source chunk'
                and not parent['success'] and acquired['success'] and acquired['private_wine_stopped'],
                'Different retained analysis refusal')
        require(parent['sources_stable'] and parent['comparison_processes_stopped']
                and parent['original_manifest_verified_before_after'] and parent['comparison']['success']
                and parent['comparison']['mismatching_pixels'] == 0
                and parent['original_reference']['continuous_from_owned_initial_history']
                and parent['original_reference']['native_player_pixels_read'] is False
                and parent['original_reference']['original_destination_capture_read'] is False,
                'Incomplete/dependent retained pixel proof')
        require(all(reader.digest(n) == h for n, h in (acquired['sources'] | parent['sources']).items()),
                'Retained capture or pixel/oracle source changed')
        controls = reader.json(args.controls)
        require(controls['success'] and controls['tests'] == 16
                and all(reader.digest(n) == h for n, h in controls['sources'].items()), 'Source chunk controls stale')
        sources = parent['sources'] | {n: reader.digest(n) for n in SUPPORT}
        result = copy.deepcopy(parent)
        result.update(success=False, sources=sources, sources_stable=False,
                      original_manifest_verified_before_after=False)
        result.pop('error', None)
        result.pop('proof', None)
        result['output_directory'] = str(ROOT / 'working/interaction-comparisons' / Path(args.pixels).stem)
        result['semantic_diagnostics_directory'] = str(out)
        result['retained_pixel_comparison'] = dict(record=parent_ref['record'], sha256=parent_ref['sha256'],
            new_original_reference_execution=False, new_pixel_comparison=False,
            all_parent_source_fingerprints_verified=True, scope='Exact complete pixel proof from comparison02; its whole interaction refusal remains preserved.')
        result['analysis_input_binding'] = dict(parent['analysis_input_binding'],
            executed_for_this_semantic_analysis=False, retained_from=parent_ref,
            scope='Quoted provenance of the retained pixel execution; current semantic dependency binding is recorded separately.')
        history = RetainedCameraHistory(ROOT)
        before = False
        logs = []

        def manifest(label):
            completed = subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], cwd=ROOT,
                                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            path = out / (label + '.log')
            path.write_bytes(completed.stdout)
            name = args.report.removesuffix('.json') + '-' + label + '.bin'
            logs.append(dict(record=name, sha256=write_report(ROOT, name, completed.stdout), returncode=completed.returncode))
            require(completed.returncode == 0, 'Original manifest verification failed')

        try:
            manifest('original-before')
            before = True
            assets = Path(acquired['session']) / 'game'
            def asset_hashes_match():
                for n, expected in parent['asset_inputs'].items():
                    with (assets / n).open('rb') as stream:
                        require(hashlib.file_digest(stream, 'sha256').hexdigest() == expected, 'Retained reference asset changed')
            asset_hashes_match()
            streams = [artifact(reader, acquired, Path(v['path']).name) for v in acquired['artifacts']
                       if '/capture/' in v['path'] and v['path'].endswith('.bin')]
            observed = acquisition(streams, acquired['session_identity'])
            require(hashlib.sha256(observed['commands']).hexdigest() == parent['captured_command_stream_sha256'],
                    'Retained command history changed')
            bound = bind_assessor(semantics.assess, history)
            result['proof'] = bound(reader, acquired, parent, observed)
            require(history.calls == 1 and history.diagnostics is not None, 'Source history not checked exactly once')
            asset_hashes_match()
            result['asset_inputs_stable'] = True
            result['current_semantic_analysis'] = dict(controls=ref(args.controls), original_game_runs=0,
                new_native_executions=0, new_original_reference_executions=0, new_pixel_comparisons=0,
                unchanged_semantic_function='tools/rendering_interaction_assessment.py:assess',
                unchanged_function_source_sha256=hashlib.sha256(inspect.getsource(semantics.assess).encode()).hexdigest(),
                replaced_dependency='source_contexts only', source=history.source,
                source_chunks=history.diagnostics, complete_camera_source_chunks=sum(v['complete_camera_packet'] for v in history.diagnostics),
                source_chunks_credited_as_Worlds=0, source_chunks_credited_as_action_witnesses=0,
                scope='Every source packet retains the original camera/terrain/cell/creature/raster checks. Complete camera packets in source chunks are validated and excluded from World/action credit. All original finite semantic guards and the exact independent pixel proof remain mandatory.')
            result['success'] = result['proof']['verified']
        except Exception as error:
            result.update(success=False, error=str(error))
        finally:
            if before:
                try:
                    manifest('original-after')
                    result['original_manifest_verified_before_after'] = True
                except Exception as error:
                    result.update(success=False, original_verification_error=str(error))
            result['semantic_analysis_logs'] = logs
            result['sources_stable'] = all(reader.digest(n) == h for n, h in result['sources'].items())
            result['success'] &= result['sources_stable'] and result['original_manifest_verified_before_after']
            digest = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
            print(json.dumps(dict(success=result['success'], error=result.get('error'), sha256=digest,
                                 original_game_runs=0, new_pixel_comparisons=0)), flush=True)
    return int(not result['success'])


if __name__ == '__main__': raise SystemExit(main())

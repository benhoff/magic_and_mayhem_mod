#!/usr/bin/env python3
"""Independent original-history comparison of the finite Campaign Mini pause/resume case."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tarfile
import numpy as np
from coverage_evidence import Reader, write_report
from rendering_pilot import acquisition, compare, inflate, require
from rendering_menu_acceptance import artifact
from comparison_process import Processes

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-pause-results-20261107/'


def main():
    p = argparse.ArgumentParser(description=__doc__); p.add_argument('--acquisition', required=True); p.add_argument('--report', required=True)
    p.add_argument('--reuse-pixels', help='Retained exact successful pixel comparison; rerun only the interaction analysis')
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh immutable comparison required')
    out = ROOT / 'working/pause-comparisons' / Path(args.report).stem; out.mkdir(parents=True, exist_ok=False); (out / '.retain').touch()
    result = dict(schema=1, kind='public_native_pause_mini_comparison', success=False, original_pixels_used_as_native_inputs=False)
    processes, before = Processes(), False
    with Reader(ROOT) as reader:
        a = reader.json(args.acquisition); result['acquisition'] = dict(record=args.acquisition, sha256=reader.digest(args.acquisition))
        plan_name = a['prospective_plan']['record']
        require(reader.digest(plan_name)==a['prospective_plan']['sha256'],'Prospective pause plan changed')
        plan = reader.json(plan_name)
        result['prospective_plan'] = dict(record=plan_name, sha256=reader.digest(plan_name))
        sources = a['sources'] | a.get('native_admission', {}).get('native_only', {}).get('sources', {}) | plan['oracle']['native_sources']
        sources.update({n: reader.digest(n) for n in ('tools/check-rendering-pause-low.py','tools/rendering_pause_low_assessment.py','tools/pause_low_ingress.py','tools/check-rendering-pause.py', 'tools/rendering_pause_assessment.py', 'tools/rendering_pause.py', 'tools/close-rendering-camera-matrix.py', 'tools/rendering_interaction.py',
            'tools/rendering_pilot.py', 'tools/rendering_capture.py', 'tools/comparison_process.py', 'tools/close-rendering-source-camera.py',
            'tools/close-rendering-world-exit.py', 'tools/join-rendering-normal-acceptance.py', 'tools/rendering_normal_acceptance.py',
            'tools/rendering_menu_acceptance.py', 'tools/resident_capture.py', 'tools/inspect-canvas-producers.py',
            'tools/rendering_gameplay_acceptance.py', 'tools/campaign_input_observation.py', 'tools/test-menu-observer.py')})
        result['sources'] = sources
        def execute(label, command):
            with (out / (label + '.log')).open('xb') as log: processes.run([str(v) for v in command], cwd=ROOT, stdout=log, timeout=300)
        try:
            require(a['success'] and a['sources_stable'] and a['private_wine_stopped']
                    and all(reader.digest(n) == h for n, h in sources.items()), 'Incomplete/changed camera source phase')
            require(all(a['sources'].get(n)==h for n,h in plan['runtime_sources'].items()),'Prospectively frozen runtime closure changed')
            for binary in [a['player_binary'], *plan['oracle']['binaries']]:
                require(reader.digest(binary['record']) == binary['sha256'] == hashlib.sha256((ROOT / binary['path']).read_bytes()).hexdigest(), 'Exact accepted executable differs')
            streams = [artifact(reader, a, Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
            observed = acquisition(streams, a['session_identity'])
            result['observations'] = observed['observations']; result['captured_command_stream_sha256'] = hashlib.sha256(observed['commands']).hexdigest()
            from rendering_normal_acceptance import world_domain, lighting
            result['world_domain'] = world_domain(observed)
            result['lighting'] = lighting(ROOT, a, {v['path']: v for v in a['artifacts']})
            source = Path(a['temporary_root']) / 'worlds.bin'
            require(source.read_bytes() == artifact(reader, a, 'worlds.bin'), 'Physical source capture changed')
            execute('original-before', [ROOT / 'tools/original-manifest.sh', 'verify']); before = True
            pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
            require(hashlib.sha256(pe.read_bytes()).hexdigest() == '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168', 'Original build differs')
            assets = Path(a['session']) / 'game'
            extensions = {'.spr', '.jpg', '.jpeg', '.bmp', '.pcx', '.sft', '.fp', '.ttd', '.wav'}
            asset_inputs = {str(f.relative_to(assets)): hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(assets.rglob('*')) if f.is_file() and f.suffix.lower() in extensions}
            require(asset_inputs and all(a['installation_inputs'][n]['sha256'] == h for n, h in asset_inputs.items()), 'Reference asset differs')
            result['asset_inputs'] = asset_inputs
            if args.reuse_pixels:
                parent = reader.json(args.reuse_pixels)
                changed_analysis = {'tools/check-rendering-pause-low.py','tools/rendering_pause_low_assessment.py','tools/pause_low_ingress.py','tools/check-rendering-pause.py', 'tools/rendering_pause_assessment.py'}
                require(parent['acquisition'] == result['acquisition'] and parent['sources_stable']
                        and parent['original_manifest_verified_before_after'] and parent['comparison_processes_stopped']
                        and parent['comparison']['success'] and parent['comparison']['mismatching_pixels'] == 0
                        and parent['captured_command_stream_sha256'] == result['captured_command_stream_sha256']
                        and parent['world_domain'] == result['world_domain'] and parent['asset_inputs'] == asset_inputs
                        and set(parent['sources']) == set(sources)
                        and all(h == sources[n] for n, h in parent['sources'].items() if n not in changed_analysis),
                        'Retained pixel proof inputs/oracle changed')
                result['original_reference'] = parent['original_reference']
                result['comparison'] = parent['comparison']
                result['retained_pixel_comparison'] = dict(record=args.reuse_pixels, sha256=reader.digest(args.reuse_pixels),
                    source_phase_changes={n:dict(before=parent['sources'][n],after=sources[n]) for n in changed_analysis},
                    new_original_reference_execution=False, new_pixel_comparison=False)
            else:
                reference = out / 'reference'
                execute('original-history', [ROOT / 'working/build/qt-shell/mnm-acceptance-world-history', source, assets, ROOT / 'working/original-reference', pe, reference])
                index = json.loads((reference / 'worlds.json').read_text())
                require(index['success'] and index['native_player_pixels_read'] is False and index['original_destination_capture_read'] is False, 'Dependent original oracle')
                worlds = {}
                for row in index['worlds']:
                    g = int(row['generation']); require(g == len(worlds) + 1 and Path(row['path']).name == row['path'], 'Original World ordering')
                    raw = inflate((reference / row['path']).read_bytes())
                    require(len(raw) == row['width'] * row['height'] * 2, 'Incomplete original plane')
                    worlds[g] = np.frombuffer(raw, dtype='<u2').astype('uint32').reshape(row['height'], row['width'])
                result['original_reference'] = dict(worlds=len(worlds), source_chunks=int(index.get('source_chunks', '0')), draws=index['draws'],
                    native_player_pixels_read=False, original_destination_capture_read=False, continuous_from_owned_initial_history=True)
                result['comparison'] = compare(observed, worlds)
            require(all(hashlib.sha256((assets / n).read_bytes()).hexdigest() == h for n, h in asset_inputs.items()), 'Reference assets changed')
            from rendering_pause_low_assessment import assess
            result['proof'] = assess(reader, a, result, observed)
            result['success'] = result['comparison']['success'] and result['proof']['verified']
        except Exception as error:
            result['error'] = str(error)
        finally:
            if before:
                try:
                    execute('original-after', [ROOT / 'tools/original-manifest.sh', 'verify']); result['original_manifest_verified_before_after'] = True
                except Exception as error: result.update(success=False, original_verification_error=str(error))
            result['sources_stable'] = all(reader.digest(n) == h for n, h in sources.items())
            result['success'] &= result['sources_stable']
            # One immutable bounded archive preserves every exact output and
            # member hash without thousands of SQLite transactions.
            bundle = out / 'comparison-outputs.tar.gz'
            members = []
            files = sorted(f for f in out.rglob('*') if f.is_file())
            with tarfile.open(bundle, 'w:gz', compresslevel=1) as archive:
                for f in files:
                    raw = f.read_bytes(); relative = str(f.relative_to(out))
                    members.append(dict(path=relative, bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest()))
                    archive.add(f, arcname=relative, recursive=False)
            raw = bundle.read_bytes(); name = args.report.removesuffix('.json') + '-outputs.bin'
            result['artifacts'] = [dict(path=bundle.name, record=name, sha256=write_report(ROOT,name,raw), bytes=len(raw), encoding='tar-gzip', members=members)]
            result['comparison_processes_stopped'] = processes.stopped()
            h = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
            print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=h,
                                  worlds=result.get('original_reference', {}).get('worlds'))), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())

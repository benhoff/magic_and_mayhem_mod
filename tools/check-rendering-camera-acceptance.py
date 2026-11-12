#!/usr/bin/env python3
"""Independent original-history comparison of a stopped public camera session."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import numpy as np
from coverage_evidence import Reader, write_report
from rendering_pilot import acquisition, compare, inflate, require
from rendering_menu_acceptance import artifact
from comparison_process import Processes

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-camera-matrix-results-20261106/'


def main():
    p = argparse.ArgumentParser(description=__doc__); p.add_argument('--acquisition', required=True); p.add_argument('--report', required=True); args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh immutable comparison required')
    out = ROOT / 'working/camera-comparisons' / Path(args.report).stem; out.mkdir(parents=True, exist_ok=False); (out / '.retain').touch()
    result = dict(schema=1, kind='public_native_camera_comparison', success=False, original_pixels_used_as_native_inputs=False)
    processes, before = Processes(), False
    with Reader(ROOT) as reader:
        a = reader.json(args.acquisition); result['acquisition'] = dict(record=args.acquisition, sha256=reader.digest(args.acquisition))
        plan = reader.json(PREFIX + 'input-plan-01.json')
        sources = a['sources'] | a.get('native_admission', {}).get('native_only', {}).get('sources', {}) | plan['oracle']['native_sources']
        sources.update({n: reader.digest(n) for n in ('tools/check-rendering-camera-acceptance.py', 'tools/close-rendering-camera-matrix.py',
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
            require(all(hashlib.sha256((assets / n).read_bytes()).hexdigest() == h for n, h in asset_inputs.items()), 'Reference assets changed')
            result['comparison'] = compare(observed, worlds)
            spec = importlib.util.spec_from_file_location('camera_matrix', ROOT / 'tools/close-rendering-camera-matrix.py')
            matrix = importlib.util.module_from_spec(spec); spec.loader.exec_module(matrix)
            result['proof'] = matrix.assess(reader, a, result, observed)
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
            result['artifacts'] = []
            for f in sorted(out.rglob('*')):
                if not f.is_file(): continue
                raw = f.read_bytes(); name = args.report.removesuffix('.json') + '-' + hashlib.sha256(str(f.relative_to(out)).encode()).hexdigest()[:20] + '.bin'
                result['artifacts'].append(dict(path=str(f.relative_to(out)), record=name, sha256=write_report(ROOT, name, raw), bytes=len(raw)))
            result['comparison_processes_stopped'] = processes.stopped()
            h = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
            print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=h,
                                  worlds=result.get('original_reference', {}).get('worlds'))), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())

"""Close and optionally compare the source-only native startup producer session."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import signal


def run_startup(repository, build, prefix_template, *, minimap_owned=False, motion=False, claims=None):
    """Reuse the validated bounded menu driver and capture lifecycle."""
    command = [sys.executable, str(repository/'tools/capture-scene-game.py'),
               '--canvas-producers', '--world-producer-handoff', '--samples', '16',
               '--producer-oracle-mib', '3072', '--skip-window-screenshot',
               '--map', '2', '--magic-items', '0', '--producer-live', str(build/'mnm-canvas-producers-live'),
               '--prefix-template', str(prefix_template.resolve())]
    if minimap_owned:command.append('--minimap-owned')
    if claims:command += ['--claims',str(claims.resolve())]
    if motion:
        fixture_build=build/'minimap-input'
        subprocess.run(['cmake','-S',str(repository/'tests/minimap-live-input'),'-B',str(fixture_build)],check=True)
        subprocess.run(['cmake','--build',str(fixture_build),'-j4'],check=True)
        command += ['--minimap-input-fixture',str(fixture_build/'mnm-minimap-live-input')]
    print('Automatically running Quick Battle startup; comparing every completed native canvas.', flush=True)
    root = None
    env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
    process = subprocess.Popen(command, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               text=True, start_new_session=True)
    try:
        for line in process.stdout:
            line = line.rstrip('\n')
            candidate = Path(line)
            if line.startswith(str(repository/'working/experiments/scene-observer/run-')) and candidate.is_dir():
                root = candidate
                print('Native World shadow session: '+str(root), flush=True)
            else:
                print(line, flush=True)
        process.wait()
        if root is None:raise RuntimeError('Native startup capture failed before staging')
        result = complete(root, True)
        try:
            if minimap_owned:
                captured=json.loads((root/'report.json').read_text())
                observed=captured['canvas_producers']['minimap_owned']
                native=result['native']
                if native['wire_version']!=2 or not native['minimap_operations'] or not observed['terrain_calls'] or any(not observed['kinds'].get(str(k)) for k in range(3)):raise RuntimeError('Incomplete native minimap composition')
                if motion:
                    timeline=observed['timeline']
                    if observed['orientations']!=[0,1,2,3] or not any(a['view']==b['view'] and a['center']!=b['center'] for a,b in zip(timeline,timeline[1:])):raise RuntimeError('Incomplete minimap rotation/pan coverage')
                result.update(minimap_owned=observed,minimap_fixture_pacing=motion)
                (root/'native-world.json').write_text(json.dumps(result,indent=2)+'\n')
        except (KeyError, ValueError, RuntimeError) as error:
            result.update(success=False,error=str(error))
            (root/'native-world.json').write_text(json.dumps(result,indent=2)+'\n')
            raise RuntimeError(str(error)) from error
        if process.returncode:
            details=root/'report.json' if (root/'report.json').exists() else root
            result.update(success=False, error='Native startup capture failed; details: '+str(details))
            (root/'native-world.json').write_text(json.dumps(result, indent=2)+'\n')
            raise RuntimeError(result['error'])
        print(f"Native startup complete: {result['queues']} World queues, {len(result['checkpoints'])} completed canvases; every canvas matches the original.", flush=True)
    finally:
        if process.poll() is None:
            process.send_signal(signal.SIGINT)
            try:process.wait(timeout=60)
            except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
        process.stdout.close()


def complete(root, verify):
    report = {'success': False, 'verify': verify, 'original_pixels_used_as_native_inputs': False,
              'original_work_bypassed': False}
    try:
        if (root/'capture-policy.json').exists():
            report['capture_policy']=json.loads((root/'capture-policy.json').read_text())
        spec = importlib.util.spec_from_file_location('startup_producers', Path(__file__).with_name('inspect-canvas-producers.py'))
        inspector = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(inspector)
        if not (root/'native-producers/live-report.json').exists():
            raise RuntimeError('Native startup session ended before completing all 16 queues; details: '+str(root/'native-producers.log'))
        native = json.loads((root/'native-producers/live-report.json').read_text())
        if not native['success']:
            raise RuntimeError(native.get('error') or 'Native startup worker failed')
        captured = inspector.analyze(root/'capture')
        spec = importlib.util.spec_from_file_location('startup_queues', Path(__file__).with_name('inspect-startup-queues.py'))
        queues = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(queues)
        report['startup_queues'] = queues.collect(root/'capture', 16)
        if not report['startup_queues']['all_raw_rows_captured']:
            raise RuntimeError('Startup queue diagnostics omitted raw draw rows')
        checks = native['checkpoints']
        expected = captured['checkpoints']
        if (not captured['all_observed_inputs_admitted'] or captured['queues'] != 16
                or not native['success'] or native['queues'] != 16
                or native['records'] != captured['records'] or native['remaining_surfaces']
                or native['input_sha256'] != captured['sha256']
                or native['viewport_image_uploads'] or native['original_oracles_read']
                or native['original_pixels_used_as_native_inputs'] or native['native_bypass_count']
                or native.get('native_canvas_writebacks', 0) or native['live_replacement']
                or not native['world_handoff'] or native['world_queues'] != 16
                or [f['queue'] for f in native['world_frames']] != list(range(1, 17))
                or not all(f['gpu_world_equal'] for f in native['world_frames'])
                or not checks or len(checks) != len(expected)):
            raise RuntimeError(native.get('error') or 'Incomplete native startup producer/World handoff')
        compared = []
        report.update(queues=16, records=captured['records'], checkpoints=compared,
                      input_sha256=captured['sha256'], native=native)
        for old, new in zip(expected, checks):
            if ((old['sequence'], old['oracle'], old['canvas']) != (new['sequence'], new['oracle'], new['canvas'])
                    or not new['gpu_equal'] or new['path'] != f"native-{old['oracle']:04d}.565"):
                raise RuntimeError('Native startup completion identity changed')
            actual = (root/'native-producers'/new['path']).read_bytes()
            if len(actual) != old['width']*old['height']*2 or hashlib.sha256(actual).hexdigest() != new['sha256']:
                raise RuntimeError('Native startup completion extent/hash changed')
            row = {key: old[key] for key in ('sequence', 'oracle', 'canvas', 'width', 'height')}
            if verify:
                original = (root/'capture'/old['path']).read_bytes()
                if hashlib.sha256(original).hexdigest() != old['sha256']:
                    raise RuntimeError('Original startup completion changed during comparison')
                row['mismatches'] = sum(a != b for a, b in zip(struct.iter_unpack('<H', actual), struct.iter_unpack('<H', original))) if actual != original else 0
            compared.append(row)
        report['success'] = not verify or all(c['mismatches'] == 0 for c in compared)
        if not report['success']:
            first = next(c for c in compared if c['mismatches'])
            raise RuntimeError(f"Native startup canvas {first['canvas']} checkpoint {first['oracle']} differs from original: {first['mismatches']} pixels")
        return report
    except (OSError, ValueError, KeyError, TypeError, RuntimeError) as error:
        report['error'] = str(error)
        raise RuntimeError(str(error) + '; report: ' + str(root/'native-world.json')) from error
    finally:
        (root/'native-world.json').write_text(json.dumps(report, indent=2)+'\n')

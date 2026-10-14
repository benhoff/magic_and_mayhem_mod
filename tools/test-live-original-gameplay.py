#!/usr/bin/env python3
"""Bounded original-display campaign observation; gameplay remains ordinary input.

Starts a private X server, stages an explicitly passive observer, imports one
unchanged checkpoint, and publishes session.json for physical input helpers.
Stop through the game's Quit controls. A stop file is bounded cleanup, not a
normal-quit result. No mission-completion or native-menu validation assertion.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]

def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--checkpoint', type=Path, required=True)
    ap.add_argument('--checkpoint-sha256', required=True)
    ap.add_argument('--claims', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--prefix', type=Path, required=True)
    ap.add_argument('--seconds', type=int, default=1200)
    a = ap.parse_args()
    if not 30 <= a.seconds <= 1800: ap.error('seconds must be 30..1800')
    checkpoint = a.checkpoint.resolve(); out = a.output.resolve()
    if not checkpoint.is_relative_to(ROOT / 'working') or digest(checkpoint) != a.checkpoint_sha256:
        raise ValueError('Checkpoint must be hash-checked working input')
    if not out.is_relative_to(ROOT / 'working') or out.exists():
        raise ValueError('Output must be a fresh working directory')
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    if any(digest(ROOT / p) != h for p, h in sources.items()):
        raise ValueError('Declared source changed before execution')
    out.mkdir(parents=True); (out / '.retain').write_text('Original gameplay diagnostics and unchanged checkpoint provenance.\n')
    shutil.copy2(a.claims, out / 'claims.json')
    env = dict(os.environ, WINEDEBUG='fixme-all', MNM_MENU_DIRECTPLAY_DX6='0',
               LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4')
    for key in ['MNM_CAMPAIGN_CASTING_COMBAT', 'MNM_MENU_CHANNEL', 'MNM_MENU_ORIGINAL_GAMEPLAY',
                'MNM_MENU_GAMEPLAY_OBSERVE', 'MNM_MENU_SPELL_OBSERVE', 'MNM_MENU_CAMPAIGN_OBSERVE']:
        env.pop(key, None)
    server = game = None; experiment = None; normal = False; error = ''
    def terminate(signum, frame):
        raise InterruptedError('Bounded termination signal')
    signal.signal(signal.SIGTERM, terminate)
    try:
        with (out / 'display').open('w+') as display, (out / 'xvfb.log').open('w') as log:
            # Children remain in the retention guard's process group.
            server = subprocess.Popen(['Xvfb', '-displayfd', str(display.fileno()), '-screen', '0',
                                       '1024x768x24', '-nolisten', 'tcp'], pass_fds=(display.fileno(),),
                                      stdout=log, stderr=subprocess.STDOUT)
            for _ in range(100):
                display.seek(0); n = display.read().strip()
                if n: env['DISPLAY'] = ':' + n; break
                time.sleep(.1)
            else: raise RuntimeError('Private display unavailable')
        with (out / 'guards.log').open('w') as log:
            subprocess.run(['python3', '-B', ROOT / 'tests/original-gameplay-launch-test.py'],
                           cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        p = subprocess.run(['python3', ROOT / 'tools/prepare-menu-observer.py', '--campaign-observe',
                            '--original-gameplay'], cwd=ROOT, env=env, capture_output=True, text=True, check=True)
        (out / 'prepare.log').write_text(p.stdout + p.stderr)
        paths = [Path(line[20:]) for line in p.stdout.splitlines() if line.startswith('Evidence directory: ')]
        if len(paths) != 1: raise RuntimeError('Missing prepared experiment')
        experiment = paths[0]; (experiment / '.retain').write_text('Referenced original gameplay observation.\n')
        saves = experiment / 'game/Save'; saves.mkdir(exist_ok=True)
        if list(saves.glob('*.sav')): raise ValueError('Expected empty disposable save folder')
        shutil.copy2(checkpoint, saves / checkpoint.name)
        if digest(saves / checkpoint.name) != a.checkpoint_sha256: raise ValueError('Checkpoint copy changed')
        with (out / 'game.log').open('w') as log:
            game = subprocess.Popen(['python3', ROOT / 'tools/run-menu-observer.py', experiment,
                                     '--prefix', a.prefix.resolve()], cwd=ROOT, env=env,
                                    stdout=log, stderr=subprocess.STDOUT)
            start = time.monotonic()
            while game.poll() is None and time.monotonic() - start < a.seconds and not (out / 'stop').exists():
                (out / 'session.json').write_text(json.dumps(dict(display=env['DISPLAY'], shell_pid=game.pid,
                    experiment=str(experiment), prefix=str(a.prefix.resolve()),
                    seconds=time.monotonic() - start, original_display=True), indent=2) + '\n')
                time.sleep(1)
            normal = game.poll() == 0
    except Exception as e:
        error = str(e)
    finally:
        if game and game.poll() is None:
            game.terminate()
            try: game.wait(timeout=30)
            except subprocess.TimeoutExpired: game.kill(); game.wait()
        if a.prefix.exists():
            subprocess.run(['wineserver', '-k'], env=dict(env, WINEPREFIX=str(a.prefix.resolve())), timeout=20)
        if server and server.poll() is None:
            server.terminate(); server.wait(timeout=10)
        with (out / 'original-after.log').open('w') as log:
            subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], cwd=ROOT,
                           stdout=log, stderr=subprocess.STDOUT, check=True)
    from campaign_gameplay import read_rows
    from campaign_spell import read_spell_rows
    rows = spells = []
    if experiment and (experiment / 'gameplay-events.bin').exists():
        rows = read_rows(experiment / 'gameplay-events.bin', complete=True)
    if experiment and (experiment / 'spell-events.bin').exists():
        spells = read_spell_rows(experiment / 'spell-events.bin', complete=True)
    stable = all(digest(ROOT / p) == h for p, h in sources.items())
    success = normal and stable and bool(rows) and bool(spells) and not error
    report = dict(success=success, normal_launcher_exit=normal, error=error, source_stable=stable,
                  claims=declaration['claims'], sources=sources, checkpoint=str(checkpoint),
                  checkpoint_sha256=a.checkpoint_sha256, experiment=str(experiment),
                  creature_rows=len(rows), spell_rows=len(spells), mission_completion=False,
                  native_character_live_validation=False,
                  scope='Explicit original-display passive creature/spell observation; no simulation, native Character or mission-completion proof')
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(out / 'report.json')
    return 0 if success else 1

if __name__ == '__main__':
    raise SystemExit(main())

#!/usr/bin/env python3
"""Bounded original -win native-copy/unknown-reader frontier investigation."""
import argparse
import hashlib
import json
import os
import shlex
from pathlib import Path
import subprocess
import sys
import time

from coverage_evidence import write_report
from native_copy_routes import decode
from native_windowed_copy import validate
from private_x11_windows import snapshot

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=('controls', 'game'), required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--source-game', type=Path)
    parser.add_argument('--prefix-template', type=Path)
    args = parser.parse_args()
    if args.output.exists() or args.report.exists(): parser.error('Require new output/report paths')
    if not args.output.resolve().is_relative_to(ROOT / 'working'):
        parser.error('Use task-owned working output')
    if args.mode == 'game' and not args.source_game: parser.error('Game mode requires admitted source-game')
    declaration = json.loads(args.claims.read_text())
    def stable():
        return all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == h
                   for p, h in declaration['sources'].items())
    if not stable(): parser.error('Prospective inputs changed')
    args.output.mkdir(parents=True)
    (args.output / '.retain').write_text('Native windowed copy/reader evidence and retained attempts.\n')
    report = dict(schema=1, success=False, sources=declaration['sources'], claims=declaration['claims'],
                  scope=__doc__, mode=args.mode, artifacts=str(args.output.resolve()),
                  original_game_executed=False, complete_replacement=False)
    env = dict(os.environ, PYTHONDONTWRITEBYTECODE='1', TMPDIR=str(args.output.resolve()))
    try:
        command = [sys.executable, '-B', str(ROOT / 'tests/test-native-windowed-copy.py'), '-v']
        if args.mode == 'game':
            command = [sys.executable, '-B', str(ROOT / 'tools/test-native-campaign.py'),
                       '--claims', str(args.claims.resolve()), '--source-game', str(args.source_game.resolve()),
                       '--original-windowed', '--resident-world', '--independent-world', '--other-pixel-producers',
                       '--no-pixel-forwarding', '--gpu-copy-diagnostics', '--copy-diagnostic-scope', 'all',
                       '--normal-quit', '--stress-seconds', '10', '--difficulty', '1',
                       '--fps-policy', 'diagnostic', '--skip-movies', '--legacy-audio',
                       '--no-native-music', '--no-route-screenshots', '--frame-trace', '--timeout', '600']
            if args.prefix_template: command += ['--prefix-template', str(args.prefix_template.resolve())]
        report['command'] = command
        start = time.monotonic()
        report['private_window_observations'] = []
        with (args.output / 'execution.log').open('xb') as log:
            child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
            while child.poll() is None:
                if time.monotonic() - start > (1500 if args.mode == 'game' else 60):
                    child.terminate(); child.wait(timeout=30)
                    raise TimeoutError('Bounded caller acquisition deadline')
                if args.mode == 'game':
                    lines = (args.output / 'execution.log').read_text().splitlines()
                    paths = [Path(line.split(': ', 1)[1]) for line in lines if line.startswith('Campaign test evidence: ')]
                    if len(paths) == 1 and (paths[0] / 'display').is_file():
                        number = (paths[0] / 'display').read_text().strip()
                        if number.isdecimal():
                            try: observed = snapshot(':' + number)
                            except (OSError, ValueError):
                                time.sleep(1)
                                continue
                            prior = report['private_window_observations']
                            if not prior or prior[-1]['windows'] != observed['windows']:
                                observed['seconds'] = time.monotonic() - start
                                if len(prior) < 32: prior.append(observed)
                time.sleep(1)
        report.update(exit_code=child.returncode, elapsed_seconds=time.monotonic() - start)
        if args.mode == 'controls' and not child.returncode:
            with (args.output / 'camera-controls.log').open('xb') as log:
                subprocess.run([sys.executable, '-B', str(ROOT/'tests/test-camera-dialogue.py'), '-v'],
                               cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=30)
            flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core','Qt6Gui'],text=True))
            binary=args.output/'capture-policy-test'
            with (args.output / 'capture-policy-build.log').open('xb') as log:
                subprocess.run(['c++','-std=c++17','-O1','-Wall','-Wextra','-Werror','-fPIC','-pthread',
                                '-I'+str(ROOT/'apps/qt-shell'),str(ROOT/'tests/native-image-encoder-test.cpp'),
                                *flags,'-o',str(binary)],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
            with (args.output / 'capture-policy.log').open('xb') as log:
                subprocess.run([str(binary)],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
            report['capture_policy_controls']=json.loads((args.output/'capture-policy.log').read_text().splitlines()[-1])
            report['capture_policy_binary_sha256']=hashlib.sha256(binary.read_bytes()).hexdigest()
            report['camera_python_methods']=13
        if args.mode == 'game':
            lines = (args.output / 'execution.log').read_text().splitlines()
            paths = [Path(line.split(': ', 1)[1]) for line in lines if line.startswith('Campaign test evidence: ')]
            if len(paths) != 1: raise ValueError('Missing unique campaign output')
            campaign_path = paths[0] / 'report.json'
            campaign_raw = campaign_path.read_bytes()
            campaign = json.loads(campaign_raw)
            durable = args.report.with_name(args.report.stem + '-campaign.json')
            write_report(ROOT, durable, campaign_raw)
            report['campaign_record'] = str(durable)
            report['original_game_executed'] = campaign.get('game_entry_verified', False)
            report['campaign_success'] = campaign.get('success', False)
            report['undeclared_campaign_sources'] = [p for p, h in campaign['sources'].items()
                                                      if declaration['sources'].get(p) != h]
            report['campaign_error'] = campaign.get('error')
            if not campaign.get('success'):
                raise ValueError('Original windowed campaign failed: ' + str(campaign.get('error')))
            if report['undeclared_campaign_sources']:
                raise ValueError('Campaign compiler/source closure is undeclared or changed')
            if not campaign.get('original_windowed_requested') or not campaign.get('normal_quit_verified'):
                raise ValueError('Explicit windowed selection and normal original Quit required')
            if campaign.get('private_desktop_screen') != '1280x1024x16':
                raise ValueError('Original windowed 16-bit desktop precondition missing')
            experiment = Path(campaign['experiment'])
            header, rows = decode(experiment / 'lock-capture/copy-routes-v2.bin')
            report['natural_copy'] = validate(rows, campaign['copy_output_closure'],
                                              module_base=header[15], module_extent=header[16])
            report['cpu_readers'] = campaign.get('native_cpu_materialization')
        elif child.returncode:
            raise ValueError('Windowed proof controls failed')
        if child.returncode: raise ValueError('Acquisition failed')
        report['success'] = True
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        report['sources_stable'] = stable()
        report['success'] &= report['sources_stable']
        write_report(ROOT, args.report, (json.dumps(report, indent=2) + '\n').encode())
    print(json.dumps({k: report.get(k) for k in ('success', 'sources_stable', 'error', 'natural_copy')}), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__': raise SystemExit(main())

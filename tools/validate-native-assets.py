#!/usr/bin/env python3
"""Run the complete native asset milestone and Windows file API audit."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixtures-only', action='store_true', help='Build/test without game or original artifacts')
    args = parser.parse_args()
    parent = REPO / 'working/tests/native-assets'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Native asset workflow: {root}', flush=True)
    steps = [('audit-fixtures', [sys.executable, str(REPO / 'tests/test-file-api-audit.py')])]
    if args.fixtures_only:
        build = REPO / 'working/build/audio'
        steps += [('configure', ['cmake', '-S', str(REPO / 'audio'), '-B', str(build)]),
                  ('build', ['cmake', '--build', str(build), '--parallel', '4']),
                  ('fixtures', ['ctest', '--test-dir', str(build), '--output-on-failure'])]
    else:
        steps += [(name, [sys.executable, str(REPO / 'tools' / script)]) for name, script in (
            ('raw-assets', 'test-asset-files.py'), ('decoded-wavs', 'test-audio-buffers.py'), ('file-api-audit', 'audit-file-apis.py'))]
    report = {'origin': 'native_asset_milestone_workflow', 'fixtures_only': args.fixtures_only,
              'live_game_validated': False, 'steps': [], 'complete': False}
    for name, command in steps:
        print(f'Running {name}', flush=True)
        with (root / f'{name}.log').open('w') as log:
            process = subprocess.Popen(command, cwd=REPO, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            evidence = []
            for line in process.stdout:
                log.write(line); log.flush()
                print(line, end='', flush=True)
                for marker in ('Asset evidence: ', 'Audio evidence: ', 'Windows file audit: '):
                    if line.startswith(marker): evidence.append(line[len(marker):].strip())
            code = process.wait()
        report['steps'].append({'name': name, 'command': command, 'exit_code': code,
                                'log': str(root / f'{name}.log'), 'evidence': evidence})
        (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        if code: raise SystemExit(f'{name} failed ({code}); see {root / "report.json"}')
    report['complete'] = True
    (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Native asset workflow passed: {root / "report.json"}')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Build and validate native campaign persistence without original game inputs.

Run through tools/working-retention.py run --reserve-gib 1 -- ...
"""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/native-campaign-persistence'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    (run / '.retain').write_text('Native campaign persistence validation evidence.\n')
    spec = importlib.util.spec_from_file_location('campaign_claims', ROOT / 'tools/draft-coverage-claims.py')
    claims = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(claims)
    declaration = claims.declaration(ROOT, json.loads((ROOT / 'research/runtime/coverage/register.json').read_text()),
        ['NP.campaign-persistence'], ['NP.campaign-persistence.restart'])
    (run / 'claims.json').write_text(json.dumps(declaration, indent=2) + '\n')
    build = run / 'build'
    commands = [
        ['cmake', '-S', str(ROOT / 'game'), '-B', str(build), '-DCMAKE_BUILD_TYPE=Debug'],
        ['cmake', '--build', str(build), '-j4'],
        ['ctest', '--test-dir', str(build), '--output-on-failure'],
    ]
    result = {'schema': 1, 'scope': 'Synthetic native campaign persistence and fresh-process world continuation; no original gameplay or resource rebinding equivalence.',
        'claims': declaration['claims'], 'sources': declaration['sources'], 'commands': [], 'success': False}
    try:
        for index, command in enumerate(commands):
            log = run / f'command-{index}.log'
            with log.open('w') as out:
                completed = subprocess.run(command, cwd=ROOT, stdout=out, stderr=subprocess.STDOUT)
            result['commands'].append({'argv': command, 'exit_code': completed.returncode,
                'log': str(log.relative_to(ROOT)), 'sha256': digest(log)})
            if completed.returncode:
                raise RuntimeError('Validation failed: ' + str(log))
        if any(digest(ROOT / p) != h for p, h in declaration['sources'].items()):
            raise RuntimeError('Declared sources changed during validation')
        result['success'] = True
    except Exception as error:
        result['error'] = str(error)
    (run / 'report.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'success': result['success'], 'report': str((run / 'report.json').relative_to(ROOT)),
        'error': result.get('error')}, indent=2))
    return 0 if result['success'] else 1

if __name__ == '__main__':
    raise SystemExit(main())

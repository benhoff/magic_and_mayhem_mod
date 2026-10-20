#!/usr/bin/env python3
"""Compare the bounded DWORD-load model with the unmodified original leaf.

Run under tools/working-retention.py run --reserve-gib 1 -- ... .
No Wine session, executable patch, or live replacement is involved.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, default=ROOT / 'original/Arcane_Nocd/Chaos.exe')
    parser.add_argument('--output', type=Path, required=True, help='New retained run directory')
    args = parser.parse_args()
    out = args.output.resolve()
    if not out.is_relative_to(ROOT / 'working'):
        parser.error('Output must be a new directory under working/')
    out.mkdir(parents=True, exist_ok=False)
    report = {'success': False, 'cases': [], 'original_sha256': BUILD,
              'scope': 'Resolved logical WORD loads in the selected 0x596490 one-pixel offset-one leaf; '
                       '640x480 and 800x640, last three source WORDs, zero/one/two readable tail bytes. '
                       'No crashed live operand, other leaves, allocation ownership or replacement claim.'}
    sources = {}
    inputs = {}
    before_attempted = False

    def run(label, command, expected=0):
        result = subprocess.run([str(x) for x in command], cwd=ROOT, capture_output=True, timeout=300)
        (out / (label + '.stdout')).write_bytes(result.stdout)
        (out / (label + '.stderr')).write_bytes(result.stderr)
        if expected is not None and result.returncode != expected:
            raise RuntimeError(f'{label}: exit {result.returncode}, expected {expected}')
        return result

    try:
        # Scope declarations and complete linked source set precede all execution.
        run('claims', ['python3', ROOT / 'tools/draft-coverage-claims.py', '--behavior',
                      'RS.displacement-final-word-read', '--output', out / 'claims.json'])
        declaration = json.loads((out / 'claims.json').read_text())
        sources = declaration['sources']
        report.update(sources=sources, claims=declaration['claims'])
        before_attempted = True
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        executable = args.executable.resolve()
        inputs[str(executable)] = sha(executable)
        if inputs[str(executable)] != BUILD:
            raise ValueError('Unexpected original executable hash')
        report['inputs'] = inputs
        run('native-build', ['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                             ROOT / 'tests/distortion-word-read-test.cpp', '-o', out / 'native'])
        run('original-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                               '-fno-pie', '-no-pie', ROOT / 'tests/distortion-word-read-reference.cpp',
                               '-o', out / 'original'])
        native = run('native-controls', [out / 'native'])
        report['native_controls'] = json.loads(native.stdout)['native_controls']
        for width, height in [(640, 480), (800, 640)]:
            logical = width * height * 2
            for distance in range(3):
                for tail in range(3):
                    label = f'{width}x{height}-distance{distance}-tail{tail}'
                    model = json.loads(run(label + '-native', [out / 'native', logical, tail, distance]).stdout)
                    original = run(label + '-original', [out / 'original', executable, width, height, tail, distance], None)
                    output = original.stdout.decode()
                    row = {'width': width, 'height': height, 'distance': distance, 'tail': tail,
                           'model': model, 'original_exit': original.returncode, 'original_output': output,
                           'matching': False}
                    report['cases'].append(row)
                    match = re.search(r'base=([0-9a-f]+) guard=([0-9a-f]+)', output)
                    if not match:
                        raise RuntimeError(label + ': no original address witness')
                    base, guard = (int(v, 16) for v in match.groups())
                    if guard - base != logical + tail:
                        raise RuntimeError(label + ': allocation extent mismatch')
                    if model['readable']:
                        original_value = json.loads(output.splitlines()[-1])
                        row['matching'] = (original.returncode == 0 and original_value['returned'] == 0 and
                                           original_value['pixel_matches'] and
                                           original_value['stored_word'] == model['stored_word'])
                    else:
                        fault = re.search(r'fault=([0-9a-f]{8}) eip=([0-9a-f]{8}) source=([0-9a-f]{8})', output)
                        row['matching'] = bool(original.returncode == 42 and fault and
                            tuple(int(v, 16) for v in fault.groups()) ==
                            (base + model['first_unreadable'], 0x5965f7, base + model['source_byte_offset']))
                    if not row['matching']:
                        raise RuntimeError(label + ': original/model mismatch')
        report['success'] = len(report['cases']) == 18 and all(c['matching'] for c in report['cases'])
    except (OSError, ValueError, KeyError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if before_attempted:
            try:
                run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_after'] = True
            except (OSError, RuntimeError, subprocess.SubprocessError) as error:
                report['manifest_error'] = str(error)
                report['success'] = False
        report['sources_stable'] = bool(sources) and all(sha(ROOT / p) == h for p, h in sources.items())
        report['inputs_stable'] = bool(inputs) and all(sha(Path(p)) == h for p, h in inputs.items())
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        report['binaries'] = {p.name: sha(p) for p in (out / 'native', out / 'original') if p.exists()}
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'success': report['success'], 'cases': len(report['cases']), 'report': str(out / 'report.json')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())

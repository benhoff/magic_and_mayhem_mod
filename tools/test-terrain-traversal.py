#!/usr/bin/env python3
"""Compare the bounded four-view traversal with the pinned original PE."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD_HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    exe, out = args.executable.resolve(), args.output.resolve()
    if sha(exe) != BUILD_HASH:
        raise ValueError('Unsupported original executable')
    out.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('draft', ROOT / 'tools/draft-coverage-claims.py')
    draft = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(draft)
    declaration = draft.declaration(ROOT, json.loads((ROOT / 'research/runtime/coverage/register.json').read_text()),
                                    ['RE.terrain_traversal'], [])
    names = ['terrain_traversal', 'terrain_submission', 'sprite_queue']
    for name in names:
        for suffix in ('.cpp', '.hpp'):
            path = 'reconstruction/rendering/' + name + suffix
            declaration['sources'][path] = sha(ROOT / path)
    for path in ('tools/test-terrain-traversal.py', 'tools/draft-coverage-claims.py',
                 'tools/coverage_claims.py'):
        declaration['sources'][path] = sha(ROOT / path)
    (out / 'declaration.json').write_text(json.dumps(declaration, indent=2) + '\n')
    report = dict(success=False, sources=declaration['sources'], claims=declaration['claims'],
                  executable_sha256=BUILD_HASH, commands=[], original_manifest_verified_before_after=False,
                  scope='Existing isolated four-view traversal/ordinary producer domain, exact records and cell mutations. '
                        'Unsafe native-domain inputs are refused before executing original dereferences. '
                        'No full World rendering, original allocation, entity production or live replacement.')

    def run(label, command, **kwargs):
        command = list(map(str, command))
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=600, **kwargs)
        (out / (label + '.stdout')).write_text(result.stdout)
        (out / (label + '.stderr')).write_text(result.stderr)
        report['commands'].append(dict(argv=command, exit_code=result.returncode))
        result.check_returncode()
        if 'runtime error:' in result.stderr or 'ERROR: AddressSanitizer' in result.stderr:
            raise RuntimeError('Sanitizer diagnostic')
        return result.stdout

    run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
    try:
        model = [ROOT / 'reconstruction/rendering' / (name + '.cpp') for name in names]
        common = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                  '-I' + str(ROOT / 'reconstruction/rendering')]
        reference, unit = out / 'reference', out / 'sanitized-unit'
        run('compile-reference', [*common, '-m32', ROOT / 'tests/terrain-traversal-reference.cpp',
                                  *model, '-o', reference])
        report['comparison'] = json.loads(run('comparison', [reference, exe]))
        assert report['comparison']['all_match'] is True
        assert report['comparison']['cases'] == 18428
        assert len(report['comparison']['orientations']) == 4
        run('compile-sanitized', [*common, '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                                  '-fno-pie', '-no-pie', ROOT / 'tests/terrain-traversal-test.cpp',
                                  *model, '-o', unit])
        run('sanitized-unit', [unit], env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',
                                              UBSAN_OPTIONS='halt_on_error=1'))
        report['sanitizers_passed'] = True
        report['binaries'] = {p.name: sha(p) for p in (reference, unit)}
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in report['sources'].items())
        report['input_stable'] = sha(exe) == BUILD_HASH
        assert report['sources_stable'] and report['input_stable']
        report['success'] = True
    finally:
        try:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        finally:
            (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(success=report['success'], comparison=report['comparison'], output=str(out))))


if __name__ == '__main__':
    main()

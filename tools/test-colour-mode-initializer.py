#!/usr/bin/env python3
"""Pin original547950 execution for its documented inventory-range exception."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_evidence import write_report
ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', type=Path, required=True)
    args = p.parse_args()
    args.report = args.report.absolute()
    if args.report.exists():
        p.error('Refusing evidence overwrite')
    parent = ROOT/'working/tests/colour-mode-initializer'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    names = ('tools/test-colour-mode-initializer.py', 'tests/colour-mode-initializer-reference.cpp',
             'tests/sprite-binary-reference.cpp', 'tools/export-menu-support.py', 'tools/original-manifest.sh')
    sha = lambda f: hashlib.sha256(f.read_bytes()).hexdigest()
    sources = {n:sha(ROOT/n) for n in names}
    report = dict(schema=1, success=False, sources=sources, commands=[], original_sha256=BUILD,
                  experiment=str(out.relative_to(ROOT)), original_entry_anchors={},
                  native_implementation_executed=False, actual_game_journey=False)

    def run(label, command):
        result = subprocess.run(list(map(str, command)), cwd=ROOT, capture_output=True, text=True, timeout=180)
        (out/(label+'.log')).write_text(result.stdout+result.stderr)
        report['commands'].append(dict(label=label, returncode=result.returncode))
        if result.returncode:
            raise ValueError(label+': '+result.stderr[-2000:])
        return result.stdout

    before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        before = True
        original = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original) != BUILD:
            raise ValueError('Unsupported original build')
        spec = importlib.util.spec_from_file_location('initializer_pe', ROOT/'tools/export-menu-support.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        data = original.read_bytes()
        expected = bytes.fromhex('8b4424048b542414')
        at = module.image_offset(data, 0x547950, len(expected))
        if data[at:at+len(expected)] != expected:
            raise ValueError('Original initializer entry bytes differ')
        report['original_entry_anchors']['0x547950'] = expected.hex()
        run('build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                      '-fno-pie', '-no-pie', ROOT/'tests/colour-mode-initializer-reference.cpp', '-o', out/'reference'])
        report['original_execution'] = json.loads(run('initializer', [out/'reference', original]).strip())
        report['binary_sha256'] = sha(out/'reference')
        report['success'] = report['original_execution']['success'] and report['original_execution']['cases']==18
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = before
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/n)==h for n,h in sources.items())
        report['success'] = report['success'] and report['sources_stable']
        write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],error=report.get('error'),record=str(args.report))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())

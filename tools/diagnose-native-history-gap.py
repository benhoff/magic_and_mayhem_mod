#!/usr/bin/env python3
"""Compare a refused first native history canvas with original replay from zero."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
from importlib.util import spec_from_file_location, module_from_spec

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def differences(a, b):
    if len(a) != len(b) or len(a) % 2:
        raise ValueError('Pixel extent differs')
    left, right = struct.iter_unpack('<H', a), struct.iter_unpack('<H', b)
    rows = [(i, x[0], y[0]) for i, (x, y) in enumerate(zip(left, right)) if x != y]
    return {'mismatches': len(rows), 'samples': rows[:32]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--experiment', type=Path, required=True)
    args = parser.parse_args()
    parent = ROOT/'working/tests/world-live-history-gap'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    source_paths = ['tools/diagnose-native-history-gap.py', 'tests/world-frame-reference.cpp',
                    'tests/sprite-binary-reference.cpp', 'tools/test-world-frames.py', 'protocols/include/mnm/world_frame_v1.h']
    sources = {p: sha(ROOT/p) for p in source_paths}
    files = [args.experiment/'world-1.bin', args.experiment/'world-1.native.565', args.experiment/'world-1.original.565']
    inputs = {str(p): sha(p) for p in files}
    pe = ROOT/'working/game-nocd/Chaos.exe'
    report = {'success': False, 'sources': sources, 'inputs': inputs,
              'source_executable_sha256': HASH,
              'scope': 'Diagnostic first-canvas original replay from native zero versus retained native output and actual original output; no initialization reconstruction or whole-prefix equivalence.'}
    subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    try:
        if sha(pe) != HASH:
            raise RuntimeError('Unsupported original executable')
        spec = spec_from_file_location('world_frames', ROOT/'tools/test-world-frames.py')
        helper = module_from_spec(spec);spec.loader.exec_module(helper)
        report['entry_anchors'] = helper.reference_anchor(pe)
        reference = out/'reference'
        subprocess.run(['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-fno-pie', '-no-pie', ROOT/'tests/world-frame-reference.cpp', '-o', reference], check=True)
        original = out/'original-zero.565'
        subprocess.run([reference, pe, files[0], original], check=True)
        native, live, zero = files[1].read_bytes(), files[2].read_bytes(), original.read_bytes()
        report.update(native_vs_original_zero=differences(native, zero), original_zero_vs_live=differences(zero, live),
                      native_vs_live=differences(native, live), original_zero_sha256=sha(original), reference_sha256=sha(reference))
        if {p:sha(ROOT/p) for p in source_paths} != sources or {str(p):sha(p) for p in files} != inputs:
            raise RuntimeError('Diagnostic sources or inputs changed')
        report.update(success=True, sources_stable=True, original_pixels_used_as_native_inputs=False)
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        report['original_manifest_verified_before_after'] = True
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Capture parser, OpenGL CLI, and three-way comparison integration (use Xvfb)."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location('capture_tests', REPO/'tests/test-render-capture.py')
fixtures = importlib.util.module_from_spec(spec); spec.loader.exec_module(fixtures)
replay = fixtures.replay


def main():
    executable = Path(sys.argv[1]).resolve()
    parent = REPO/'working/tests/render-opengl'; parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    results = []

    def run(name, data, valid=True):
        source, output = root/(name+'.bin'), root/(name+'-native.bin')
        preview = root/(name+'.png')
        source.write_bytes(data)
        result = subprocess.run([str(executable), str(source), '--output', str(output), '--preview', str(preview)],
                                capture_output=True, text=True, timeout=20)
        (root/(name+'.log')).write_text(result.stderr)
        report = json.loads(result.stdout)
        assert result.returncode == (0 if valid else 2), (name, report)
        assert report['rendered'] is valid
        if valid:
            expected = replay.replay(replay.decode(data))
            assert output.read_bytes() == expected, name
            assert report['capture_sha256'] == hashlib.sha256(data).hexdigest()
            assert report['output_sha256'] == hashlib.sha256(expected).hexdigest()
            rgb = replay.ppm(replay.decode(data), expected).split(b'\n', 3)[3]
            rgba = b''.join(rgb[i:i+3]+b'\xff' for i in range(0, len(rgb), 3))
            assert report['presentation_rgba_sha256'] == hashlib.sha256(rgba).hexdigest()
            assert preview.read_bytes().startswith(b'\x89PNG\r\n\x1a\n')
            assert report['surface_stats']['uploads'] == 2 and report['surface_stats']['copies'] == 1
            assert report['surface_stats']['presentations'] == 1
            results.append(report)
        else:
            assert not output.exists() and not preview.exists(), name
        return source, output

    for bits in (8, 16, 24, 32):
        for keyed in (False, True):
            run(f'copy-{bits}-{keyed}', fixtures.fixture(bits, keyed, fast=keyed))
    # PALETTEENTRY flags are not an alpha channel. Presentation stays opaque.
    flagged = bytearray(fixtures.fixture(8));c = replay.decode(flagged)
    palette_start = 128+len(c['source'])+2*len(c['before'])+1024
    for entry in range(256):flagged[palette_start+entry*4+3]=entry
    run('palette-flags', flagged)
    # GPU inputs cannot include the captured output; poison it deliberately.
    data = bytearray(fixtures.fixture(32)); data[128+4*4] ^= 0x80  # Different before image.
    c = replay.decode(data); after_offset = 128+len(c['source'])+len(c['before'])
    data[after_offset:] = bytes([0xa5])*(len(data)-after_offset)
    source, output = run('poisoned-after', data)
    again = subprocess.run([str(executable), str(source), '--output', str(output)], capture_output=True, text=True)
    assert again.returncode == 2 and output.read_bytes() == replay.replay(replay.decode(data))
    # The Python front-end must report a mismatch, not promote CPU agreement to
    # agreement with original output, and must record both comparison results.
    result = subprocess.run([sys.executable, str(REPO/'tools/replay-render-capture.py'), str(source),
                             '--backend', 'opengl', '--gl-executable', str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 1, result.stdout+result.stderr
    report_path = next(line.removeprefix('Report: ') for line in result.stdout.splitlines() if line.startswith('Report: '))
    report = json.loads(Path(report_path).read_text())
    assert not report['comparison']['matching'] and not report['cpu_comparison']['matching']
    assert report['opengl_vs_cpu']['matching']
    # An unavailable/failed GL backend must remain inconclusive, even though the
    # CPU reference itself succeeded. Never silently fall back to CPU rendering.
    result = subprocess.run([sys.executable, str(REPO/'tools/replay-render-capture.py'), str(root/'copy-16-False.bin'),
                             '--backend', 'opengl', '--gl-executable', str(root/'missing-renderer')],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 2 and 'Inconclusive' in result.stdout
    malformed = [b'', fixtures.fixture()[:127], fixtures.fixture()[:-1], fixtures.fixture()+b'\0']
    for offset, value in ((8, 2), (12, 9), (20, 0x200), (24, 0), (28, 3), (32, 2),
                          (36, 0x80000000), (40, 0xffffffff), (48, 1), (72, 257), (80, 2049),
                          (88, 15), (92, 0xf801), (96, 0xf800), (104, 1), (112, 1024), (124, 1)):
        bad = bytearray(fixtures.fixture()); struct.pack_into('<I', bad, offset, value); malformed.append(bad)
    for index, bad in enumerate(malformed):
        try: replay.decode(bad)
        except ValueError: pass
        else: raise AssertionError('Python accepted malformed fixture')
        run(f'reject-{index}', bad, valid=False)
    (root/'report.json').write_text(json.dumps({'origin': 'synthetic_opengl_integer', 'captures': results,
        'rejected': len(malformed), 'poisoned_output_detected': True, 'overwrite_rejected': True,
        'missing_backend_inconclusive': True}, indent=2)+'\n')
    print(f'OpenGL capture integration passed: {root}')


if __name__ == '__main__': main()

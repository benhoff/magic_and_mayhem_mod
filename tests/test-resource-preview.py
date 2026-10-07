#!/usr/bin/env python3
"""CLI admission/output checks using an independently encoded tiny BMP."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


def main():
    binary = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix='mnm-resource-preview-') as directory:
        root = Path(directory) / 'assets'
        root.mkdir()
        # One bottom-up row, two BGR pixels + DWORD padding: black, green.
        bmp = b'BM' + struct.pack('<IHHI', 62, 0, 0, 54)
        bmp += struct.pack('<IiiHHIIiiII', 40, 2, 1, 1, 24, 0, 8, 0, 0, 0, 0)
        bmp += bytes([0, 0, 0, 0, 255, 0, 0, 0])
        source = root / 'image.bmp'
        source.write_bytes(bmp)
        output = Path(directory) / 'preview.png'
        common = [binary, '--root', str(root), '--id', 'menu/title', '--image',
                  'image.bmp', '--format', 'bmp']

        def run(args, success):
            result = subprocess.run(args, capture_output=True, text=True, timeout=20)
            assert (result.returncode == 0) == success, result.stderr
            return result

        result = json.loads(run(common + ['--output', str(output)], True).stdout)
        assert result == {'ok': True, 'id': 'ui:menu/title', 'frame': 0,
                          'decoded_loads': 1, 'surfaces': 0, 'decoded_bytes': 0}
        png = output.read_bytes()
        assert png.startswith(b'\x89PNG\r\n\x1a\n')
        assert struct.unpack('>II', png[16:24]) == (4, 3)
        run(common + ['--output', str(output)], False)
        assert output.read_bytes() == png
        run(common + ['--output', str(root/'new.png')], False)
        assert not (root/'new.png').exists()
        run(common + ['--output', str(Path(directory)/'bad.png'), '--frame', '1'], False)
        run(common + ['--output', str(Path(directory)/'bad.png'), '--frame', '-1'], False)
        run(common + ['--output', str(Path(directory)/'bad.png'), '--kind', 'creature'], False)
        run(common + ['--output', str(Path(directory)/'bad.png'), '--id', 'Upper'], False)
        assert not (Path(directory)/'bad.png').exists()
        assert source.read_bytes() == bmp
    print('Resource preview recipe/PNG, input preservation, existing/root output and invalid selections pass')


if __name__ == '__main__':
    main()

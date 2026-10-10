#!/usr/bin/env python3
"""Exercise Qt tail-reader refusal before any records or asset inputs are applied."""
import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('consumer', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    cases = [('v1-overbound', 1, 128*1024*1024+1, None, 'version bound'),
             ('v2-overbound', 2, 512*1024*1024+1, None, 'version bound'),
             ('wrong-version', 2, 64, 8, 'envelope'),
             ('header-reserve', 2, 64, 28, 'reserve'),
             ('unfinished-record', 2, 100, None, 'Incomplete final')]
    rows = []
    for name, version, size, patch, expected in cases:
        with tempfile.TemporaryDirectory(prefix='mnm-tail-') as directory:
            root = Path(directory)
            game = root/'game'
            game.mkdir()
            header = bytearray(64)
            header[:8] = b'MNMPRO01' if version == 1 else b'MNMPRO02'
            struct.pack_into('<5I', header, 8, version, 64, 0x40209ca7,
                             65536 if version == 1 else 262144, 1)
            if patch:
                struct.pack_into('<I', header, patch, 99)
            with (root/'canvas-producers.bin').open('wb') as stream:
                stream.write(header)
                stream.truncate(size)
            (root/'canvas-producers.done').write_bytes(b'MNMPDONE'+bytes(24))
            completed = subprocess.run(['xvfb-run', '-a', str(args.consumer.resolve()),
                str(root/'canvas-producers.bin'), str(game), str(root/'native')],
                env={**os.environ, 'LIBGL_ALWAYS_SOFTWARE':'1', 'QT_QPA_PLATFORM':'xcb'},
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
            report = json.loads((root/'native/live-report.json').read_text())
            if completed.returncode != 1 or report['success'] or report['records'] or expected not in report['error']:
                raise RuntimeError('Missing expected tail refusal: '+name)
            rows.append(dict(case=name, error=report['error'], records=report['records']))
    (args.output/'report.json').write_text(json.dumps(dict(success=True, cases=rows), indent=2)+'\n')
    print(args.output/'report.json')

if __name__ == '__main__':
    main()

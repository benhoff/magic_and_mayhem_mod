#!/usr/bin/env python3
"""Native WAV adapter fixtures, independent PCM expectations and input limits."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import wave


def main():
    executable = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix='mnm-audio-input-') as temporary:
        base = Path(temporary)
        root = base / 'install'
        (root / 'Sounds' / 'Nested').mkdir(parents=True)
        pcm = bytes(range(256)) * 600
        path = root / 'Sounds' / 'Nested' / 'Tone.wav'
        with wave.open(str(path), 'wb') as wav:
            wav.setnchannels(2)
            wav.setsampwidth(2)
            wav.setframerate(22050)
            wav.writeframes(pcm)
        original = path.read_bytes()
        dump = base / 'pcm.bin'
        requests = [
            [str(path)],
            ['--assets', str(root), '--path', 'sOuNdS\\nEsTeD\\TONE.WAV'],
            ['--assets', str(root), '--prefix', 'C:/Game', '--path', 'c:\\gAmE\\SOUNDS\\Nested\\tone.wav'],
        ]
        for arguments in requests:
            completed = subprocess.run([str(executable), *arguments, '--dump', str(dump)], capture_output=True, text=True)
            assert completed.returncode == 0, completed.stderr
            report = json.loads(completed.stdout)
            assert report['channels'] == 2 and report['bits'] == 16 and report['rate'] == 22050
            assert report['samples'] == len(pcm) and report['alignment'] == 4
            assert report['revision'] == 1 and report['remaining_buffers'] == 0
            assert not report['audible_output'] and dump.read_bytes() == pcm
            assert path.read_bytes() == original

        sentinel = b'output must survive rejected input'
        dump.write_bytes(sentinel)
        (root / 'Sounds' / 'Invalid.wav').write_bytes(b'not WAV')
        with (root / 'Sounds' / 'Oversized.wav').open('wb') as output:
            output.truncate(32 * 1024 * 1024 + 1)
        # Valid RIFF with a data payload beyond the existing Device sample cap.
        with wave.open(str(root / 'Sounds' / 'TooMuchPcm.wav'), 'wb') as wav:
            wav.setnchannels(1)
            wav.setsampwidth(1)
            wav.setframerate(22050)
            wav.writeframes(b'\x80' * (16 * 1024 * 1024 + 2))
        failures = [
            ('Sounds/Missing.wav', 'No matching component'),
            ('Sounds/Invalid.wav', 'Not RIFF/WAVE'),
            ('Sounds/Oversized.wav', 'File exceeds size or allocation limit'),
            ('Sounds/TooMuchPcm.wav', 'Native sample upload failed'),
            ('../outside.wav', 'Parent traversal'),
        ]
        for request, message in failures:
            completed = subprocess.run([str(executable), '--assets', str(root), '--path', request,
                '--dump', str(dump)], capture_output=True, text=True)
            assert completed.returncode == 1 and message in completed.stderr, completed.stderr
            assert dump.read_bytes() == sentinel
        for arguments in (['--assets', str(root)], ['--path', 'Sounds/x'],
                          [str(path), '--assets', str(root), '--path', 'Sounds/x'],
                          [str(path), '--prefix', 'C:/Game']):
            completed = subprocess.run([str(executable), *arguments, '--dump', str(dump)], capture_output=True, text=True)
            assert completed.returncode == 2
        completed = subprocess.run([str(executable), '--assets', str(root), '--path', 'Sounds/Nested/Tone.wav',
            '--dump', str(path)], capture_output=True, text=True)
        assert completed.returncode == 1 and path.read_bytes() == original
        completed = subprocess.run([str(executable), '--help'], capture_output=True, text=True)
        assert completed.returncode == 0 and '--assets' in completed.stdout
    print('WAV asset adapter, exact PCM ownership, input/sample limits and CLI errors passed')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Fixture-only checks of comparison results, hashes, offsets and failures."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix='mnm-asset-compare-') as temporary:
        base = Path(temporary)
        root = base / 'install'
        refs = base / 'references'
        (root / 'Sounds').mkdir(parents=True)
        refs.mkdir()
        data = bytes(range(256)) * 400 + b'\0\r\n\xff'
        cases = {
            'Bytes.bin': (data, data),
            'Empty.bin': (b'', b''),
            'Changed.bin': (data[:70000] + b'!' + data[70001:], data),
            'Long.bin': (data + b'extra', data),
            'Short.bin': (data[:40000], data),
            'Late.bin': (data[:10] + b'?' + data[11:90000] + b'!' + data[90001:], data),
        }
        manifest = base / 'manifest.json'
        entries = []
        for name, (actual, expected) in cases.items():
            (root / 'Sounds' / name).write_bytes(actual)
            (refs / name).write_bytes(expected)
            entries.append({'path': 'c:\\gAmE\\sOuNdS\\' + name.swapcase(), 'reference': str(refs / name)})
        manifest.write_text(json.dumps(entries))
        before = {p: p.read_bytes() for p in root.rglob('*') if p.is_file()}
        command = [str(executable), '--root', str(root), '--prefix', 'C:/Game', '--manifest', str(manifest)]
        completed = subprocess.run(command, capture_output=True, text=True)
        assert completed.returncode == 1, completed.stderr
        report = json.loads(completed.stdout)
        assert (report['equal'], report['different'], report['errors'], report['changed']) == (2, 4, 0, 0)
        offsets = {'Changed.bin': 70000, 'Long.bin': len(data), 'Short.bin': 40000, 'Late.bin': 10}
        for row, (name, (actual, expected)) in zip(report['assets'], cases.items(), strict=True):
            assert row['size'] == len(actual) and row['reference_size'] == len(expected)
            assert row['inputs_unchanged'] and row['sequential_complete'] and row['seek_complete']
            assert row['interface_matches_source']
            assert row['first_differing_offset'] == offsets.get(name)
            assert row['seek_equal'] == (actual == expected)
            assert row['seek_reads'] >= (len(actual) + 32767) // 32768
            if len(actual) == len(expected):
                assert row['seek_reads'] == (len(actual) + 32767) // 32768
            actual_hash = hashlib.sha256(actual).hexdigest()
            expected_hash = hashlib.sha256(expected).hexdigest()
            assert row['sequential_sha256'] == row['source_sha256_before'] == row['source_sha256_after'] == actual_hash
            assert row['reference_sha256_before'] == row['reference_sha256_after'] == expected_hash
        assert all(p.read_bytes() == data_before for p, data_before in before.items())
        output = base / 'report.json'
        completed = subprocess.run(command + ['--report', str(output)], capture_output=True, text=True)
        assert completed.returncode == 1 and not completed.stdout
        assert json.loads(output.read_text()) == report

        entries = [
            {'path': 'Sounds/Missing.bin', 'reference': str(refs / 'Bytes.bin')},
            {'path': 'Sounds/Bytes.bin', 'reference': str(refs / 'Missing.bin')},
            {'path': '../outside', 'reference': str(refs / 'Bytes.bin')},
        ]
        manifest.write_text(json.dumps(entries))
        completed = subprocess.run(command, capture_output=True, text=True)
        assert completed.returncode == 2
        rows = json.loads(completed.stdout)['assets']
        assert [r['error']['category'] for r in rows] == ['notFound', 'notFound', 'invalidPath']
        assert rows[0]['error']['requested_path'] == 'Sounds/Missing.bin'
        assert all(r['status'] == 'error' for r in rows)

        # Exact spelling must still report a case-fold collision on Linux.
        (root / 'Sounds' / 'bytes.bin').write_bytes(b'collision')
        if not (root / 'Sounds' / 'bytes.bin').samefile(root / 'Sounds' / 'Bytes.bin'):
            manifest.write_text(json.dumps([{'path': 'Sounds/Bytes.bin', 'reference': str(refs / 'Bytes.bin')}]))
            completed = subprocess.run(command, capture_output=True, text=True)
            assert completed.returncode == 2
            assert json.loads(completed.stdout)['assets'][0]['error']['category'] == 'ambiguousPath'
        (root / 'Sounds' / 'bytes.bin').unlink()

        for content in ('[]', 'not JSON', '[{"path":"x"}]', '[{"path":"x","reference":"relative"}]'):
            manifest.write_text(content)
            completed = subprocess.run(command, capture_output=True, text=True)
            assert completed.returncode == 2 and completed.stderr
        manifest.write_text(json.dumps([{'path': 'Sounds/Empty.bin', 'reference': str(refs / 'Empty.bin')}]))
        completed = subprocess.run(command, capture_output=True, text=True)
        assert completed.returncode == 0 and json.loads(completed.stdout)['equal'] == 1
        for forbidden in (root / 'Sounds/Empty.bin', refs / 'Empty.bin', manifest):
            previous = forbidden.read_bytes()
            completed = subprocess.run(command + ['--report', str(forbidden)], capture_output=True, text=True)
            assert completed.returncode == 2 and completed.stderr
            assert forbidden.read_bytes() == previous
    print('Raw comparison hashes, seek coverage, difference offsets and errors passed')


if __name__ == '__main__':
    main()

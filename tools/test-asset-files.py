#!/usr/bin/env python3
"""Compare installed loose files through Qt against independent reference reads."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]


def inventory(root):
    if not root.is_dir():
        raise ValueError(f'Installation directory missing: {root}')
    files = {}
    for path in sorted(root.rglob('*')):
        if path.is_symlink():
            raise ValueError(f'Installed inventory requires ordinary files/directories: {path}')
        if path.is_file():
            relative = path.relative_to(root).as_posix()
            relative.encode('ascii')
            digest = hashlib.sha256()
            with path.open('rb') as stream:
                for chunk in iter(lambda: stream.read(65536), b''):
                    digest.update(chunk)
            files[relative] = {'size': path.stat().st_size, 'sha256': digest.hexdigest()}
    if not files:
        raise ValueError(f'No installed files: {root}')
    return files


def mixed(path):
    return ''.join(c.upper() if i % 2 else c.lower() for i, c in enumerate(path)).replace('/', '\\')


def verify_original(evidence, phase):
    completed = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True)
    (evidence / f'original-{phase}.log').write_text(completed.stdout + completed.stderr)
    print(completed.stdout, end='', flush=True)
    completed.check_returncode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=REPO / 'working/game-nocd')
    parser.add_argument('--reference-root', type=Path, help='Separate reference tree; default is the same installed files')
    args = parser.parse_args()
    root = args.root.resolve()
    reference = (args.reference_root or root).resolve()
    parent = REPO / 'working/tests/asset-files'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Asset evidence: {evidence}', flush=True)
    verify_original(evidence, 'before')
    try:
        build = REPO / 'working/build/assets'
        subprocess.run(['cmake', '-S', str(REPO / 'assets'), '-B', str(build)], check=True)
        subprocess.run(['cmake', '--build', str(build), '--parallel', '4'], check=True)
        subprocess.run(['ctest', '--test-dir', str(build), '--output-on-failure'], check=True)
        before = inventory(root)
        reference_before = before if reference == root else inventory(reference)
        names = sorted(before.keys() | reference_before.keys())
        entries = [{'path': ('c:\\MagicMayhem\\' if i % 2 else '') + mixed(name),
                    'reference': str(reference / name)} for i, name in enumerate(names)]
        manifest = evidence / 'manifest.json'
        manifest.write_text(json.dumps(entries, indent=2) + '\n')
        completed = subprocess.run([str(build / 'mnm-asset-compare'), '--root', str(root),
            '--prefix', 'C:/MagicMayhem', '--manifest', str(manifest), '--report', str(evidence / 'comparison.json')],
            capture_output=True, text=True)
        (evidence / 'comparison.stderr').write_text(completed.stderr)
        after = inventory(root)
        reference_after = after if reference == root else inventory(reference)
        unchanged = before == after and reference_before == reference_after
        report = {'origin': 'installed_raw_asset_interface_validation', 'root': str(root),
            'reference_root': str(reference), 'same_tree_reference': root == reference,
            'file_count': len(names), 'bytes': sum(v['size'] for v in before.values()),
            'inputs_unchanged': unchanged, 'comparison_exit_code': completed.returncode,
            'comparison_binary_sha256': hashlib.sha256((build / 'mnm-asset-compare').read_bytes()).hexdigest(),
            'source_inventory': before, 'reference_inventory': reference_before,
            'live_game_validated': False, 'decoded_output_compared': False,
            'checks': ['independent_binary_reads', 'sequential_all_bytes', 'reverse_seek_all_bytes',
                       'mixed_case_windows_paths', 'explicit_drive_aliases', 'sha256_before_after']}
        (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        if not unchanged:
            raise ValueError(f'Inputs changed during comparison; see {evidence}')
        if completed.returncode:
            raise ValueError(f'Asset comparison failed ({completed.returncode}); see {evidence}')
        result = json.loads((evidence / 'comparison.json').read_text())
        if result['equal'] != len(names) or result['errors'] or result['changed'] or result['different']:
            raise ValueError('Incomplete or failing comparison report')
        print(f'Raw asset comparison passed: {len(names)} files, {report["bytes"]} bytes; {evidence / "report.json"}')
    finally:
        verify_original(evidence, 'after')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare per-file and persistent evidence filters in an isolated Git fixture.

This measures filter overhead, not complete commit/merge latency. Both modes
must stage identical trees and check out exact pointer bytes. Results go through
the SQLite report writer; no repository Git settings are changed.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess
import tempfile
import time

from coverage_evidence import install_filter, pointer, write_report

ROOT = Path(__file__).resolve().parents[1]


def benchmark(files=256, runs=3):
    with tempfile.TemporaryDirectory(prefix='evidence-filter-benchmark-') as temporary:
        root = Path(temporary)
        env = dict(os.environ, GIT_CONFIG_NOSYSTEM='1', GIT_CONFIG_GLOBAL=os.devnull)
        # Real Git exercises the installed protocol. Env overrides isolate the
        # fixture from the caller's pending evidence and optional index file.
        env.pop('GIT_INDEX_FILE', None)
        env.pop('GIT_DIR', None)
        env.pop('GIT_WORK_TREE', None)

        def git(*args):
            return subprocess.check_output(['git', '-C', str(root), *args], env=env,
                                           stderr=subprocess.PIPE)

        git('init', '-q')
        install_filter(root, ROOT/'tools/evidence-store.py')
        process_command = git('config', 'filter.mm-evidence.process').decode().strip()
        (root/'.gitattributes').write_text('research/runtime/** filter=mm-evidence\n')
        directory = root/'research/runtime'; directory.mkdir(parents=True)
        # Pointer cleaning, the common migrated-repository path, requires no
        # evidence resolution. Hash these deterministic fixture bytes exactly.
        data = pointer(hashlib.sha256(b'benchmark payload').hexdigest(), 17)
        for number in range(files):
            (directory/f'result-{number}.json').write_bytes(data)
        git('add', '.gitattributes', 'research')
        results = {mode: {'stage_seconds': [], 'checkout_seconds': []}
                   for mode in ('single_file', 'persistent')}
        trees = set()
        for run in range(runs):
            # Alternate order to reduce systematic warm-cache bias.
            order = ('single_file', 'persistent') if run % 2 == 0 else ('persistent', 'single_file')
            for mode in order:
                if mode == 'single_file':
                    git('config', '--unset', 'filter.mm-evidence.process')
                else:
                    git('config', 'filter.mm-evidence.process', process_command)
                started = time.perf_counter()
                git('add', '--renormalize', '--', 'research')
                results[mode]['stage_seconds'].append(time.perf_counter()-started)
                trees.add(git('write-tree').decode().strip())
                output = root/f'checkout-{run}-{mode}'
                started = time.perf_counter()
                git('checkout-index', '--all', '--prefix='+str(output)+'/')
                results[mode]['checkout_seconds'].append(time.perf_counter()-started)
                for number in range(files):
                    if (output/'research/runtime'/f'result-{number}.json').read_bytes() != data:
                        raise ValueError('Checkout changed evidence pointer bytes')
                # Leave the process option defined for the next iteration.
                git('config', 'filter.mm-evidence.process', process_command)
        if len(trees) != 1:
            raise ValueError('Filter modes staged different trees')
        for values in results.values():
            values['stage_median_seconds'] = statistics.median(values['stage_seconds'])
            values['checkout_median_seconds'] = statistics.median(values['checkout_seconds'])
        return {'schema': 1, 'success': True, 'scope': __doc__, 'files': files, 'runs': runs,
                'git_version': git('--version').decode().strip(), 'results': results,
                'identical_staged_trees': True, 'exact_checkout_pointers': True,
                'sources': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                            for p in (Path(__file__).resolve(), ROOT/'tools/evidence_filter.py',
                                      ROOT/'tools/coverage_evidence.py', ROOT/'tools/evidence-store.py')}}


def benchmark_staged(root):
    """Time the same actual index with configured filters and identity bypass."""
    root = root.resolve()
    git = ['git', '-C', str(root)]
    entries = []
    for row in subprocess.check_output(git+['ls-files', '--stage', '-z']).split(b'\0'):
        if not row:
            continue
        metadata, name = row.split(b'\t', 1)
        mode, oid, stage = metadata.split()
        path = Path(os.fsdecode(name))
        if path.is_absolute() or '..' in path.parts or stage != b'0':
            raise ValueError('Unsafe or unmerged index entry')
        if path.parts[0] in {'original', 'working', '.git'}:
            continue
        if mode not in {b'100644', b'100755'}:
            raise ValueError('Unsupported index file mode')
        entries.append((name, oid))
    algorithm = subprocess.check_output(git+['rev-parse', '--show-object-format']).decode().strip()
    result = {'root': str(root), 'files': len(entries), 'seconds': {}, 'exact_blobs': {}}
    for mode in ('configured', 'bypass'):
        with tempfile.TemporaryDirectory(prefix='staged-filter-benchmark-') as directory:
            options = [] if mode == 'configured' else [
                '-c', 'filter.mm-evidence.process=', '-c', 'filter.mm-evidence.smudge=',
                '-c', 'filter.mm-evidence.required=false']
            started = time.perf_counter()
            subprocess.run(git+options+['checkout-index', '--prefix='+directory+'/', '-z', '--stdin'],
                           input=b'\0'.join(name for name, _ in entries)+b'\0' if entries else b'',
                           check=True, capture_output=True)
            result['seconds'][mode] = time.perf_counter()-started
            for name, oid in entries:
                path = Path(directory)/os.fsdecode(name)
                sha = hashlib.new(algorithm)
                sha.update(b'blob '+str(path.stat().st_size).encode()+b'\0')
                with path.open('rb') as data:
                    for chunk in iter(lambda: data.read(1024*1024), b''):
                        sha.update(chunk)
                if sha.hexdigest().encode() != oid:
                    raise ValueError('Checkout changed index blob: '+os.fsdecode(name))
            result['exact_blobs'][mode] = True
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--files', type=int, default=256)
    parser.add_argument('--runs', type=int, default=3)
    parser.add_argument('--staged-root', type=Path,
                        help='Also compare materialization of this real index (read-only)')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if not 1 <= args.files <= 4096 or not 1 <= args.runs <= 10:
        parser.error('Use 1..4096 files and 1..10 runs')
    if args.output.exists():
        parser.error('Refusing to overwrite an existing result')
    result = benchmark(args.files, args.runs)
    if args.staged_root:
        result['staged_materialization'] = benchmark_staged(args.staged_root)
    write_report(ROOT, args.output, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()

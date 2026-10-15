#!/usr/bin/env python3
"""Execute the unchanged pinned mouse writer with owned ring and Alt responses."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        declaration = json.loads(args.claims.read_text())
        sources = declaration['sources']
        if {name: sha(ROOT / name) for name in sources} != sources or sha(args.executable) != BUILD:
            raise ValueError('Declared sources/original differ before execution')
        parent = ROOT / 'working/tests/drawing-mouse-ring'
        parent.mkdir(parents=True, exist_ok=True)
        output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
        with (output / 'build.log').open('x') as log:
            subprocess.run(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/drawing-mouse-ring-reference.cpp'), '-o', str(output / 'reference')], stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (output / 'cases.jsonl').open('x') as rows, (output / 'execution.log').open('x') as log:
            result = subprocess.run([str(output / 'reference'), str(args.executable)], stdout=rows, stderr=log, timeout=90)
        (output / 'execution-status.json').write_text(json.dumps(dict(return_code=result.returncode)) + '\n')
        if result.returncode:
            raise ValueError('Original mouse fixture refused; retained at ' + str(output))
        rows = [json.loads(line) for line in (output / 'cases.jsonl').read_text().splitlines()]
        if rows[:-1] != [dict(write=i, cases=1728) for i in range(32)] or rows[-1] != dict(complete=True, cases=55296, collision_cases=27648, write_wraps=1728, current_wraps=1728, tail_wraps=864):
            raise ValueError('Incomplete/duplicate original matrix')
        if {name: sha(ROOT / name) for name in sources} != sources or sha(args.executable) != BUILD:
            raise ValueError('Declared sources/original changed during execution')
        artifacts = {str(p.relative_to(ROOT)): dict(sha256=sha(p), bytes=p.stat().st_size) for p in sorted(output.iterdir()) if p.is_file()}
        report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, original_sha256=BUILD, input_unchanged=True, original_writer_instructions_unchanged=True, original_game_executed=False, original_cases=55296, matrix=rows[-1], artifacts=artifacts, scope='Unchanged original4a3800 privately mapped32bit instructions, owned valid32record/40byte ring and one mocked GetAsyncKeyState(VK18) per call. All32write indices, two valid current indices,16button/modifier flag cells with ignored upper bits, three previous-coordinate/button relationships, three unsigned16bit coordinate values, three Alt responses and collision/noncollision tails. Exact full ring/unrelated cells and three index writes checked. Does not execute the game/window procedure, establish queue-consumer or concurrency/lifetime behavior, validate invalid indices, persistent selection/tooltip, native equivalence or physical presentation.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, cases=55296, artifacts=str(output))))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()

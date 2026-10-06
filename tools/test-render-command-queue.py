#!/usr/bin/env python3
"""Validate actual queued channel C code with heap/mapping fakes; no original inputs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    paths = ['runtime/render/command_channel.h', 'runtime/render/command_queue.h',
             'protocols/include/mnm/render_command_ring.h', 'protocols/include/mnm/render_commands_v1.h',
             'protocols/include/mnm/render_commands_v2.h', 'tests/render-command-writer-test.c',
             'tests/render-command-queue-test.c', 'tools/test-render-command-queue.py']
    def sha(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()
    sources = {p:sha(ROOT/p) for p in paths}
    parent = ROOT/'working/tests/render-command-queue'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    report = dict(success=True, sources=sources, scope='Actual C queue/channel under native heap/mapped '
                  'API fakes; original calls, PE32 and GPU integration require separate execution evidence.')
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1')
    for label, flags in [('native', ['-O2']), ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
        binary = run/label
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', *flags,
                        str(ROOT/'tests/render-command-queue-test.c'), '-o', str(binary)], check=True)
        result = subprocess.run([str(binary)], env=env, capture_output=True, text=True, timeout=45)
        (run/(label+'.log')).write_text(result.stdout+result.stderr)
        assert result.returncode == 0, result.stderr
        report[label] = [json.loads(line) for line in result.stdout.splitlines()]
        assert all(x['success'] for x in report[label])
    assert all(sha(ROOT/p) == h for p,h in sources.items()), 'Source changed during execution'
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(dict(success=True, report=str(run/'report.json'))), flush=True)


if __name__ == '__main__':
    main()

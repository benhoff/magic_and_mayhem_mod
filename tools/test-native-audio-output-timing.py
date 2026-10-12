#!/usr/bin/env python3
"""Validate native consumed-frame timing with independent fake and silent Qt clocks."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    if any(sha(ROOT / name) != digest for name, digest in sources.items()):
        raise ValueError('Consumed timing declaration drift')
    parent = ROOT / 'working/tests/native-audio-output-timing'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=sources, claims=declaration['claims'],
                  scope='Independent fake consumed-frame oracle and silent current Qt backend completion/restart. No original-driver, physical speaker, physical loss, live game timing or audible equivalence.')
    try:
        fixture = out / 'input-spec.json'
        fixture.write_text(json.dumps(dict(source_rates=[700, 1000, 1700, 2000],
                                          output_rate=2000, lengths=[1, 3, 7],
                                          consumed_frames=15, pcm='silence', backend='current default Qt output')) + '\n')
        inputs = {fixture.name: sha(fixture)}
        build = ROOT / 'working/build/audio'
        with (out / 'build.log').open('x') as log:
            subprocess.run(['cmake', '-S', str(ROOT / 'audio'), '-B', str(build)],
                           check=True, stdout=log, stderr=subprocess.STDOUT, timeout=90)
            subprocess.run(['cmake', '--build', str(build), '--target', 'audio-output-timeline-test', '-j', '4'],
                           check=True, stdout=log, stderr=subprocess.STDOUT, timeout=90)
        executable = build / 'audio-output-timeline-test'
        for name, arguments in [('oracle', []), ('backend', ['--backend'])]:
            with (out / (name + '.log')).open('x') as log:
                subprocess.run([str(executable), *arguments], check=True, stdout=log,
                               stderr=subprocess.STDOUT, timeout=15)
        report.update(inputs=inputs, inputs_stable=all(sha(out / name) == digest for name, digest in inputs.items()),
                      sources_stable=all(sha(ROOT / name) == digest for name, digest in sources.items()),
                      executable_sha256=sha(executable), independent_oracle_cases=360,
                      backend_completion_and_restart=True)
        if not report['inputs_stable'] or not report['sources_stable']:
            raise ValueError('Consumed timing sources or inputs changed during execution')
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
        raise
    finally:
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()

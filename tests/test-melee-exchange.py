#!/usr/bin/env python3
"""Bounded original-profile native exchange and fresh-process/window continuation."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--contract', type=Path, required=True)
    p.add_argument('--window', type=Path)
    p.add_argument('--installation', type=Path, default=ROOT/'working/game-clean')
    p.add_argument('--claims', type=Path)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    if a.output.exists():
        p.error('New output directory required')
    a.output.mkdir(parents=True)
    claim = json.loads(a.claims.read_text()) if a.claims else {'claims': [], 'sources': {}}
    if any(sha(ROOT/path) != digest for path, digest in claim['sources'].items()):
        raise ValueError('Declared source changed before execution')
    paths = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani']
    inputs = {path: sha(a.installation/path) for path in paths}
    commands = []

    def run(name, args, good=True):
        commands.append([str(x) for x in args])
        r = subprocess.run([str(x) for x in args], capture_output=True, text=True, timeout=90, cwd=ROOT)
        (a.output/(name+'.stdout')).write_text(r.stdout)
        (a.output/(name+'.stderr')).write_text(r.stderr)
        if (r.returncode == 0) != good or 'runtime error:' in r.stderr or 'ERROR: AddressSanitizer' in r.stderr:
            raise RuntimeError(f'{name}: {r.returncode}\n{r.stdout[-2000:]}{r.stderr[-2000:]}')
        return r.stdout

    contract = json.loads(run('contract', [a.contract, a.installation]))
    traces = [json.loads(line) for line in run('full', [a.binary, 'new', a.installation, 4096, 1, a.output/'full.mnm']).splitlines()]
    assert traces[-1]['health'].count(0) == 1
    assert sum(event[-1] for tick in traces for event in tick['events']) == 1
    hit_ticks = [t['tick'] for t in traces if any(e[1] == 2 for e in t['events'])]
    assert hit_ticks and hit_ticks[0] > 2
    points = sorted(set(range(min(40, traces[-1]['tick']+1))) | {traces[-1]['tick']-1, traces[-1]['tick']} | set(hit_ticks[:8]))
    for tick in points:
        checkpoint = a.output/f'checkpoint-{tick}.mnm'
        run(f'capture-{tick}', [a.binary, 'new', a.installation, tick, 1, checkpoint])
        resumed = [json.loads(line) for line in run(f'resume-{tick}', [a.binary, 'resume', checkpoint, 4096, 0, a.output/f'resumed-{tick}.mnm']).splitlines()]
        assert resumed[0]['tick'] == tick
        assert resumed[1:] == traces[tick+1:]
        assert (a.output/f'resumed-{tick}.mnm').read_bytes() == (a.output/'full.mnm').read_bytes()
    # Distinct seeds and fresh-process repeatability; originals are not required.
    for seed in [0, 0x80000000, 0xffffffff]:
        run(f'seed-{seed}', [a.binary, 'new', a.installation, 4096, seed, a.output/f'seed-{seed}.mnm'])
        run(f'seed-repeat-{seed}', [a.binary, 'new', a.installation, 4096, seed, a.output/f'seed-repeat-{seed}.mnm'])
        assert (a.output/f'seed-{seed}.mnm').read_bytes() == (a.output/f'seed-repeat-{seed}.mnm').read_bytes()
    (a.output/'truncated.mnm').write_bytes((a.output/'full.mnm').read_bytes()[:-1])
    run('refuse-truncated', [a.binary, 'resume', a.output/'truncated.mnm', 1, 0, a.output/'bad.mnm'], False)
    run('refuse-missing', [a.binary, 'resume', a.output/'missing.mnm', 1, 0, a.output/'bad.mnm'], False)
    run('refuse-tick-limit', [a.binary, 'new', a.installation, 4097, 1, a.output/'bad.mnm'], False)
    run('refuse-existing', [a.binary, 'new', a.installation, 1, 1, a.output/'full.mnm'], False)
    window = None
    if a.window:
        run('window', ['xvfb-run', '-a', a.window, '--installation', a.installation, '--test-output', a.output/'window'])
        window = json.loads((a.output/'window/result.json').read_text())
        assert window['success'] and window['health'] == traces[-1]['health'] and window['tick'] == traces[-1]['tick']
        assert (a.output/'window/final.mnm').read_bytes() == (a.output/'full.mnm').read_bytes()
        resumed = [json.loads(line) for line in run('window-fresh-resume', [a.binary, 'resume', a.output/'window/mid-attack.mnm', 4096, 0, a.output/'window-resumed.mnm']).splitlines()]
        assert resumed[1:] == traces[14:]
        assert (a.output/'window-resumed.mnm').read_bytes() == (a.output/'full.mnm').read_bytes()
    assert all(sha(a.installation/path) == digest for path, digest in inputs.items())
    stable = all(sha(ROOT/path) == digest for path, digest in claim['sources'].items())
    assert stable
    report = dict(success=True, contract=contract, claims=claim['claims'], source_sha256=claim['sources'],
                  input_sha256=inputs, input_unchanged=True, sources_unchanged=stable,
                  process_continuations=len(points), final=traces[-1], first_hit_tick=hit_ticks[0], window=window,
                  binaries={'cli': sha(a.binary), 'contract': sha(a.contract)}, commands=commands,
                  wine_required=False, original_required_on_resume=False, live_replacement=False,
                  scope='Bounded stationary native policy with original-profile attack/health adapters and complete checkpoints; not whole original fight/AI/stun/reach equivalence.')
    (a.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({k: report[k] for k in ['success', 'process_continuations', 'first_hit_tick', 'final']}))


if __name__ == '__main__':
    main()

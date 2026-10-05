#!/usr/bin/env python3
"""Native stationary blockers and fresh-process continuation; no game media."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('fixture', ROOT / 'tools/create-movement-fixture.py')
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


def main():
    sandbox, test = map(lambda p: str(Path(p).resolve()), sys.argv[1:3])
    persistent = Path(sys.argv[3]).resolve() if len(sys.argv) == 4 else None
    temporary = tempfile.TemporaryDirectory(prefix='mnm-occupancy-') if persistent is None else None
    root = Path(temporary.name) if temporary else persistent
    root.mkdir(parents=True, exist_ok=True)
    if any(root.iterdir()):
        raise ValueError('Refusing occupied output directory')

    def run(binary, *args, good=True):
        result = subprocess.run([binary, *map(str, args)], capture_output=True, text=True, timeout=60)
        if result.returncode != (0 if good else 1):
            raise AssertionError((args, result.stdout, result.stderr))
        if 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise AssertionError(result.stderr)
        return result

    map_file = root / 'map.frozen'
    map_file.write_bytes(fixture.fixture())
    result = run(test, map_file, root)
    (root / 'cases.stdout').write_text(result.stdout)
    run(sandbox, 'resume', root / 'obstructed.mnms', root / 'resumed.mnms', 4)
    assert (root / 'resumed.mnms').read_bytes() == (root / 'uninterrupted.mnms').read_bytes()
    trace = [json.loads(line) for line in run(sandbox, 'trace', root / 'obstructed.mnms', 4).stdout.splitlines()]
    assert trace[0]['entities'][0]['action'] == 2
    assert all(row['entities'][0]['action'] == 4 and row['entities'][0]['position'] == [1, 1, 1] for row in trace[1:])
    # Existing v2 wire carries stationary entity state; rebuilt occupancy agrees after restart.
    run(sandbox, 'move-occupied', map_file, root / 'initial.mnms', 1, 1, 1, 5, 1, 1, 5, 1, 1, 0)
    run(sandbox, 'resume', root / 'initial.mnms', root / 'split.mnms', 2)
    run(sandbox, 'resume', root / 'initial.mnms', root / 'whole.mnms', 8)
    run(sandbox, 'resume', root / 'split.mnms', root / 'continued.mnms', 6)
    assert (root / 'whole.mnms').read_bytes() == (root / 'continued.mnms').read_bytes()
    final = json.loads(run(sandbox, 'inspect-json', root / 'whole.mnms').stdout)
    assert final['entities'][0]['action'] == 4 and len(final['entities']) == 2
    # A stationary blocker on the direct path produces a usable detour.
    run(sandbox, 'move-occupied', map_file, root / 'detour-initial.mnms', 1, 1, 1, 5, 1, 1, 3, 1, 1, 0)
    detour = [json.loads(line) for line in run(sandbox, 'trace', root / 'detour-initial.mnms', 14).stdout.splitlines()]
    assert detour[-1]['entities'][0]['action'] == 3 and detour[-1]['entities'][0]['position'] == [5, 1, 1]
    assert all(row['entities'][0]['position'] != [3, 1, 1] for row in detour)
    run(sandbox, 'resume', root / 'detour-initial.mnms', root / 'detour-split.mnms', 3)
    run(sandbox, 'resume', root / 'detour-initial.mnms', root / 'detour-whole.mnms', 14)
    run(sandbox, 'resume', root / 'detour-split.mnms', root / 'detour-continued.mnms', 11)
    assert (root / 'detour-whole.mnms').read_bytes() == (root / 'detour-continued.mnms').read_bytes()
    # Opt-in admission refuses overlaps before checkpoint publication.
    run(sandbox, 'move-occupied', map_file, root / 'refused.mnms', 1, 1, 1, 5, 1, 1, 1, 1, 1, 0, good=False)
    assert not (root / 'refused.mnms').exists()
    sources = [ROOT / 'game/CMakeLists.txt', ROOT / 'tools/create-movement-fixture.py', Path(__file__).resolve(), ROOT / 'tests/native-occupancy-test.cpp']
    for directory in ['game', 'apps/world-sandbox', 'reconstruction/pathfinding', 'reconstruction/motion']:
        sources += [p for p in (ROOT / directory).rglob('*') if p.is_file() and p.suffix in ('.cpp', '.hpp')]
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    report = {'all_match': True, 'fresh_process_continuations': 3, 'detour_arrival': True, 'overlap_refused_before_output': True,
              'intra_cell_obstruction': True, 'cleanup_and_release': True, 'stale_generation': True,
              'checkpoint_mode_mismatch_refusal': True, 'live_validated': False,
              'scope': 'Native opt-in logical occupancy for one mover and stationary same-profile blockers; native slot token projection, positive nonwrapping bounded boxes, cleanup/release and per-tick edge checks. No original world production/reservation or multiple moving creature equivalence.',
              'source_sha256': {str(p.relative_to(ROOT)): digest(p) for p in sorted(set(sources))},
              'binary_sha256': {sandbox: digest(Path(sandbox)), test: digest(Path(test))},
              'artifact_sha256': {p.name: digest(p) for p in sorted(root.iterdir()) if p.is_file()}}
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Native occupancy and three fresh-process continuations pass:', root)
    if temporary:
        temporary.cleanup()


if __name__ == '__main__':
    main()

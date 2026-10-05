#!/usr/bin/env python3
"""Native multi-mover reservations and fresh-process checkpoints; synthetic inputs."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('fixture', ROOT / 'tools/create-movement-fixture.py')
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


def slow_fixture():
    raw = bytearray(fixture.fixture())
    sizes = struct.unpack_from('<7I', raw, 64)
    scalar = 92 + sum(sizes[:5])
    struct.pack_into('<i', raw, scalar + 16, 10)
    for i in range(48):
        struct.pack_into('<I', raw, scalar + 0xd8 + i * 4, 8)
    return raw


def animation():
    records, starts = [], [0]
    for direction in range(16):
        program = [(6, -1)] if direction < 8 else [(1, 2), (0, 100 + direction), (0, 101 + direction), (5, 2), (6, -1)]
        records += program
        starts.append(len(records))
    size = 44 + len(starts) * 4 + len(records) * 44
    return b'ANI\0' + struct.pack('<5I', size, len(records), 5, 0, len(starts)) + b'fixture.spr\0'.ljust(20, b'\0') + struct.pack('<17I', *starts) + b''.join(struct.pack('<Ii9I', op, arg, *([0] * 9)) for op, arg in records)


def main():
    sandbox, test = [str(Path(p).resolve()) for p in sys.argv[1:3]]
    temporary = tempfile.TemporaryDirectory(prefix='mnm-multi-movement-') if len(sys.argv) == 3 else None
    root = Path(temporary.name) if temporary else Path(sys.argv[3]).resolve()
    root.mkdir(parents=True, exist_ok=True)
    if any(root.iterdir()):
        raise ValueError('Refusing occupied output directory')

    def run(binary, *args, good=True):
        result = subprocess.run([binary, *map(str, args)], text=True, capture_output=True, timeout=120)
        if result.returncode != (0 if good else 1) or 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise AssertionError((args, result.stdout, result.stderr))
        return result.stdout

    normal, slow, ani = [root / name for name in ['normal.frozen', 'slow.frozen', 'movement.ani']]
    normal.write_bytes(fixture.fixture())
    slow.write_bytes(slow_fixture())
    ani.write_bytes(animation())
    output = run(test, normal, slow, ani, root)
    (root / 'cases.stdout').write_text(output)
    # Saved ongoing reservations and independent ANI cursors survive a fresh process.
    for label, ticks in [('fine', 180), ('ani', 500)]:
        checkpoint = root / ('mid-fine.mnms' if label == 'fine' else 'ani-mid.mnms')
        run(sandbox, 'resume', checkpoint, root / f'{label}-resumed.mnms', ticks)
        assert (root / f'{label}-resumed.mnms').read_bytes() == (root / f'{label}-whole.mnms').read_bytes()
    continuations = 2
    scenarios = [('parallel', 'move-pair', normal, [1, 1, 1, 5, 1, 1, 1, 4, 1, 5, 4, 1], 12, 3),
                 ('parallel-fine', 'move-pair-fine', slow, [1, 1, 1, 5, 1, 1, 1, 4, 1, 5, 4, 1], 180, 17),
                 ('converging', 'move-pair', normal, [1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 2, 1], 8, 3)]
    for label, command, map_file, coords, count, split in scenarios:
        initial, half, whole, resumed = [root / f'{label}-{name}.mnms' for name in ['initial', 'half', 'whole', 'resumed']]
        run(sandbox, command, map_file, initial, *coords, 0)
        run(sandbox, 'resume', initial, half, split)
        run(sandbox, 'resume', initial, whole, count)
        run(sandbox, 'resume', half, resumed, count - split)
        assert whole.read_bytes() == resumed.read_bytes()
        trace = [json.loads(line) for line in run(sandbox, 'trace', initial, count).splitlines()]
        assert all(len({tuple(e['position']) for e in state['entities']}) == 2 for state in trace)
        final = trace[-1]['entities']
        if label == 'converging':
            assert final[0]['action'] == 3 and final[1]['action'] == 2
        else:
            assert all(e['action'] == 3 for e in final)
        (root / f'{label}-trace.json').write_text(json.dumps(trace, indent=2) + '\n')
        continuations += 1
    # Deliberate swap waiting is reproducible, not silently resolved by teleportation.
    run(sandbox, 'resume', root / 'swap.mnms', root / 'swap-half.mnms', 3)
    run(sandbox, 'resume', root / 'swap.mnms', root / 'swap-whole.mnms', 8)
    run(sandbox, 'resume', root / 'swap-half.mnms', root / 'swap-resumed.mnms', 5)
    assert (root / 'swap-whole.mnms').read_bytes() == (root / 'swap-resumed.mnms').read_bytes()
    final = json.loads(run(sandbox, 'inspect-json', root / 'swap-whole.mnms'))['entities']
    assert [e['position'] for e in final] == [[1, 1, 1], [2, 1, 1]] and all(e['action'] == 2 for e in final)
    continuations += 1
    run(sandbox, 'move-pair', normal, root / 'refused.mnms', 1, 1, 1, 5, 1, 1, 1, 1, 1, 5, 4, 1, 0, good=False)
    assert not (root / 'refused.mnms').exists()
    sources = [Path(__file__).resolve(), ROOT / 'tests/native-multi-movement-test.cpp', ROOT / 'game/CMakeLists.txt', ROOT / 'tools/create-movement-fixture.py', ROOT / 'assets/animation_decode.cpp', ROOT / 'assets/animation.hpp']
    for directory in ['game', 'apps/world-sandbox', 'reconstruction/pathfinding', 'reconstruction/motion', 'reconstruction/animation']:
        sources += [p for p in (ROOT / directory).rglob('*') if p.is_file() and p.suffix in ('.cpp', '.hpp')]
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    report = {'all_match': True, 'fresh_process_continuations': continuations,
              'logical_reservations': True, 'conservative_swap_crossing_waits': True,
              'independent_ani_solo_transitions': 1000, 'shared_search_budget': 53, 'moving_capacity': 32,
              'generation_cleanup_rollback': True, 'saved_conflicting_claims_refused': True,
              'overlap_refused_before_output': True, 'live_validated': False,
              'scope': 'Native opt-in bounded same-profile multi-movers, conservative swept grid boxes, whole-phase holds, conflict waiting, rotating planner priority and actual shared expansion debit; derived fine-edge reservations and independent ANI/checkpoint continuation. No original scheduler/occupancy production, physical collision, mixed profiles, presentation or live equivalence.',
              'source_sha256': {str(p.relative_to(ROOT)): digest(p) for p in sorted(set(sources))},
              'binary_sha256': {sandbox: digest(Path(sandbox)), test: digest(Path(test))},
              'artifact_sha256': {p.name: digest(p) for p in sorted(root.iterdir()) if p.is_file()}}
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Native multi-movement and', continuations, 'fresh-process continuations pass:', root)
    if temporary:
        temporary.cleanup()


if __name__ == '__main__':
    main()

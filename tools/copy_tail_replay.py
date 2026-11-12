"""Offline COPY attribution and replay checks; never promote historical capture."""
from collections import Counter
import hashlib
import json
import struct

from rendering_pilot import commands, require


def trace(raw, witnessed, routes, checkpoints):
    """Bind each COPY uniquely to its emitted original interval, not its ordinal."""
    observed = {row['sequence']: row for row in witnessed}
    require(len(observed) == len(witnessed), 'Duplicate captured COPY sequence')
    intervals = sorted((r[40], r[41], r) for r in routes if r[40])
    require(all(a[1] < b[0] for a, b in zip(intervals, intervals[1:])),
            'Overlapping original COPY intervals')
    at = 0
    world = 0
    copies = []
    seen = set()
    for op, seq, payload in commands(raw):
        if op == 19:
            world = struct.unpack_from('<Q', payload, 4)[0]
        if op != 3:
            continue
        require(len(payload) == 40, 'COPY payload length')
        words = list(struct.unpack('<10I', payload)) + [0, 0, 0]
        while at < len(intervals) and intervals[at][1] < seq:
            at += 1
        require(at < len(intervals) and intervals[at][0] <= seq <= intervals[at][1],
                'COPY lacks original interval')
        first, last, row = intervals[at]
        require(words[:2] == list(row[42:44]), 'Original COPY logical handles')
        covered = seq in observed
        if covered:
            require(observed[seq]['words'] == words, 'Captured COPY arguments differ')
            seen.add(seq)
        prior = [p for p in checkpoints if int(p['source_generation']) <= world]
        following = [p for p in checkpoints if int(p['source_generation']) > world]
        copies.append(dict(sequence=seq, world_generation=world, words=words,
                           original_call=row[0], caller_rva=hex(row[2]),
                           route=('unprepared', 'cpu_prepared', 'gpu_prepared', 'fill_prepared')[row[15]],
                           interval=[first, last], individually_captured=covered,
                           keyed=bool(words[8]), attached_clipper=bool(row[23]),
                           self_alias=bool(row[10] & 128),
                           checkpoint_window=[prior[-1]['phase'] if prior else 'session-start',
                                              following[0]['phase'] if following else 'normal-quit']))
    require(seen == set(observed), 'Captured witness outside complete command history')
    missing = [row for row in copies if not row['individually_captured']]
    groups = Counter((r['route'], r['caller_rva'], tuple(r['checkpoint_window']),
                      r['keyed'], r['attached_clipper'], r['self_alias']) for r in missing)
    return dict(copies=len(copies), individually_captured=len(observed), missing=len(missing),
                missing_original_calls=len({r['original_call'] for r in missing}),
                missing_gpu_calls=len({r['original_call'] for r in missing if r['route'] == 'gpu_prepared'}),
                missing_groups=[dict(route=k[0], caller_rva=k[1], checkpoint_window=list(k[2]),
                                     keyed=k[3], attached_clipper=k[4], self_alias=k[5], copies=v)
                                for k, v in sorted(groups.items())], missing_copies=missing)


def offline_observer(source):
    """Exact, reviewable build-only observer variant; production file stays intact."""
    edits = [
        ('#include <set>', '#include <set>\n#include <thread>\n#include <chrono>'),
        ('p.enqueued>=8192', 'p.enqueued>=16384'),
        ('if(!p.worker.reserve(p.pendingCharge)){p.checkError();p.truncated=1;return false;}',
         'require(p.pendingCharge<=Impl::QueueLimit,"Offline COPY exceeds queue");\n'
         '    QElapsedTimer wait;wait.start();\n'
         '    while(!p.worker.reserve(p.pendingCharge)){p.checkError();'
         'require(wait.elapsed()<60000,"Offline COPY sink timeout");'
         'std::this_thread::sleep_for(std::chrono::milliseconds(1));}'),
        ('Selected native COPY commands until8192copies/32GiB diagnostic readbacks.',
         'Offline source-only replay until16384copies/32GiB readbacks; bounded sink waits.'),
    ]
    for before, after in edits:
        require(source.count(before) == 1, 'Observer preimage changed: ' + before)
        source = source.replace(before, after)
    return source


def compare_records(captured, replayed, expected_copies):
    require(len(replayed) == expected_copies and len({r['sequence'] for r in replayed}) == expected_copies,
            'Incomplete or duplicate replay COPY history')
    replay = {r['sequence']: r for r in replayed}
    keys = ('words', 'source_width', 'source_height', 'target_width', 'target_height',
            'pixels', 'mismatches', 'source_sha256', 'before_sha256', 'actual_sha256', 'expected_sha256')
    for row in replayed:
        require(row['mismatches'] == 0 and row['actual_sha256'] == row['expected_sha256'],
                'Replay COPY mismatch')
    for row in captured:
        require(row['sequence'] in replay and all(row[k] == replay[row['sequence']][k] for k in keys),
                'Replay differs from actual captured COPY prefix at ' + str(row['sequence']))
    return dict(prefix_anchors=len(captured), replay_copies=len(replayed),
                additional_native_replay_checks=len(replayed)-len(captured),
                historical_capture_complete=False)


def compare_cpu(captured, replayed):
    expected = {}
    for row, body in captured:
        if row['kind'] == 'cpu-read':
            require(len(body) >= 64 and row['sequence'] not in expected, 'CPU reply identity')
            expected[row['sequence']] = hashlib.sha256(body).hexdigest()
    require(len(replayed) == len(expected) and len({r['sequence'] for r in replayed}) == len(expected),
            'CPU reply history incomplete')
    for row in replayed:
        require(expected.get(row['sequence']) == row['sha256'], 'Actual CPU reply differs from replay')
    return len(expected)

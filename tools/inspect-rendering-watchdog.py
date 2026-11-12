#!/usr/bin/env python3
"""Read retained command/host timing evidence without granting acceptance."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

from coverage_evidence import Reader, write_report
from rendering_pilot import capture, require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-watchdog-results-20261101/'


def inspect(reader, name):
    acquired = reader.json(name)
    artifacts = acquired['artifacts']
    result = dict(schema=1, kind='native_watchdog_observation_review',
                  success=True, acquisition=dict(record=name, sha256=reader.digest(name)),
                  acquisition_success=acquired['success'],
                  sources={'tools/inspect-rendering-watchdog.py': reader.digest('tools/inspect-rendering-watchdog.py')},
                  observations=[], host_operations=[], host_polls=[], host_events=[],
                  scope='Verified retained bytes and bounded timing observations only; no root cause, repair, current-code equivalence, requirement or failure-retirement claim.')
    for artifact in artifacts:
        if artifact['path'].endswith('/public-launch.log'):
            raw = reader.read_bytes(artifact['record'])
            require(hashlib.sha256(raw).hexdigest() == artifact['sha256'], 'Changed launch log')
            result['observations'].append(artifact)
            for line in raw.decode(errors='replace').splitlines():
                for label, key in [('NATIVE_HOST_OPERATION ', 'host_operations'), ('NATIVE_RESIDENT_POLL ', 'host_polls'), ('NATIVE_HOST_EVENT ', 'host_events')]:
                    if not line.startswith(label):
                        continue
                    row = json.loads(line[len(label):])
                    if key == 'host_operations':
                        require(set(row) == {'stage', 'begin_host_us', 'elapsed_us'}, 'Unknown host timing fields')
                        require(type(row['stage']) is str and len(row['stage']) < 128, 'Invalid timing label')
                        for field in ('begin_host_us', 'elapsed_us'):
                            require(type(row[field]) is str and row[field].isascii() and row[field].isdecimal(), 'Invalid host timing identity')
                        require(int(row['elapsed_us']) >= 50000, 'Unexpected host timing threshold')
                    result[key].append(row)
            require(len(result['host_operations']) <= 256 and len(result['host_polls']) <= 256 and len(result['host_events']) <= 256, 'Unbounded timing output')
    result['host_operations'].sort(key=lambda row: int(row['elapsed_us']), reverse=True)
    result['host_events'].sort(key=lambda row: int(row['elapsed_us']), reverse=True)
    rings = [row for row in artifacts if row['path'].endswith('/render-commands.bin')]
    if rings:
        require(len(rings) == 1, 'Ambiguous command ring')
        ring = rings[0]
        raw = reader.read_bytes(ring['record'])
        require(hashlib.sha256(raw).hexdigest() == ring['sha256'], 'Changed ring')
        require(len(raw) == 1048640 and raw[:8] == b'MNMRDC03', 'Unsupported ring layout')
        header = struct.unpack_from('<16I', raw)
        require(header[2:4] == (3, 1048640) and header[4] and not any(header[10:]), 'Invalid ring identity')
        published, ack = header[5], header[9]
        count = (published - ack) & 0xffffffff
        require(count <= 1048576, 'Unbounded ring distance')
        offset = ack % 1048576
        tail = (raw[64+offset:] + raw[64:64+offset])[:count]
        command_captures = [row for row in artifacts if '/capture/commands-' in row['path'] and row['path'].endswith('.bin')]
        require(len(command_captures) == 1, 'Missing unique consumed command capture')
        captured = reader.read_bytes(command_captures[0]['record'])
        rows = capture(captured)
        consumed = b''.join(body for row, body in rows if row['kind'] == 'fragment')
        require(len(consumed) & 0xffffffff == ack, 'Capture and ACK differ')
        require(consumed[:16] == b'MNMCMD05' + struct.pack('<II', 5, 16), 'Unsupported consumed stream')
        at = 16
        sequence = 0
        while at < len(consumed):
            require(at + 12 <= len(consumed), 'Partial consumed command header')
            op, seq, size = struct.unpack_from('<III', consumed, at)
            require(seq == sequence + 1 and at + 12 + size <= len(consumed), 'Partial consumed command body/order')
            sequence = seq
            at += 12 + size
        commands = []
        at = 0
        while at < len(tail):
            require(at + 12 <= len(tail), 'Partial unread command header')
            op, seq, size = struct.unpack_from('<III', tail, at)
            require(seq == sequence + 1 and at + 12 + size <= len(tail), 'Partial unread command body/order')
            words = min(size // 4, 5 if op in (19, 20) else 10)
            commands.append(dict(offset=(ack + at) & 0xffffffff, operation=op, sequence=seq, bytes=size,
                                 words=list(struct.unpack_from('<' + str(words) + 'I', tail, at+12))))
            sequence = seq
            at += 12 + size
        result['ring'] = dict(record=ring, state=header[6], reason=header[7], cancelled=header[8],
                              ack=ack, published=published, unread_bytes=count,
                              consumed_last_sequence=sequence-len(commands),
                              operation_counts=dict(Counter(row['operation'] for row in commands)), commands=commands)
        result['observations'].append(command_captures[0])
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--acquisition', required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'New owned immutable report required')
    with Reader(ROOT) as reader:
        result = inspect(reader, args.acquisition)
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(record=args.report, sha256=sha, acquisition_success=result['acquisition_success'],
                         host_operations=result['host_operations'][:8], host_events=result['host_events'][:8], ring={k:v for k,v in result.get('ring', {}).items() if k not in {'commands','record'}})), flush=True)


if __name__ == '__main__':
    main()

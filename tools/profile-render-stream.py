#!/usr/bin/env python3
"""Read-only publication-rate sampling; does not measure Qt paints or game FPS."""
import argparse
import json
import mmap
from pathlib import Path
import signal
import struct
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'protocols/python'))
from mnm_protocols import frame_v1 as wire


def sample(mapping):
    sequence = struct.unpack_from('<I', mapping, wire.SEQUENCE_OFFSET)[0]
    if sequence & 1:
        return 'busy', None, None
    header = bytes(mapping[:wire.HEADER_SIZE])
    after = struct.unpack_from('<I', mapping, wire.SEQUENCE_OFFSET)[0]
    if after != sequence:
        return 'busy', None, None
    if header[:8] != wire.MAGIC or struct.unpack_from('<II', header, 8) != (wire.VERSION, wire.DECLARED_SIZE):
        return 'invalid', None, None
    return 'stable', struct.unpack_from('<I', header, wire.FRAME_COUNT_OFFSET)[0], struct.unpack_from('<I', header, wire.STATUS_OFFSET)[0]


class RateWindow:
    def __init__(self, start):
        self.start = self.previous = start
        self.counter = self.status = None
        self.samples = self.busy = self.invalid = self.updates = 0

    def observe(self, state, counter, status):
        self.samples += 1
        self.busy += state == 'busy'
        self.invalid += state == 'invalid'
        if state == 'stable':
            if self.counter is not None:
                self.updates += (counter - self.counter) & 0xffffffff
            self.counter, self.status = counter, status

    def report(self, now):
        seconds = now - self.previous
        row = dict(version=1, origin='frame_v1_header_sampling', elapsed_seconds=now-self.start,
                   window_seconds=seconds, published_updates=self.updates,
                   published_updates_per_second=self.updates/seconds if seconds else 0,
                   samples=self.samples, busy_samples=self.busy, invalid_samples=self.invalid,
                   last_frame_count=self.counter, status=self.status)
        self.previous = now
        self.samples = self.busy = self.invalid = self.updates = 0
        return row


def process_identity(pid):
    try:
        fields = Path(f'/proc/{pid}/stat').read_text().rsplit(')', 1)[1].split()
        return None if fields[0] == 'Z' else fields[19]
    except (OSError, IndexError):
        return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('stream', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--parent-pid', type=int)
    parser.add_argument('--seconds', type=float, default=0)
    args = parser.parse_args()
    if args.seconds <= 0 and not args.parent_pid:
        parser.error('Specify a positive --seconds or --parent-pid.')
    identity = process_identity(args.parent_pid) if args.parent_pid else None
    if args.parent_pid and identity is None:
        return
    stop = False

    def finish(*unused):
        nonlocal stop
        stop = True

    signal.signal(signal.SIGTERM, finish)
    signal.signal(signal.SIGINT, finish)
    with args.stream.open('rb') as file, mmap.mmap(file.fileno(), wire.SIZE, access=mmap.ACCESS_READ) as mapping, args.output.open('w') as output:
        window = RateWindow(time.monotonic())
        while not stop:
            window.observe(*sample(mapping))
            now = time.monotonic()
            ended = (args.seconds > 0 and now-window.start >= args.seconds) or (args.parent_pid and process_identity(args.parent_pid) != identity)
            if now-window.previous >= 1 or ended:
                output.write(json.dumps(window.report(now))+'\n')
                output.flush()
            if ended:
                break
            time.sleep(1/60)
        if window.samples:
            output.write(json.dumps(window.report(time.monotonic()))+'\n')


if __name__ == '__main__':
    main()

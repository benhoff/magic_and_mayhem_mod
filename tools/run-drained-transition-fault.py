#!/usr/bin/env python3
"""Wait for the unchanged injector's drained admission inside its five-second call."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time

HELPER = 'working/broader-player/rendering-transition-fault'
HELPER_SHA256 = '13d7b2270af2418e8d95f47ea360eeb360b7c061bc90c1d781fef64ca962b2a0'
DRAIN_REFUSAL = b'Require drained Mini boundary, not a pending-read fault\n'
WAIT_SECONDS = 2.0
MAXIMUM_ATTEMPTS = 128


def admit(command, expected_sha256, *, timeout=WAIT_SECONDS, maximum_attempts=MAXIMUM_ATTEMPTS):
    """Retry only the exact pre-mutation guard refusal; never launch a game."""
    helper = Path(command[0])
    if hashlib.sha256(helper.read_bytes()).hexdigest() != expected_sha256:
        raise ValueError('Frozen native fault helper changed')
    started = time.monotonic()
    deadline = started + timeout
    refusals = []
    for attempt in range(1, maximum_attempts + 1):
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break
        result = subprocess.run(command, capture_output=True, timeout=remaining)
        if result.returncode == 0:
            value = json.loads(result.stdout)
            if result.stderr or value.get('success') is not True or value.get('reason') != 5 or value.get('writer_claimed') is not False or value.get('cursor_or_payload_written') is not False:
                raise ValueError('Unexpected successful native fault receipt')
            value['drain_admission'] = dict(attempts=attempt, prior_refusals=refusals,
                maximum_attempts=maximum_attempts, budget_seconds=timeout,
                elapsed_seconds=time.monotonic()-started, unknown_refusals_retried=False,
                original_game_starts=0, cursor_or_payload_written_by_adapter=False)
            return value
        if result.returncode != 1 or result.stdout or result.stderr != DRAIN_REFUSAL:
            raise ValueError('Native fault helper refused without retry: '+repr((result.returncode,result.stdout,result.stderr)))
        refusals.append(dict(attempt=attempt,stderr=result.stderr.decode(),host_ns=time.monotonic_ns()))
        time.sleep(min(.01,max(0,deadline-time.monotonic())))
    raise ValueError('Drained native fault admission deadline/attempt bound: '+str(len(refusals)))


def main():
    if len(sys.argv) != 4:
        raise ValueError('Usage: adapter owned-repository channel expected-session')
    root = Path(sys.argv[1]).resolve(strict=True)
    if root != Path(__file__).resolve().parents[1]:
        raise ValueError('Adapter must run in its owned repository')
    helper = root/HELPER
    print(json.dumps(admit([str(helper),*sys.argv[1:]],HELPER_SHA256)),flush=True)


if __name__ == '__main__':
    try:
        main()
    except (ValueError,OSError,subprocess.TimeoutExpired) as error:
        print(str(error),file=sys.stderr)
        raise SystemExit(1)

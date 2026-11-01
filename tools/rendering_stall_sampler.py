"""Diagnostic parent-GDB sampling of one admitted V3 command consumer.

Read-only ring observation never acknowledges commands or renews a deadline.
"""
import hashlib
import json
import math
import os
from pathlib import Path
import signal
import struct
import subprocess
import time

CAPACITY = 1048576
SIZE = CAPACITY + 64
THRESHOLD_SECONDS = 1.5


class PendingAckWatch:
    def __init__(self, threshold=THRESHOLD_SECONDS):
        if not math.isfinite(threshold) or not 0 < threshold < 5:
            raise ValueError('Sample before the unchanged five-second watchdog')
        self.threshold = threshold
        self.session = self.ack = self.since = self.last_time = None
        self.requested = False
        self.peak_seconds = 0

    def reset_interval(self):
        self.ack = self.since = None

    def observe(self, raw, size, now):
        if not math.isfinite(now) or (self.last_time is not None and now < self.last_time):
            raise ValueError('Invalid monotonic observation time')
        self.last_time = now
        if len(raw) != 64 or size != SIZE:
            raise ValueError('Incomplete V3 ring')
        w = struct.unpack('<16I', raw)
        if (raw[:8] != b'MNMRDC03' or w[2:4] != (3, SIZE) or not w[4]
                or w[6] not in range(4) or w[7] not in range(6) or w[8] not in (0, 1)
                or any(w[10:]) or ((w[5] - w[9]) & 0xffffffff) > CAPACITY):
            raise ValueError('Invalid V3 ring header')
        if self.session is not None and self.session != w[4]:
            raise ValueError('Command session changed')
        self.session = w[4]
        if w[6] != 1 or w[7] or w[8] or w[5] == w[9]:
            self.reset_interval()
            return None
        if self.since is None or self.ack != w[9]:
            self.since = now
        self.ack = w[9]
        elapsed = now - self.since
        self.peak_seconds = max(self.peak_seconds, elapsed)
        if elapsed < self.threshold or self.requested:
            return None
        self.requested = True
        return dict(session=w[4], published=w[5], acknowledged=w[9],
                    unread_bytes=(w[5] - w[9]) & 0xffffffff,
                    pending_seconds=elapsed, host_monotonic_seconds=now,
                    header_hex=raw.hex())


class OwnedRing:
    """Resolve only the launcher's admission journal, never a global recent run."""
    def __init__(self, repository, owner):
        self.repository = Path(repository).resolve()
        self.owner = Path(owner).resolve()
        if not self.owner.is_relative_to(self.repository/'working/tests/native-output-owner'):
            raise ValueError('Sampler requires the actual public-player owner')
        if not self.owner.is_file():
            raise ValueError('Missing public-player owner receipt')
        self.path = None
        self.file_identity = None

    def read(self):
        entries = sorted((self.owner.parent/'sessions').glob('*.json'))
        if not entries:
            return None
        if len(entries) != 1 or entries[0].is_symlink():
            raise ValueError('Sampler requires exactly one admitted generation')
        raw_admission = entries[0].read_bytes()
        if len(raw_admission) > 4*1024**2:
            raise ValueError('Oversized admission journal')
        try:
            admission = json.loads(raw_admission)
        except json.JSONDecodeError:
            return None  # Admission is published through a physical JSON writer.
        session = Path(admission['session'])
        parent = self.repository/'working/experiments/menu-observer'
        if (not session.is_absolute() or session.resolve() != session
                or session.parent != parent or not session.name.startswith('run-')
                or admission.get('native_only', {}).get('policy') != 'native-only-rendering-v1'):
            raise ValueError('Invalid admitted session identity')
        path = session/'render-commands.bin'
        if self.path is not None and self.path != path:
            raise ValueError('Admitted generation changed')
        self.path = path
        if not path.exists():
            return None
        if path.is_symlink():
            raise ValueError('Symlinked command ring')
        with path.open('rb') as stream:
            info = os.fstat(stream.fileno())
            identity = (info.st_dev, info.st_ino)
            if self.file_identity is not None and self.file_identity != identity:
                raise ValueError('Command ring file replaced')
            raw = stream.read(64)
            if info.st_size != SIZE or len(raw) != 64 or not any(raw):
                return None  # The channel may still be under construction.
            stream.seek(0)
            if stream.read(64) != raw:
                return None  # A concurrent publish/ACK is not a stable snapshot.
        self.file_identity = identity
        return raw, info.st_size


GDB_COMMANDS = '''set debuginfod enabled off
set auto-load off
set disable-randomization off
set print thread-events off
set pagination off
handle SIGPIPE nostop noprint pass
handle SIGUSR1 nostop noprint pass
handle SIGUSR2 nostop noprint pass
run
if $_isvoid($_exitcode)
  if $_siginfo.si_signo != 2
    quit 128 + $_siginfo.si_signo
  end
  echo MNM_WATCHDOG_STACK_BEGIN\\n
  thread apply all bt 32
  echo MNM_WATCHDOG_STACK_END\\n
  continue
end
if $_isvoid($_exitcode)
  quit 125
end
quit $_exitcode
'''


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def run(binary, arguments, ring, output, debugger='gdb'):
    """Run the unchanged ELF as GDB's child; sample once and preserve exit status.

    The caller owns output as bounded execution interchange and imports it into
    SQLite. GDB/stdout are inherited by the already retained public launch log.
    """
    import shutil
    binary = Path(binary).resolve(strict=True)
    with binary.open('rb') as stream:
        if stream.read(4) != b'\x7fELF':
            raise ValueError('Sampler requires the actual ELF, not a wrapper')
    debugger = Path(shutil.which(debugger) or debugger).resolve(strict=True)
    output = Path(output)
    output.mkdir(exist_ok=False)
    commands = output/'commands.gdb'
    commands.write_text(GDB_COMMANDS)
    watch = PendingAckWatch()
    result = dict(schema=1, kind='parent_debugger_stall_observation',
                  threshold_seconds=watch.threshold, max_stack_samples=1,
                  player_binary=dict(path=str(binary), sha256=digest(binary)),
                  debugger=dict(path=str(debugger), sha256=digest(debugger)),
                  arguments=arguments, command_file_sha256=digest(commands),
                  sample=None, observations=0, success=False, historical_watchdog_resolved=False,
                  deadline_or_ownership_changed=False)
    child = None
    old_handlers = {}
    def stop(signum, frame):
        raise SystemExit(128 + signum)
    try:
        for sig in (signal.SIGTERM, signal.SIGINT):
            old_handlers[sig] = signal.signal(sig, stop)
        child = subprocess.Popen([str(debugger), '-q', '-nx', '-batch', '-x', str(commands),
                                  '--args', str(binary), *arguments],
                                 env=dict(os.environ, DEBUGINFOD_URLS=''))
        result['debugger_pid'] = child.pid
        error = None
        while child.poll() is None:
            if error is None:
                try:
                    observed = ring.read()
                    if observed is None:
                        watch.reset_interval()
                    else:
                        sample = watch.observe(*observed, time.monotonic())
                        result['observations'] += 1
                        if sample:
                            sample['ring'] = str(ring.path)
                            result['sample'] = sample
                            child.send_signal(signal.SIGINT)
                except (ValueError, OSError, KeyError, json.JSONDecodeError) as exc:
                    # Ambiguous evidence disables sampling, never stops the player.
                    error = str(exc)
                    result['observation_error'] = error
            time.sleep(.05)
        result['exit_code'] = child.returncode
        result['peak_pending_seconds'] = watch.peak_seconds
        result['player_binary_stable'] = digest(binary) == result['player_binary']['sha256']
        result['success'] = (error is None and result['observations'] > 0
                             and result['player_binary_stable'] and child.returncode == 0)
        return child.returncode if result['success'] else (child.returncode or 125)
    finally:
        if child is not None and child.poll() is None:
            child.terminate()
            try:
                child.wait(timeout=5)
            except subprocess.TimeoutExpired:
                child.kill()
                child.wait(timeout=5)
        for sig, handler in old_handlers.items():
            signal.signal(sig, handler)
        # Physical JSON is required collector interchange, not durable evidence.
        (output/'result.json').write_text(json.dumps(result, indent=2)+'\n')

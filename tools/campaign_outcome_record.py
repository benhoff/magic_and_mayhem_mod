"""Bounded helper snapshots; disk latency must not set ordinary-input cadence."""
from pathlib import Path
import threading
import time


class AtomicSnapshotWriter:
    MAX_BYTES = 8 * 1024**2

    def __init__(self, path, write=None):
        self.path = Path(path)
        self._write = write or self._atomic_write
        self._condition = threading.Condition()
        self._pending = None
        self._stopping = False
        self._error = None
        self._submitted = self._written = self._coalesced = 0
        self._max_write_ms = 0
        self._worker = threading.Thread(target=self._run, daemon=True)
        self._worker.start()

    def _atomic_write(self, payload):
        temporary = self.path.with_suffix('.tmp')
        temporary.write_bytes(payload)
        temporary.replace(self.path)

    def _check(self):
        if self._error is not None:
            raise RuntimeError('Outcome snapshot persistence failed: '+str(self._error)) from self._error

    def submit(self, payload):
        if not isinstance(payload, bytes) or len(payload) > self.MAX_BYTES:
            raise ValueError('Invalid bounded immutable outcome snapshot')
        with self._condition:
            self._check()
            if self._stopping:
                raise RuntimeError('Outcome snapshot writer has retired')
            self._coalesced += self._pending is not None
            self._pending = payload
            self._submitted += 1
            self._condition.notify()

    def statistics(self):
        with self._condition:
            return dict(policy='coalesced-immutable-atomic-v1', submitted=self._submitted,
                        written=self._written, coalesced=self._coalesced,
                        max_write_ms=self._max_write_ms, pending_limit=1)

    def _run(self):
        while True:
            with self._condition:
                self._condition.wait_for(lambda: self._pending is not None or self._stopping)
                if self._pending is None:
                    return
                payload, self._pending = self._pending, None
            started = time.monotonic()
            try:
                self._write(payload)
            except Exception as error:
                with self._condition:
                    self._error = error
                    self._pending = None
                    self._condition.notify_all()
                return
            with self._condition:
                self._written += 1
                self._max_write_ms = max(self._max_write_ms, (time.monotonic()-started)*1000)

    def close(self):
        with self._condition:
            self._stopping = True
            self._condition.notify_all()
        self._worker.join()
        with self._condition:
            self._check()

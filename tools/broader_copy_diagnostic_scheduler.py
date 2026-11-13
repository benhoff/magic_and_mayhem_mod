"""A child-only host policy for bounded COPY observation, with no engine changes."""
import hashlib
import os
from pathlib import Path
import threading
import time
from rendering_pilot import require

POLICY = 'copy-diagnostic-owner-scheduling-v1'


class CopyDiagnosticScheduler:
    def __init__(self, root, binary, sha256, policy):
        self.root, self.binary, self.sha256 = Path(root).resolve(), Path(binary).resolve(), sha256
        require(policy.get('policy') == POLICY and policy.get('main_nice') == 19
                and type(policy.get('cpu')) is int and policy['cpu'] in os.sched_getaffinity(0)
                and policy.get('child_only') is True, 'Unreviewed COPY observation scheduling policy')
        self.policy = dict(policy)
        self.records, self.monitors = [], []
        self.ready = threading.Event()

    def _identity(self, child):
        require(child.poll() is None, 'Native child exited before scheduling')
        proc = Path('/proc') / str(child.pid)
        require((proc / 'exe').resolve() == self.binary, 'COPY scheduling child executable changed')
        status = (proc / 'status').read_text().splitlines()
        uid = next(line.split()[1:] for line in status if line.startswith('Uid:'))
        require(uid and all(int(value) == os.getuid() for value in uid), 'COPY scheduling child UID differs')

    def _snapshot(self, pid):
        result = {}
        for task in (Path('/proc') / str(pid) / 'task').iterdir():
            try:
                tid = int(task.name)
                result[task.name] = dict(nice=os.getpriority(os.PRIO_PROCESS, tid),
                                        affinity=sorted(os.sched_getaffinity(tid)))
            except ProcessLookupError:
                pass
        return result

    def _observe(self, child, marker, record):
        try:
            while child.poll() is None and not marker.is_file():
                time.sleep(.01)
            require(marker.is_file(), 'Native child exited before COPY diagnostic constructor')
            self._identity(child)
            cpu = self.policy['cpu']
            before = os.getpriority(os.PRIO_PROCESS, child.pid)
            affinity = os.sched_getaffinity(child.pid)
            threads = self._snapshot(child.pid)
            require(before == 0 and affinity == {cpu}
                    and any(int(tid) != child.pid and t['nice'] == 0 and t['affinity'] == [cpu]
                            for tid, t in threads.items()), 'COPY owner/worker scheduling precondition differs')
            os.setpriority(os.PRIO_PROCESS, child.pid, self.policy['main_nice'])
            after = os.getpriority(os.PRIO_PROCESS, child.pid)
            require(after == 19 and os.sched_getaffinity(child.pid) == {cpu}, 'COPY owner scheduling was not applied')
            record.update(applied=True, applied_host_ns=time.monotonic_ns(), main_nice_before=before,
                          main_nice_after=after, main_affinity=[cpu], threads_before=threads)
        except Exception as error:
            record.update(applied=False, error=repr(error))
        finally:
            self.ready.set()

    def wrap(self, popen):
        def launch(command, *args, **kwargs):
            direct = isinstance(command, (list, tuple)) and bool(command)
            direct = direct and Path(str(command[0])).is_absolute() and Path(str(command[0])).resolve() == self.binary
            if not direct:
                return popen(command, *args, **kwargs)
            require(not self.records, 'COPY scheduling permits only one exact Native child')
            require(not kwargs.get('shell', False), 'COPY scheduling requires a direct argv launch')
            require(hashlib.sha256(self.binary.read_bytes()).hexdigest() == self.sha256,
                    'COPY scheduling Native binary identity changed')
            environment = kwargs.get('env', {})
            require(environment.get('MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE') == 'all', 'COPY scheduling requires all-copy observation')
            directory = Path(environment.get('MNM_NATIVE_COPY_DIAGNOSTICS', ''))
            require(directory.is_absolute() and directory.resolve().is_relative_to(self.root / 'working'),
                    'COPY scheduling diagnostic directory is not owned')
            marker = directory / 'copies.jsonl'
            require(not marker.exists(), 'COPY scheduling constructor marker already exists')
            wrapped = ['taskset', '--cpu-list', str(self.policy['cpu']), *map(str, command)]
            child = popen(wrapped, *args, **kwargs)
            record = dict(pid=child.pid, command=list(map(str, command)), wrapped_command=wrapped,
                          diagnostic_marker=str(marker), applied=False, policy=dict(self.policy))
            self.records.append(record)
            monitor = threading.Thread(target=self._observe, args=(child, marker, record), daemon=True)
            self.monitors.append(monitor)
            monitor.start()
            return child
        return launch

    def receipt(self):
        for monitor in self.monitors:
            monitor.join(timeout=1)
        require(len(self.records) == 1 and self.ready.is_set() and self.records[0]['applied'],
                'COPY observation scheduling lacks one successful owned-child receipt')
        return dict(policy=POLICY, launches=list(self.records), Native_sources_changed=False,
                    actions_caps_or_deadlines_changed=False, parent_scheduling_changed=False,
                    scope='Pin the exact Native child owner and inherited standard workers to one allowed CPU; '
                          'lower only the main thread to nice19 after the existing diagnostic constructor marker. '
                          'External driver thread affinities remain explicit. No input, draw, pixel, queue bound, '
                          'controller action/wait or response deadline changes. Complete live required COPY proof remains mandatory.')

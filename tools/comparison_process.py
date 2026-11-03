"""Own comparison process groups through completion, including failed commands."""
import os
import signal
import subprocess


def alive(group):
    try:
        os.killpg(group, 0)
        return True
    except ProcessLookupError:
        return False


class Processes:
    def __init__(self):
        self.groups = []

    def run(self, command, *, cwd, stdout, timeout):
        child = subprocess.Popen(command, cwd=cwd, stdout=stdout,
                                 stderr=subprocess.STDOUT, start_new_session=True)
        self.groups.append(child.pid)
        try:
            code = child.wait(timeout=timeout)
            if code:
                raise subprocess.CalledProcessError(code, command)
        finally:
            if alive(child.pid):
                os.killpg(child.pid, signal.SIGTERM)
                try:
                    child.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(child.pid, signal.SIGKILL)
                    child.wait(timeout=5)
                if alive(child.pid):
                    os.killpg(child.pid, signal.SIGKILL)
        if not self.stopped():
            raise ValueError('Comparison process group still exists; retain output')

    def stopped(self):
        return all(not alive(group) for group in self.groups)

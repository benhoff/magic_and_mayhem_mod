"""Own a private software-test X server; let Xvfb allocate its display atomically."""
from contextlib import contextmanager
import os
from pathlib import Path
import select
import subprocess


@contextmanager
def private_display(log_path):
    reader, writer = os.pipe()
    process = None
    try:
        with Path(log_path).open('x') as log:
            process = subprocess.Popen(
                ['Xvfb', '-displayfd', str(writer), '-screen', '0', '1280x1024x24',
                 '-nolisten', 'tcp', '-ac'],
                pass_fds=(writer,), stdout=log, stderr=subprocess.STDOUT)
            os.close(writer)
            writer = None
            if not select.select([reader], [], [], 30)[0]:
                raise RuntimeError('Private Xvfb did not become ready within 30 seconds')
            display = os.read(reader, 64).decode().strip()
            if not display.isdecimal() or process.poll() is not None:
                raise RuntimeError('Private Xvfb failed to allocate a display')
            environment = dict(os.environ, DISPLAY=':' + display)
            environment.pop('XAUTHORITY', None)
            yield environment
    finally:
        os.close(reader)
        if writer is not None:
            os.close(writer)
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()

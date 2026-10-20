#!/usr/bin/env python3
"""Run one automated scene control on an atomically allocated private X server."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from terrain_validation_display import private_display


def main():
    arguments = sys.argv[1:]
    if arguments and arguments[0] == '-a':
        arguments = arguments[1:]
    if not arguments:
        raise ValueError('Supply a test command')
    parent = os.environ.get('MNM_SCENE_DISPLAY_EVIDENCE')
    temporary = None
    if parent:
        Path(parent).mkdir(parents=True, exist_ok=True)
        out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    else:
        temporary = tempfile.TemporaryDirectory(prefix='mnm-scene-display-')
        out = Path(temporary.name)
    try:
        with private_display(out / 'xvfb.log') as environment:
            environment.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
            return subprocess.run(arguments, env=environment, check=False).returncode
    finally:
        if temporary:
            temporary.cleanup()


if __name__ == '__main__':
    raise SystemExit(main())

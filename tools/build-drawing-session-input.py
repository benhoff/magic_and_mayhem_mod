#!/usr/bin/env python3
"""Build bounded original-window XTest schedule runner, separate from game hooks."""
import shlex,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def build():
    root=ROOT/'working/build/drawing-session-input';root.mkdir(parents=True,exist_ok=True);binary=root/'mnm-drawing-session-input'
    flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','xcb','x11','xtst'],text=True))
    subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-session-input.cpp'),str(ROOT/'apps/qt-shell/window_host.cpp'),*flags,'-o',str(binary)],check=True);return binary
if __name__=='__main__':print(build())

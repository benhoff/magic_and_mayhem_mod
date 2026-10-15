#!/usr/bin/env python3
"""Start the existing Qt shell with an independently prepared controls source."""
import os
import subprocess
import sys
from rts_controls_source import ROOT, prepare_source

def main():
    source=prepare_source(os.environ.get('MNM_MENU_GAME_SOURCE',ROOT/'working/game-nocd'))
    environment=dict(os.environ,MNM_RTS_CONTROLS='1',MNM_VERIFY_ORIGINALS='1',
                     MNM_MENU_GAME_SOURCE=str(source))
    print('Controls launch source: '+str(source),flush=True)
    return subprocess.run([ROOT/'tools/run-qt-shell.sh',*sys.argv[1:]],cwd=ROOT,env=environment).returncode

if __name__=='__main__':raise SystemExit(main())

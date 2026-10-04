#!/usr/bin/env python3
"""Export pinned voice retirement and wrapper destruction without running the game."""
import importlib.util
from pathlib import Path
import subprocess
REPO=Path(__file__).resolve().parents[1]
def main():
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('audio_export',REPO/'tools/export-audio-support.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.RANGES.update(retire_voice=(0x4de090,0x4de1d0),wrapper_destructors=(0x56de50,0x56df60),source_cleanup=(0x5700e0,0x570140),wrapper_reset=(0x572150,0x572180),schedule_unlink=(0x571d20,0x571d80))
        root=module.export(REPO/'working/game-nocd/Chaos.exe')
        print(f'Audio retirement evidence: {root}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()

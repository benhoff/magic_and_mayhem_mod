#!/usr/bin/env python3
"""Export hash-guarded primary controls and manager lifecycle, without game execution."""
import importlib.util
from pathlib import Path
import subprocess
REPO=Path(__file__).resolve().parents[1]

def main():
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('audio_export',REPO/'tools/export-audio-support.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.RANGES.update(primary_get_volume=(0x56fd10,0x56fd2d),primary_set_volume=(0x56fd30,0x56fd4d),
                             primary_start=(0x56fd50,0x56fd83),manager_disable=(0x56fd90,0x56fe76),
                             manager_lifecycle_helpers=(0x56de90,0x56df60))
        module.SITES.update({0x56e4ff:'Primary.GetVolume saves startup volume at manager +0x258',
                            0x56e520:'Primary.Play looping after successful startup GetVolume',
                            0x56e661:'Primary.SetVolume restores saved startup volume; result ignored',
                            0x56fd27:'Primary.GetVolume, guarded by manager +0x1c',
                            0x56fd47:'Primary.SetVolume, guarded by manager +0x1c',
                            0x56fd73:'Primary.Play looping; +0x18 set only for zero result',
                            0x56fe62:'Primary.Stop after selected secondary retirement; +0x18 cleared before call'})
        root=module.export(REPO/'working/game-nocd/Chaos.exe')
        print(f'Primary audio evidence: {root}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)

if __name__=='__main__':main()

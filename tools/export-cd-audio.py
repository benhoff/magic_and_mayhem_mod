#!/usr/bin/env python3
"""Export guarded original CD/MCI control ranges without executing the game."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable',type=Path,default=ROOT/'working/game-nocd/Chaos.exe')
    args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('cd_static_export',ROOT/'tools/export-audio-support.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.RANGES={'cd_control_ranges':(0x481330,0x482d00),'cd_open':(0x481330,0x481490),
                       'cd_initialize':(0x481490,0x481770),'cd_close':(0x481770,0x481930),
                       'cd_controls':(0x481930,0x4822a0),'cd_play':(0x4822a0,0x482910),
                       'cd_mixer':(0x482910,0x482d00)}
        module.SITES={}
        output=module.export(args.executable)
        manifest=json.loads((output/'manifest.json').read_text())
        manifest['sources']={path:hashlib.sha256((ROOT/path).read_bytes()).hexdigest() for path in ['tools/export-cd-audio.py','tools/export-audio-support.py']}
        manifest['scope']='Selected original CD/MCI static ranges and direct callers; labels provisional, no original execution or native playback validation.'
        manifest['original_manifest_verified_before']=True
        (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    manifest['original_manifest_verified_after']=True
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(output,flush=True)
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Hash-pinned filename/open-boundary assembly and cabinet inventory; no game."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import shutil
REPO=Path(__file__).resolve().parents[1]
def main():
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('audio_export',REPO/'tools/export-audio-support.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.RANGES.update(source_cache=(0x56f400,0x56f86e),source_pool=(0x56e910,0x56ef00),wave_open=(0x58fbb0,0x58fd86),file_open=(0x4a1360,0x4a1550),crt_fopen=(0x59cae2,0x59cb60))
        executable=REPO/'working/game-nocd/Chaos.exe';root=module.export(executable)
        cabinet=REPO/'original/MagicMayhem_CD/data1.cab'
        manifest=json.loads((root/'manifest.json').read_text())
        if shutil.which('unshield'):
            listing=subprocess.run(['unshield','l',str(cabinet)],capture_output=True,text=True,timeout=60)
            (root/'cabinet-list.log').write_text(listing.stdout+listing.stderr);listing.check_returncode()
            manifest['cabinet_inventory']='fresh unshield listing'
        else:
            log=REPO/'working/manifests/unshield.log'
            (root/'cabinet-list.log').write_bytes(log.read_bytes())
            manifest['cabinet_inventory']='historical extraction log; unshield unavailable, not a fresh cabinet listing'
            manifest['historical_log_sha256']=hashlib.sha256(log.read_bytes()).hexdigest()
        manifest['cabinet_sha256']=hashlib.sha256(cabinet.read_bytes()).hexdigest();manifest['artifacts']['cabinet-list.log']=hashlib.sha256((root/'cabinet-list.log').read_bytes()).hexdigest();(root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        print(f'Filename boundary evidence: {root}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()

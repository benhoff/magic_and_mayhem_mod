#!/usr/bin/env python3
"""Exercise the PE32 bridge in a dedicated Wine prefix with a synthetic engine."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
REPO=Path(__file__).resolve().parent.parent

def load(name,file):
    spec=importlib.util.spec_from_file_location(name,REPO/file)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def main():
    dll=load('shadow_build','tools/build-shadow-bridge.py').build(True)
    parent=REPO/'working/tests/neighbor-shadow';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game';game.mkdir();(game/'shadow').mkdir()
    shutil.copy2(dll,game/dll.name)
    # Test the same added-import loader path used by the staged game.
    data=(dll.parent/'selftest.exe').read_bytes()
    (game/'selftest.exe').write_bytes(load('shadow_stage','tools/prepare-shadow-experiment.py').add_import(data))
    (root/'manifest.json').write_text(json.dumps({'origin':'synthetic_wine_harness'})+'\n')
    env=os.environ.copy();env['WINEPREFIX']=str(REPO/'working/tests/shadow-wine')
    env['WINEDEBUG']='-all';env['MNM_SHADOW_CALLS']='1';env['MNM_SHADOW_DIR']='shadow'
    with (root/'wine.log').open('w') as log:
        subprocess.run(['wine',str(game/'selftest.exe')],cwd=game,env=env,stdout=log,stderr=log,check=True,timeout=60)
    subprocess.run([str(REPO/'tools/compare-neighbor-shadow.py'),str(root)],check=True)
    if len(list((game/'shadow').glob('world-*.bin')))!=1 or len(list((game/'shadow').glob('expansion-*.bin')))!=1:
        raise ValueError('Expected exactly one completed sample')
    print(f'Synthetic PE32 bridge passed: {root}')
if __name__=='__main__':main()

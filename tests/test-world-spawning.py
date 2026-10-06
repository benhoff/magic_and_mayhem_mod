#!/usr/bin/env python3
"""Production Qt spawn controls and frozen native admission; synthetic fixtures."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('spawning_fixture',ROOT/'tests/test-native-ani-motion.py')
a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
def main():
    test,sandbox=map(lambda p:str(Path(p).resolve()),sys.argv[1:])
    def run(binary,*args):
        p=subprocess.run([binary,*map(str,args)],capture_output=True,text=True)
        assert p.returncode==0,(args,p.stdout,p.stderr)
        assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr
        return p.stdout
    with tempfile.TemporaryDirectory(prefix='mnm-scene-spawning-') as folder:
        root=Path(folder);frozen=root/'map';frozen.write_bytes(a.segment.old.fixture.fixture())
        ani=root/'movement.ani';ani.write_bytes(a.ani());saved=root/'spawned'
        print(run(test,frozen,ani,saved).strip())
        state=json.loads(run(sandbox,'inspect-json',saved));assert len(state['entities'])==2 and state['pending']==1
        seed=root/'seed';state=json.loads(run(sandbox,'spawn-terrain-ani',frozen,ani,8,seed,1,1,1))
        assert state['tick']==0 and state['pending']==0 and len(state['entities'])==1
        print('Multi-policy initial scene and synthetic spawn/order checkpoint passed')
if __name__=='__main__':main()

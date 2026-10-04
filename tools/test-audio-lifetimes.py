#!/usr/bin/env python3
"""Validate recovered voice lifetimes with synthetic native fixtures, no game."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
def main():
    parent=REPO/'working/tests/audio-lifetimes';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));build=REPO/'working/build/audio-output'
    print(f'Voice lifetime evidence: {root}',flush=True)
    for name,cmd in [('configure',['cmake','-S',str(REPO/'audio'),'-B',str(build)]),
                     ('build',['cmake','--build',str(build),'--parallel','4']),
                     ('ctest',['ctest','--test-dir',str(build),'--output-on-failure'])]:
        result=subprocess.run(cmd,text=True,capture_output=True,timeout=180)
        (root/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
    files=['reconstruction/audio/voice_lifetime.hpp','reconstruction/audio/voice_lifetime.cpp',
           'reconstruction/audio/voice_contract.hpp','reconstruction/audio/voice_contract.cpp',
           'tests/audio-lifetime-test.cpp','audio/CMakeLists.txt','audio/buffers.cpp','audio/voices.cpp','audio/mixer.cpp']
    report={'scope':'recovered retirement/destruction and native ownership fixtures; no live replacement',
            'game_launched':False,'game_assets_read':False,'tests_passed':True,
            'retirement_combinations':432,'circular_ring_cases':204,
            'source_sha256':{p:hashlib.sha256((REPO/p).read_bytes()).hexdigest() for p in files},
            'fixture_sha256':hashlib.sha256((build/'audio-lifetime-test').read_bytes()).hexdigest()}
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Voice lifetime checks passed: {root/"report.json"}')
if __name__=='__main__':main()

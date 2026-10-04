#!/usr/bin/env python3
"""Validate Qt asset/profile input through the native audio manager, no game."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
def main():
    parent=REPO/'working/tests/audio-native-manager';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));build=REPO/'working/build/audio-output'
    print(f'Native manager evidence: {root}',flush=True)
    for name,cmd in [('configure',['cmake','-S',str(REPO/'audio'),'-B',str(build)]),
                     ('build',['cmake','--build',str(build),'--parallel','4']),
                     ('ctest',['ctest','--test-dir',str(build),'--output-on-failure'])]:
        result=subprocess.run(cmd,text=True,capture_output=True,timeout=180)
        (root/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
    files=['assets/profile.hpp','assets/profile.cpp','reconstruction/audio/native_manager_backend.hpp','reconstruction/audio/native_manager_backend.cpp','tests/audio-native-manager-test.cpp','assets/CMakeLists.txt','reconstruction/audio/manager_lifecycle.hpp','reconstruction/audio/manager_lifecycle.cpp','tests/audio-manager-lifecycle-test.cpp','reconstruction/audio/manager_contract.hpp','reconstruction/audio/manager_contract.cpp','reconstruction/audio/dsound_setup.cpp','reconstruction/audio/manager_configuration.hpp','reconstruction/audio/manager_configuration.cpp','tests/audio-manager-configuration-test.cpp','reconstruction/audio/source_cache.hpp','reconstruction/audio/source_cache.cpp',
           'reconstruction/audio/voice_admission.hpp','reconstruction/audio/voice_admission.cpp',
           'reconstruction/audio/voice_scheduler.hpp','reconstruction/audio/voice_scheduler.cpp',
           'reconstruction/audio/voice_lifetime.hpp',
           'reconstruction/audio/voice_contract.hpp','reconstruction/audio/voice_contract.cpp',
           'tests/audio-source-cache-test.cpp','tests/audio-admission-test.cpp',
           'audio/wave_loader.cpp','audio/wave_loader.hpp','assets/asset_file.cpp','assets/path_resolver.cpp',
           'reconstruction/audio/voice_lifetime.cpp','audio/CMakeLists.txt','audio/buffers.hpp','audio/buffers.cpp','audio/voices.cpp','audio/mixer.cpp']
    report={'scope':'Qt-backed ASCII profile/WAV input through recovered manager to native duplicate PCM and lifecycle teardown; no live replacement',
            'game_launched':False,'game_assets_read':False,'tests_passed':True,
            'source_sha256':{p:hashlib.sha256((REPO/p).read_bytes()).hexdigest() for p in files},
            'fixture_sha256':hashlib.sha256((build/'audio-native-manager-test').read_bytes()).hexdigest()}
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Native manager checks passed: {root/"report.json"}')
if __name__=='__main__':main()

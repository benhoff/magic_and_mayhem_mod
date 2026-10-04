#!/usr/bin/env python3
"""Check native voice dispatch and x86 COM routing with fixtures, never Chaos.exe."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
REPO = Path(__file__).resolve().parents[1]

def main():
    parent=REPO/'working/tests/audio-bridge'
    parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    print(f'Voice bridge evidence: {root}',flush=True)
    build=REPO/'working/build/audio-output'
    for name,cmd in [('configure',['cmake','-S',str(REPO/'audio'),'-B',str(build)]),
                     ('build',['cmake','--build',str(build),'--parallel','4']),
                     ('ctest',['ctest','--test-dir',str(build),'--output-on-failure'])]:
        result=subprocess.run(cmd,text=True,capture_output=True,timeout=180)
        (root/(name+'.log')).write_text(result.stdout+result.stderr)
        print(result.stdout+result.stderr,end='',flush=True)
        result.check_returncode()
    spec=importlib.util.spec_from_file_location('audio_build',REPO/'tools/build-audio-bridge.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    dll=module.build(True)
    shutil.copy2(dll,root/dll.name);shutil.copy2(dll.parent/'selftest.exe',root/'selftest.exe')
    channel=root/'voices.bin'
    env={key:value for key,value in os.environ.items() if not key.startswith('MNM_')}
    env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all',
               MNM_AUDIO_CHANNEL='Z:'+str(channel).replace('/','\\'))
    with (root/'server.log').open('w') as log:
        server=subprocess.Popen([str(build/'mnm-audio-server'),'--channel',str(channel),'--silent','--seconds','15'],stdout=log,stderr=log)
        try:
            deadline=time.monotonic()+5
            while True:
                if channel.exists() and channel.stat().st_size==128+16*1024*1024:
                    with channel.open('rb') as source:
                        source.seek(80)
                        if source.read(4)==b'\x01\0\0\0':break
                if server.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Voice server not ready')
                time.sleep(0.01)
            with (root/'wine.log').open('w') as wine_log:
                subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=wine_log,stderr=wine_log,check=True,timeout=30)
            # Snapshot command acknowledgements before orderly server teardown.
            with channel.open('rb') as source: header=source.read(128)
        finally:
            server.terminate();server.wait(timeout=5)
    env['MNM_AUDIO_STALE_TEST']='1'
    with (root/'stale-host.log').open('w') as log:
        subprocess.run(['wine',str(root/'selftest.exe')],cwd=root,env=env,stdout=log,stderr=log,check=True,timeout=10)
    report={'scope':'native dispatcher plus PE32 COM fixture, not live game replacement',
            'tests_passed':True,'game_launched':False,'game_assets_read':False,'audible_output':False,
            'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
            'fixture_sha256':hashlib.sha256((root/'selftest.exe').read_bytes()).hexdigest(),
            'primary_fixture_sha256':hashlib.sha256((build/'audio-primary-test').read_bytes()).hexdigest(),
            'native_fixture_sha256':hashlib.sha256((build/'audio-voice-bridge-test').read_bytes()).hexdigest(),
            'source_sha256':{str(p.relative_to(REPO)):hashlib.sha256(p.read_bytes()).hexdigest()
                             for p in [*sorted((REPO/'runtime/audio').glob('*')),REPO/'audio/voice_bridge.cpp',REPO/'audio/voice_bridge.hpp',REPO/'tests/audio-voice-bridge-test.cpp']},
            'last_request':int.from_bytes(header[16:20],'little'),
            'last_ack':int.from_bytes(header[64:68],'little'),
            'native_devices_selected':int.from_bytes(header[84:88],'little'),
            'fallback_attempts':int.from_bytes(header[88:92],'little'),
            'checks':['native_pcm_and_controls','COM_method_slots_stdcall','primary_metadata','primary_volume_roundtrip','primary_format_validation',
                      'primary_stop_preserves_secondary_playback','manager_gate_failure_order_matrices',
                      'secondary_lock_unlock','duplicate_source_retirement','independent_status',
                      'invalid_unlock_and_flags','unsupported_pitch_and_seek','reference_lifetime',
                      'stale_host_timeout_and_permanent_retirement']}
    assert report['last_request']==report['last_ack'] and report['last_ack']>10,report
    assert report['native_devices_selected']==1 and report['fallback_attempts']==0,report
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Voice bridge checks passed: {root/"report.json"}')

if __name__=='__main__':main()

#!/usr/bin/env python3
"""Freeze a real native music host while the PE producer polls status."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import signal
import struct
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
def wait(predicate,process,seconds):
    deadline=time.monotonic()+seconds
    while not predicate():
        if process.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Music liveness fixture exit/deadline')
        time.sleep(.005)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/audio');p.add_argument('--claims',type=Path,required=True);args=p.parse_args()
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    if any(sha(ROOT/path)!=digest for path,digest in sources.items()):raise ValueError('Music watchdog declaration drift')
    parent=ROOT/'working/tests/native-music-liveness';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],scope='Pure monotonic32 rollover/retirement helper and actual PE32 producer versus frozen/resumed silent native music worker. Status failure276, local failure notification8 and permanent no-revival are fixture assertions; no physical device or original window/driver equivalence.')
    server=None;wine=None;frozen=False
    try:
        catalog=out/'catalog';load('liveness_fixture','tools/test-native-music-service.py').fixture(catalog);inputs={str(path.relative_to(out)):sha(path) for path in catalog.iterdir()}
        with (out/'build.log').open('x') as log:
            subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/music-liveness-test.c'),'-o',str(out/'watchdog')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
            subprocess.run([str(out/'watchdog')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=5);report['helper_passed']=True
            subprocess.run(['cmake','--build',str(args.build.resolve()),'--target','mnm-music-server','-j','4'],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
            subprocess.run(['python3',str(ROOT/'tools/build-audio-bridge.py'),'--selftest'],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
        build=ROOT/'working/build/audio-bridge-selftest'
        for name in ['MnmAudio.dll','music-fault-selftest.exe']:shutil.copy2(build/name,out/name)
        channel=out/'music.bin';ready=out/'producer-ready';done=out/'producer-retired'
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='dsound=')
        def words():return struct.unpack('<40I',channel.read_bytes()[:160])
        with (out/'server.log').open('x') as log,(out/'wine.log').open('x') as wine_log:
            server=subprocess.Popen([str(args.build.resolve()/'mnm-music-server'),'--channel',str(channel),'--catalog',str(catalog),'--silent','--seconds','40','--report',str(out/'native.json')],env=env,stdout=log,stderr=subprocess.STDOUT)
            wait(lambda:channel.exists() and channel.stat().st_size==1024 and words()[21]==1,server,10)
            env.update(MNM_MUSIC_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_MUSIC_FAULT_READY='Z:'+str(ready).replace('/','\\'),MNM_MUSIC_FAULT_DONE='Z:'+str(done).replace('/','\\'))
            wine=subprocess.Popen(['wine',str(out/'music-fault-selftest.exe')],cwd=out,env=env,stdout=wine_log,stderr=subprocess.STDOUT)
            wait(ready.exists,wine,20);server.send_signal(signal.SIGSTOP);frozen=True;time.sleep(.05);before=words()
            wait(done.exists,wine,12);after=words()
            if before[20]!=after[20] or after[21]!=1:raise ValueError('Host was not frozen with readiness still published')
            report['frozen_heartbeat']=after[20];report['frozen_ready']=after[21]
            server.send_signal(signal.SIGCONT);frozen=False;wine.wait(timeout=5);report['fixture_returncode']=wine.returncode
            if wine.returncode:raise ValueError('PE music watchdog fixture failed: '+str(wine.returncode))
            wait(lambda:words()[20]!=after[20],server,3);report['heartbeat_resumed']=True
            server.wait(timeout=45)
        native=json.loads((out/'native.json').read_text());report.update(native=native,inputs=inputs,inputs_stable=all(sha(out/path)==digest for path,digest in inputs.items()),sources_stable=all(sha(ROOT/path)==digest for path,digest in sources.items()))
        if native['producer_features']!=31 or native['unsupported_cd_calls'] or native['legacy_other_mci_calls'] or native['failed_requests'] or native['ready_after_stop'] is not False or not report['inputs_stable'] or not report['sources_stable']:raise ValueError('Watchdog fixture native census/retirement/source guards')
        report['success']=True
    except Exception as error:report['error']=str(error);raise
    finally:
        if server and frozen:server.send_signal(signal.SIGCONT)
        if wine and wine.poll() is None:wine.terminate();wine.wait(timeout=5)
        if server and server.poll() is None:server.terminate();server.wait(timeout=5)
        subprocess.run(['wineserver','-k'],env=dict(os.environ,WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=False,timeout=10)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()

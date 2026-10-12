#!/usr/bin/env python3
"""Drive the actual PE32 native CD wrapper against the native music worker."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
import struct
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--build',type=Path,default=ROOT/'working/build/audio');parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    if any(sha(ROOT/path)!=digest for path,digest in sources.items()):raise ValueError('Native MCI declaration drift')
    parent=ROOT/'working/tests/native-mci-cd';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],scope='Actual PE32 selected CD/MCI wrapper with native host and synthetic TOC/PCM; exported callback/LastError/notification fixture only, no original game/live hook/window/driver equivalence.')
    server=None
    try:
        catalog=out/'catalog';load('mci_fixture','tools/test-native-music-service.py').fixture(catalog);inputs={str(path.relative_to(out)):sha(path) for path in catalog.iterdir()}
        with (out/'build.log').open('x') as log:
            subprocess.run(['cmake','--build',str(args.build.resolve()),'--target','mnm-music-server','-j','4'],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=90)
            subprocess.run(['python3',str(ROOT/'tools/build-audio-bridge.py'),'--selftest'],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=90)
        build=ROOT/'working/build/audio-bridge-selftest';shutil.copy2(build/'MnmAudio.dll',out/'MnmAudio.dll');shutil.copy2(build/'music-selftest.exe',out/'music-selftest.exe')
        channel=out/'music.bin';env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='dsound=')
        with (out/'server.log').open('x') as log:
            server=subprocess.Popen([str(args.build.resolve()/'mnm-music-server'),'--channel',str(channel),'--catalog',str(catalog),'--silent','--seconds','25','--report',str(out/'native.json')],env=env,stdout=log,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+10
            def ready():
                if not channel.exists():return False
                data=channel.read_bytes()
                return len(data)==1024 and data[:8]==b'MNMMUS01' and struct.unpack_from('<I',data,84)[0]==1
            while not ready():
                if server.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native MCI host startup failed')
                time.sleep(.005)
            env['MNM_MUSIC_CHANNEL']='Z:'+str(channel).replace('/','\\')
            with (out/'wine.log').open('x') as wine:
                result=subprocess.run(['wine',str(out/'music-selftest.exe')],cwd=out,env=env,stdout=wine,stderr=subprocess.STDOUT,timeout=20)
            report['fixture_returncode']=result.returncode
            if result.returncode:raise ValueError('PE32 native CD fixture failed: '+str(result.returncode))
            server.wait(timeout=30)
        native=json.loads((out/'native.json').read_text());report.update(native=native,inputs=inputs,inputs_stable=all(sha(out/path)==digest for path,digest in inputs.items()),sources_stable=all(sha(ROOT/path)==digest for path,digest in sources.items()))
        if native['ready_after_stop'] is not False or native['producer_features']!=31 or native['unsupported_cd_calls']!=1 or native['legacy_other_mci_calls']!=1 or not report['inputs_stable'] or not report['sources_stable']:
            raise ValueError('Native CD fixture census/retirement/source guards')
        report['success']=True
    except Exception as error:report['error']=str(error);raise
    finally:
        if server and server.poll() is None:server.terminate();server.wait(timeout=5)
        subprocess.run(['wineserver','-k'],env=dict(os.environ,WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=False,timeout=10)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()

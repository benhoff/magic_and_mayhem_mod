#!/usr/bin/env python3
"""Independent synthetic prepared-CD/PCM, lifecycle and decoder checks."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import wave

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def fixture(root):
    root.mkdir();rows=[];expected={}
    for track,start in [(2,2),(3,77)]:
        # Independent signed stereo ramp, with opposing channels and no clipping.
        pcm=b''.join(struct.pack('<hh',(frame*37)%20001-10000,10000-(frame*53)%20001) for frame in range(44100))
        path=root/f'Track{track:02}.wav'
        with wave.open(str(path),'wb') as target:target.setparams((2,2,44100,0,'NONE','not compressed'));target.writeframes(pcm)
        rows.append(dict(track=track,file=path.name,start_sector=start,end_sector=start+75,frames=44100,pcm_bytes=len(pcm),pcm_sha256=hashlib.sha256(pcm).hexdigest(),wav_sha256=sha(path)))
        expected[track]=hashlib.sha256(pcm).hexdigest()
    manifest=dict(schema=1,success=True,format=dict(channels=2,bits=16,rate=44100,byte_order='little'),tracks=rows)
    (root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');return manifest,expected
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--build',type=Path,default=ROOT/'working/build/audio');parser.add_argument('--claims',type=Path);args=parser.parse_args()
    parent=ROOT/'working/tests/native-music-service';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    if not args.claims:
        args.claims=out/'prospective-claims.json'
        subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','HOST.native-music-service','--scenario','native-music-service-synthetic-20261012','--output',str(args.claims)],check=True)
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    if any(sha(ROOT/name)!=digest for name,digest in sources.items()):raise ValueError('Native music declaration drift')
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],scope='Independent synthetic strict CD catalog refusals, generation-bound fake output lifecycle/device recovery and real silent Qt full PCM decode/loop/pause/seek/stop. No original assets/engine/driver, physical device fault/listening, live MCI or timing equivalence.')
    try:
        root=out/'catalog';manifest,expected=fixture(root);inputs={str(path.relative_to(out)):sha(path) for path in root.iterdir()}
        executable=args.build.resolve()/'music-service-test'
        with (out/'build.log').open('x') as log:subprocess.run(['cmake','--build',str(args.build.resolve()),'--target','music-service-test','-j','4'],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
        cases=[]
        def check_catalog(path,valid,name):
            result=subprocess.run([str(executable),str(path),'--catalog-check'],capture_output=True,text=True,check=True,timeout=20);observed=json.loads(result.stdout)
            if observed['valid']!=valid:raise ValueError('Catalog admission differs: '+name)
            cases.append(dict(name=name,valid=valid,observed=observed))
        check_catalog(root,True,'valid')
        mutations=[('schema',lambda value:value.update(schema=2)),('format',lambda value:value['format'].update(rate=22050)),
                   ('track-number',lambda value:value['tracks'][0].update(track=1)),('span',lambda value:value['tracks'][1].update(start_sector=78)),
                   ('pcm-bytes',lambda value:value['tracks'][0].update(pcm_bytes=4)),('frames',lambda value:value['tracks'][0].update(frames=1)),
                   ('path-escape',lambda value:value['tracks'][0].update(file='../Track02.wav')),('hash',lambda value:value['tracks'][0].update(wav_sha256='0'*64)),
                   ('no-audio',lambda value:value.update(tracks=[])),('negative',lambda value:value['tracks'][0].update(start_sector=-1))]
        for name,mutate in mutations:
            target=out/name;shutil.copytree(root,target);value=copy.deepcopy(manifest);mutate(value);(target/'manifest.json').write_text(json.dumps(value));check_catalog(target,False,name)
        for name in ('wrong-header','truncated','symlink-outside'):
            target=out/name;shutil.copytree(root,target);path=target/'Track02.wav'
            if name=='wrong-header':data=bytearray(path.read_bytes());struct.pack_into('<H',data,22,1);path.write_bytes(data)
            elif name=='truncated':path.write_bytes(path.read_bytes()[:-4])
            else:path.unlink();path.symlink_to(root/'Track02.wav')
            check_catalog(target,False,name)
        with (out/'execution.log').open('x') as log:
            subprocess.run([str(executable),str(root),str(out/'native.json')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
        native=json.loads((out/'native.json').read_text());pcm=native['qt_decode']
        if not native['success'] or pcm['pcm_s16le_stereo_44100_sha256']!=expected[2] or pcm['audio_frames']!=44100 or pcm['audio_bytes']!=176400:
            raise ValueError('Qt music PCM differs from independent source')
        report.update(catalog_cases=cases,native=native,decoded_matches_source=True,inputs=inputs,
                      inputs_stable=all(sha(out/name)==digest for name,digest in inputs.items()),sources_stable=all(sha(ROOT/name)==digest for name,digest in sources.items()),executable_sha256=sha(executable))
        if not report['inputs_stable'] or not report['sources_stable']:raise ValueError('Music validation inputs/sources changed')
        report['success']=True
    except Exception as error:report['error']=str(error);raise
    finally:(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()

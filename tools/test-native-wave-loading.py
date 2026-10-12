#!/usr/bin/env python3
"""Compare session-native WAV transport with independent installed PCM inputs."""
import argparse
import hashlib
import json
import mmap
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import time
import wave

ROOT=Path(__file__).resolve().parents[1]
SIZE=128+16*1024*1024
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sounds-root',type=Path,default=ROOT/'working/game-nocd/Sounds')
    args=parser.parse_args();sounds=args.sounds_root.resolve()
    if not sounds.is_relative_to(ROOT/'working') or not sounds.is_dir():parser.error('Use a Sounds directory under working/')
    parent=ROOT/'working/tests/native-wave-loading';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Native WAV evidence: '+str(out),flush=True)
    def run(name,command):
        p=subprocess.run(command,text=True,capture_output=True,timeout=180)
        (out/(name+'.log')).write_text(p.stdout+p.stderr);p.check_returncode();return p
    report={'schema':1,'success':False,'scope':'Silent host WAV transport and independent installed PCM comparison; no original engine, Wine or physical output execution.',
            'game_launched':False,'audible_output':False,'cases':[],'input_root':str(sounds)}
    server=None;mapping=None;channel_file=None
    try:
        run('manifest-before',[str(ROOT/'tools/original-manifest.sh'),'verify'])
        declaration=out/'prospective-claims.json'
        run('declaration',['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','HOST.native-wave-loading',
                           '--scenario','native-wave-loading-installed-20261012','--output',str(declaration)])
        declared=json.loads(declaration.read_text());report.update(claims=declared['claims'],sources=declared['sources'])
        inputs={str(p):sha(p) for p in sounds.iterdir() if p.is_file() and p.suffix.lower()=='.wav'}
        if not inputs:raise ValueError('No installed WAV inputs')
        report['inputs']=inputs
        build=ROOT/'working/build/audio-output'
        run('configure',['cmake','-S',str(ROOT/'audio'),'-B',str(build)])
        run('build',['cmake','--build',str(build),'--target','mnm-audio-server','audio-voice-bridge-test','--parallel','4'])
        binary=build/'mnm-audio-server';report['binary_sha256']=sha(binary)
        run('bridge-regression',[str(build/'audio-voice-bridge-test')])
        channel=out/'voices.bin'
        with (out/'server.log').open('w') as log:
            env=os.environ.copy();env['QT_QPA_PLATFORM']='offscreen'
            server=subprocess.Popen([str(binary),'--channel',str(channel),'--silent','--sounds-root',str(sounds),'--seconds','30'],env=env,stdout=log,stderr=log)
            deadline=time.monotonic()+5
            while not channel.exists() or channel.stat().st_size!=SIZE:
                if server.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native wave server did not start')
                time.sleep(.005)
            channel_file=channel.open('r+b');mapping=mmap.mmap(channel_file.fileno(),SIZE)
            while struct.unpack_from('<I',mapping,80)[0]!=1:
                if server.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native wave broker not ready')
                time.sleep(.001)
            if mapping[:8]!=b'MNMAUD01' or struct.unpack_from('<II',mapping,8)!=(4,SIZE) or struct.unpack_from('<I',mapping,124)[0]!=3:
                raise RuntimeError('Invalid native wave channel')
            sequence=0;total=0
            def load(name):
                nonlocal sequence
                raw=name.encode('ascii');sequence+=1
                struct.pack_into('<11I',mapping,20,17,0,0,len(raw),0,0,0,0,0,0,0)
                mapping[128:128+len(raw)]=raw
                # Single producer with one outstanding request. The PE32 peer
                # separately validates acquire/release publication and ABI.
                struct.pack_into('<I',mapping,16,sequence)
                deadline=time.monotonic()+1
                while struct.unpack_from('<I',mapping,64)[0]!=sequence:
                    if server.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native wave transport deadline')
                    time.sleep(.001)
                status,count=struct.unpack_from('<II',mapping,68)
                metadata=struct.unpack_from('<7I',mapping,96)
                if status:
                    if count or any(metadata):raise ValueError('Failed wave exposed stale result')
                    return status,metadata,b''
                if not count or count>16*1024*1024 or metadata[0]!=count:raise ValueError('Invalid returned native PCM bounds')
                return status,metadata,mapping[128:128+count]
            def compare(request,path):
                nonlocal total
                with wave.open(str(path),'rb') as source:
                    pcm=source.readframes(source.getnframes());channels=source.getnchannels();bits=source.getsampwidth()*8;rate=source.getframerate()
                alignment=channels*(bits//8);byte_rate=rate*alignment
                expected=(len(pcm),rate,byte_rate,1|(channels<<16),alignment|(bits<<16),((len(pcm)*1000)&0xffffffff)//byte_rate,0)
                status,metadata,native=load(request)
                if status or metadata!=expected or native!=pcm:raise ValueError('Native wave mismatch: '+request)
                total+=len(pcm)
                report['cases'].append({'request':request,'input':str(path),'bytes':len(pcm),'metadata':list(metadata),'pcm_sha256':hashlib.sha256(native).hexdigest(),'matched':True})
            for name in sorted(inputs):compare(Path(name).name,Path(name))
            for leaf in ('Picview1','Picview2','Death','Ranged combat'):
                candidates=[Path(p) for p in inputs if Path(p).name.lower()==(leaf+'.wav').lower()]
                if len(candidates)!=1:raise ValueError('Quoted fixture input is missing or ambiguous: '+leaf)
                compare("'"+leaf+"'.wav",candidates[0])
            for name in ('../outside.wav','not-an-installed-file.wav'):
                status,_,_=load(name)
                if status!=0x80004005:raise ValueError('Expected native file rejection: '+name)
                report['cases'].append({'request':name,'expected_rejection':True,'status':status})
            report.update(compared_pcm_bytes=total,last_request=sequence,last_ack=struct.unpack_from('<I',mapping,64)[0],
                          native_selections=struct.unpack_from('<I',mapping,84)[0],fallbacks=struct.unpack_from('<I',mapping,88)[0])
            server.terminate();server.wait(timeout=5);server=None
        report['sources_unchanged']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
        report['inputs_stable']=all(sha(Path(p))==h for p,h in inputs.items())
        if not report['sources_unchanged'] or not report['inputs_stable'] or report['fallbacks']!=0:raise RuntimeError('Inputs/sources changed or audio fallback')
        report['success']=True
    except Exception as error:
        report['error']=str(error)
    finally:
        if server is not None:server.terminate();server.wait(timeout=5)
        if mapping is not None:mapping.close()
        if channel_file is not None:channel_file.close()
        try:run('manifest-after',[str(ROOT/'tools/original-manifest.sh'),'verify']);report['original_manifest_verified_after']=True
        except Exception as error:report.update(success=False,manifest_error=str(error))
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())

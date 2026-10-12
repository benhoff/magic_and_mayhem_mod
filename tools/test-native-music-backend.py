#!/usr/bin/env python3
"""Actual native music loss, paused replacement, default switch and retained seek."""
import argparse,hashlib,json,os,shlex,struct,subprocess,tempfile,time,wave
from pathlib import Path
from private_audio_backend import PrivateAudioBackend
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args();declared=json.loads(args.claims.read_text())
    assert all(sha(ROOT/path)==value for path,value in declared['sources'].items())
    parent=ROOT/'working/tests/native-music-backend';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declared['claims'],sources=declared['sources'],trace=[])
    def run(label,argv):
        with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in argv],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240)
    catalog=out/'catalog';catalog.mkdir();pcm=b''.join(struct.pack('<hh',(i*37)%8001-4000,(i*61)%4001-2000) for i in range(44100*6))
    with wave.open(str(catalog/'Track02.wav'),'wb') as f:f.setnchannels(2);f.setsampwidth(2);f.setframerate(44100);f.writeframes(pcm)
    (catalog/'manifest.json').write_text(json.dumps(dict(schema=1,success=True,format=dict(channels=2,bits=16,rate=44100,byte_order='little'),tracks=[dict(track=2,start_sector=150,end_sector=600,pcm_bytes=len(pcm),frames=len(pcm)//4,file='Track02.wav',wav_sha256=sha(catalog/'Track02.wav'))])))
    build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-music-backend','-B',build,'-DCMAKE_BUILD_TYPE=Release']);run('build',['cmake','--build',build,'--target','native-music-backend-test','-j4'])
    deps={'tests/native-music-backend/CMakeLists.txt','tools/test-native-music-backend.py','tools/private_audio_backend.py'}
    for path in build.rglob('*.o.d'):
        for name in shlex.split(path.read_text().replace('\\\n',' ')):
            if Path(name).is_absolute():
                source=Path(name).resolve()
                if source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):deps.add(str(source.relative_to(ROOT)))
    if deps-set(declared['sources']):raise ValueError('Undeclared music compiler inputs: '+repr(sorted(deps-set(declared['sources']))))
    backend=PrivateAudioBackend(out);player=None;status=out/'status.json';command=out/'command.json';final=out/'final.json';serial=0
    def wait(predicate,label):
        deadline=time.monotonic()+10
        while time.monotonic()<deadline:
            r=json.loads(status.read_text()) if status.exists() else {}
            if predicate(r):report['trace'].append(dict(event=label,state=r));return r
            if player.poll() is not None:raise RuntimeError('Music ended: '+label+' '+json.dumps(r))
            time.sleep(.03)
        raise TimeoutError(label+' '+json.dumps(r))
    def control(op,**kw):
        nonlocal serial
        serial+=1;tmp=out/'command.tmp';tmp.write_text(json.dumps(dict(serial=serial,operation=op,**kw)));tmp.replace(command)
        return wait(lambda r:r.get('command_ack')==serial,'Control '+op)
    try:
        with backend:
            backend.add_sink('a');env=dict(backend.environment,QT_QPA_PLATFORM='offscreen',QT_MEDIA_BACKEND='ffmpeg')
            with (out/'player.log').open('x') as log:
                player=subprocess.Popen([str(build/'native-music-backend-test'),str(catalog),str(status),str(command),str(final)],env=env,stdout=log,stderr=subprocess.STDOUT)
                initial=wait(lambda r:r.get('phase')==3 and r.get('position_ms',0)>350,'Initial active music')
                assert initial['track']==2 and initial['loop'] and initial['level']==12345
                backend.remove_sink('a');lost=wait(lambda r:r.get('phase')==6 and r.get('losses')==1,'Actual music output loss')
                control('pause');time.sleep(.2);frozen=json.loads(status.read_text());assert frozen['position_ms']==lost['position_ms']
                report['trace'].append(dict(event='Frozen unavailable cursor',state=frozen))
                assert frozen['audio_device_available'] is False,'Removed music endpoint reported available'
                backend.add_sink('b');paused=wait(lambda r:r.get('phase')==4 and r.get('recoveries')==1,'Replacement remains caller paused')
                assert abs(paused['position_ms']-lost['position_ms'])<=50 and paused['level']==12345 and paused['loop'] and paused['track']==2
                control('seek',milliseconds=1200);time.sleep(.1);seeked=json.loads(status.read_text());assert abs(seeked['position_ms']-1200)<=50
                backend.add_sink('c');switched=wait(lambda r:r.get('phase')==4 and r.get('recoveries')==2 and r.get('losses')==2,'Non-null default switch remains paused')
                assert abs(switched['position_ms']-1200)<=50 and switched['level']==12345 and switched['loop'] and switched['track']==2
                control('resume');playing=wait(lambda r:r.get('phase')==3 and r.get('position_ms',0)>1450,'Music resumes retained seek')
                assert playing['completions']==0 and playing['audio_device_available'] and playing['error']==''
                serial+=1;command.write_text(json.dumps(dict(serial=serial,operation='finish')));player.wait(timeout=5);assert player.returncode==0
                report['native']=json.loads(final.read_text());assert report['native']['success'] and report['native']['closed']
                backend.finish_graph();report['success']=True
    except Exception as error:report['error']=repr(error);raise
    finally:
        if player is not None and player.poll() is None:player.terminate();player.wait(timeout=5)
        backend.close();report['backend_cleanup']=backend.report;report['compiler_dependencies']=sorted(deps)
        report['sources_stable']=all(sha(ROOT/path)==value for path,value in declared['sources'].items())
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_stable'];print('Native music backend passed',out/'report.json')
if __name__=='__main__':main()

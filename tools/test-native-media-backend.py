#!/usr/bin/env python3
"""Real Qt media recovery under an isolated private software audio graph."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
import time
import wave
from private_audio_backend import PrivateAudioBackend

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True)
    declaration=json.loads(parser.parse_args().claims.read_text())
    if any(sha(ROOT/path)!=value for path,value in declaration['sources'].items()):
        raise ValueError('Backend recovery declaration drift')
    parent=ROOT/'working/tests/native-media-backend';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=declaration['sources'],
                scope='Actual private software endpoint topology around Qt media; exact complete decoded PCM/RGBA and retained logical position/pause. No physical hardware, audible waveform/tail or original-driver equivalence.',cases=[])
    def run(label,command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(value) for value in command],stdout=log,stderr=subprocess.STDOUT,
                           cwd=ROOT,check=True,timeout=180)
    pcm=b''.join(struct.pack('<hh',(i*37)%8001-4000,(i*61)%4001-2000) for i in range(22050*3))
    sound=out/'sound.wav'
    with wave.open(str(sound),'wb') as wav:
        wav.setnchannels(2);wav.setsampwidth(2);wav.setframerate(22050);wav.writeframes(pcm)
    rgb=b''.join(bytes(((x*7+n*31)&255,(y*11+n*13)&255,(x+y+n*29)&255))
                 for n in range(30) for y in range(32) for x in range(48))
    rgba=b''.join(rgb[i:i+3]+b'\xff' for i in range(0,len(rgb),3))
    raw=out/'frames.rgb';raw.write_bytes(rgb);movie=out/'movie.avi'
    run('fixture',['ffmpeg','-v','error','-f','rawvideo','-pixel_format','rgb24','-video_size','48x32',
                   '-framerate','10','-i',raw,'-i',sound,'-c:v','ffv1','-pix_fmt','bgr0','-c:a','pcm_s16le',movie])
    build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-media-backend','-B',build,'-DCMAKE_BUILD_TYPE=Release'])
    run('build',['cmake','--build',build,'-j4'])
    dependencies={'tests/native-media-backend/CMakeLists.txt','tools/test-native-media-backend.py','tools/private_audio_backend.py'}
    for depfile in build.rglob('*.o.d'):
        for name in shlex.split(depfile.read_text().replace('\\\n',' ')):
            if not Path(name).is_absolute():continue
            dependency=Path(name).resolve()
            if dependency.is_relative_to(ROOT) and not dependency.is_relative_to(ROOT/'working'):
                dependencies.add(str(dependency.relative_to(ROOT)))
    if dependencies-set(declaration['sources']):
        raise ValueError('Undeclared compiler dependencies: '+repr(sorted(dependencies-set(declaration['sources']))))
    try:
        for kind,asset in [('movie',movie),('sound',sound)]:
            case_dir=out/kind;case_dir.mkdir();backend=PrivateAudioBackend(case_dir)
            process=None
            try:
                with backend:
                    env=dict(backend.environment,QT_QPA_PLATFORM='offscreen',QT_MEDIA_BACKEND='ffmpeg')
                    status=case_dir/'status.json';command=case_dir/'command.json';final=case_dir/'final.json'
                    with (case_dir/'player.log').open('x') as log:
                        process=subprocess.Popen([str(build/'native-media-backend-test'),kind,str(asset),str(status),str(command),str(final)],
                                                 env=env,stdout=log,stderr=subprocess.STDOUT)
                        def current():
                            if not status.exists():return {}
                            return json.loads(status.read_text())
                        def wait(predicate,description,seconds=10):
                            deadline=time.monotonic()+seconds
                            while time.monotonic()<deadline:
                                value=current()
                                if predicate(value):return value
                                if process.poll() is not None:raise RuntimeError(description+': player ended '+json.dumps(value))
                                time.sleep(.02)
                            raise TimeoutError(description+': '+json.dumps(current()))
                        initial=wait(lambda v:v.get('waiting_for_audio_output') is True,'Initial actual endpoint absence')
                        time.sleep(.2);initial=current();assert initial['position_ms']==0 and initial['audio_frames']==0 and initial['video_frames']==0,initial
                        backend.add_sink('a')
                        arrival=wait(lambda v:v.get('output_recoveries',0)>=1 and v.get('position_ms',0)>=300,'Actual endpoint arrival')
                        backend.remove_sink('a')
                        wait(lambda v:v.get('waiting_for_audio_output') is True,'Actual active endpoint loss')
                        time.sleep(.1);lost=current();frozen=lost['position_ms'];time.sleep(.2)
                        assert current()['position_ms']==frozen,current()
                        def pause(sequence,value):
                            temporary=case_dir/'command-next.json';temporary.write_text(json.dumps(dict(schema=1,sequence=sequence,operation='pause',value=value)))
                            os.replace(temporary,command)
                            return wait(lambda v:v.get('command_ack')==sequence,'Owned playback command ACK')
                        pause(1,True);backend.add_sink('b')
                        recovered=wait(lambda v:v.get('output_recoveries',0)>=2 and v.get('waiting_for_audio_output') is False,'Actual replacement endpoint')
                        assert recovered['caller_paused'] and recovered['position_ms']==frozen,recovered
                        backend.add_sink('c')
                        switched=wait(lambda v:v.get('default_output_changes',0)>=1,'Distinct actual default switch')
                        assert switched['caller_paused'] and switched['position_ms']==frozen,switched
                        pause(2,False);process.wait(timeout=12)
                        result=json.loads(final.read_text());assert process.returncode==0 and result['success'],result
                        assert result['audio_bytes']==len(pcm) and result['audio_s16le_stereo_22050_sha256']==hashlib.sha256(pcm).hexdigest(),result
                        if kind=='movie':assert result['video_frames']==30 and result['video_rgba_sha256']==hashlib.sha256(rgba).hexdigest(),result
                        report['cases'].append(dict(kind=kind,initial=initial,arrival=arrival,lost=lost,recovered=recovered,switched=switched,final=result))
                    backend.finish_graph()
            finally:
                if process and process.poll() is None:
                    process.terminate()
                    try:process.wait(timeout=5)
                    except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
                backend.close()
                report.setdefault('backend_cleanup',[]).append(backend.report)
        report['success']=True
    except Exception as error:
        report['error']=str(error);raise
    finally:
        report['sources_stable']=all(sha(ROOT/path)==value for path,value in declaration['sources'].items())
        report['compiled_sources']={name:sha(ROOT/name) for name in sorted(dependencies)}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_stable'];print('Actual software endpoint recovery passed',out/'report.json')

if __name__=='__main__':main()

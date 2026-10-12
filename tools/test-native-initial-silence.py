#!/usr/bin/env python3
"""Native voice/movie/file-sound/music progress and attachment from an empty graph."""
import argparse,hashlib,json,mmap,os,shlex,struct,subprocess,tempfile,time,wave
from pathlib import Path
from private_audio_backend import PrivateAudioBackend
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True)
    declared=json.loads(parser.parse_args().claims.read_text())
    if any(sha(ROOT/path)!=value for path,value in declared['sources'].items()):raise ValueError('Silence declaration drift')
    parent=ROOT/'working/tests/native-initial-silence';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declared['claims'],sources=declared['sources'],cases=[])
    def run(label,argv):
        with (out/(label+'.log')).open('x') as log:subprocess.run([str(v) for v in argv],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240)
    def wav(path,rate,frames):
        pcm=b''.join(struct.pack('<hh',(i*37)%8001-4000,(i*61)%4001-2000) for i in range(frames))
        with wave.open(str(path),'wb') as f:f.setnchannels(2);f.setsampwidth(2);f.setframerate(rate);f.writeframes(pcm)
        return pcm
    sounds=out/'sounds';sounds.mkdir();voice_pcm=wav(sounds/'Test.wav',48000,48000*3)
    pcm=wav(out/'sound.wav',22050,22050*3)
    rgb=b''.join(bytes(((x*7+n*31)&255,(y*11+n*13)&255,(x+y+n*29)&255)) for n in range(30) for y in range(32) for x in range(48))
    rgba=b''.join(rgb[i:i+3]+b'\xff' for i in range(0,len(rgb),3));(out/'frames.rgb').write_bytes(rgb)
    run('fixture',['ffmpeg','-v','error','-f','rawvideo','-pixel_format','rgb24','-video_size','48x32','-framerate','10','-i',out/'frames.rgb','-i',out/'sound.wav','-c:v','ffv1','-pix_fmt','bgr0','-c:a','pcm_s16le',out/'movie.avi'])
    catalog=out/'catalog';catalog.mkdir();music_pcm=wav(catalog/'Track02.wav',44100,44100*3)
    (catalog/'manifest.json').write_text(json.dumps(dict(schema=1,success=True,format=dict(channels=2,bits=16,rate=44100,byte_order='little'),tracks=[dict(track=2,start_sector=150,end_sector=375,pcm_bytes=len(music_pcm),frames=len(music_pcm)//4,file='Track02.wav',wav_sha256=sha(catalog/'Track02.wav'))])))
    build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-initial-silence','-B',build,'-DCMAKE_BUILD_TYPE=Release'])
    run('build',['cmake','--build',build,'--target','initial-voice-test','initial-media-test','initial-music-test','-j4'])
    deps={'tests/native-initial-silence/CMakeLists.txt','tools/test-native-initial-silence.py','tools/private_audio_backend.py'}
    for path in build.rglob('*.o.d'):
        for name in shlex.split(path.read_text().replace('\\\n',' ')):
            if Path(name).is_absolute():
                source=Path(name).resolve()
                if source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):deps.add(str(source.relative_to(ROOT)))
    if deps-set(declared['sources']):raise ValueError('Undeclared compiler inputs: '+repr(sorted(deps-set(declared['sources']))))
    try:
        for kind in ['voice','movie','sound','music']:
            case=out/kind;case.mkdir();backend=PrivateAudioBackend(case);process=None;channel=None;handle=None
            entry=dict(kind=kind);report['cases'].append(entry)
            try:
                with backend:
                    env=dict(backend.environment,QT_QPA_PLATFORM='offscreen',QT_MEDIA_BACKEND='ffmpeg')
                    status=case/'status.json';command=case/'command.json';final=case/'final.json';channel_path=case/'voice.bin'
                    if kind=='voice':argv=[build/'initial-voice-test',channel_path,status,command,final,sounds]
                    elif kind=='music':argv=[build/'initial-music-test',catalog,status,final]
                    else:argv=[build/'initial-media-test','quiet-'+kind,out/(kind+'.avi' if kind=='movie' else 'sound.wav'),status,command,final]
                    with (case/'player.log').open('x') as log:
                        process=subprocess.Popen([str(v) for v in argv],env=env,stdout=log,stderr=subprocess.STDOUT)
                        def current():return json.loads(status.read_text()) if status.exists() else {}
                        def wait(predicate,label):
                            deadline=time.monotonic()+10
                            while time.monotonic()<deadline:
                                value=current()
                                if predicate(value):return value
                                if process.poll() is not None:raise RuntimeError(label+': process ended '+json.dumps(value))
                                time.sleep(.02)
                            raise TimeoutError(label+': '+json.dumps(current()))
                        if kind=='voice':
                            initial=wait(lambda v:v.get('ready') and v.get('output_silent'),'Virtual voice ready')
                            handle=channel_path.open('r+b');channel=mmap.mmap(handle.fileno(),128+16*1024*1024);serial=0;calls=[]
                            assert struct.unpack_from('<I',channel,124)[0]==31
                            def request(op,voice=0,value=0,length=0,payload=b'',fmt=None,flags=0):
                                nonlocal serial
                                words=[op,voice,value&0xffffffff,length,0,0,0,0,0,0,flags]
                                if fmt:words[4:10]=fmt
                                struct.pack_into('<11I',channel,20,*words)
                                if payload:channel[128:128+len(payload)]=payload
                                serial+=1;struct.pack_into('<I',channel,16,serial);deadline=time.monotonic()+2
                                while struct.unpack_from('<I',channel,64)[0]!=serial:
                                    if time.monotonic()>deadline:raise TimeoutError('Quiet voice ACK')
                                    time.sleep(.001)
                                code,value=struct.unpack_from('<2I',channel,68);calls.append(dict(operation=op,voice=voice,status=code,value=value));assert code==0,calls[-1];return value
                            count=request(17,length=8,payload=b'Test.wav');assert count==len(voice_pcm) and channel[128:128+count]==voice_pcm
                            fmt=[48000,2,16,4,192000,1]
                            short=request(1,length=48000,fmt=fmt,flags=0xea);request(2,short,length=48000,payload=bytes(48000))
                            loop=request(1,length=len(voice_pcm),fmt=fmt,flags=0xea);request(2,loop,length=len(voice_pcm),payload=bytes(len(voice_pcm)))
                            request(13,value=1);request(5,short);request(5,loop,value=1)
                            time.sleep(.4);assert request(10,short)==0 and request(10,loop)==5
                            before=request(20,loop);assert before>0;request(22,loop)
                            backend.add_sink('a');attached=wait(lambda v:v.get('output_available') and not v.get('output_silent'),'Virtual voice attachment')
                            time.sleep(.15);after=request(20,loop);assert after>before and request(10,short)==0
                            request(6,loop);request(4,short);request(4,loop);request(14)
                            command.write_text(json.dumps(dict(schema=1,operation='stop')));process.wait(timeout=5)
                            entry.update(initial=initial,attached=attached,cursor_before=before,cursor_after=after,calls=calls)
                        else:
                            flag='initial_silent_output'
                            initial=wait(lambda v:v.get(flag) and v.get('position_ms',0)>=400,'Silent decoder progress')
                            entry['initial']=initial
                            assert not initial['audio_device_available'] and initial['audio_frames']>0,initial
                            backend.add_sink('a');attached=wait(lambda v:v.get('audio_device_available') and not v.get(flag),'Silent decoder attachment')
                            process.wait(timeout=6);entry.update(initial=initial,attached=attached)
                        result=json.loads(final.read_text());assert process.returncode==0 and result['success'],result
                        if kind in ['movie','sound']:
                            assert result['audio_bytes']==len(pcm) and result['audio_s16le_stereo_22050_sha256']==hashlib.sha256(pcm).hexdigest(),result
                            if kind=='movie':assert result['video_frames']==30 and result['video_rgba_sha256']==hashlib.sha256(rgba).hexdigest(),result
                        if kind=='music':assert result['audio_bytes']==len(music_pcm) and result['pcm_s16le_stereo_44100_sha256']==hashlib.sha256(music_pcm).hexdigest(),result
                        entry['final']=result
                    backend.finish_graph()
            finally:
                if process and process.poll() is None:
                    process.terminate()
                    try:process.wait(timeout=5)
                    except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
                if channel:channel.close()
                if handle:handle.close()
                backend.close();entry['backend_cleanup']=backend.report
        report['success']=True
    except Exception as error:report['error']=repr(error);raise
    finally:
        report['sources_stable']=all(sha(ROOT/path)==value for path,value in declared['sources'].items())
        report['compiled_sources']={name:sha(ROOT/name) for name in sorted(deps)}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_stable'];print('Native silent startup passed',out/'report.json')
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Capture actual native voice output with an owned private endpoint monitor."""
import argparse,hashlib,json,math,mmap,os,shlex,struct,subprocess,tempfile,time,wave
from pathlib import Path
from private_audio_backend import PrivateAudioBackend
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True)
    declared=json.loads(parser.parse_args().claims.read_text())
    if any(sha(ROOT/path)!=value for path,value in declared['sources'].items()):raise ValueError('Waveform declaration drift')
    parent=ROOT/'working/tests/native-voice-waveforms';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declared['claims'],sources=declared['sources'],cases=[])
    def run(label,argv):
        with (out/(label+'.log')).open('x') as log:subprocess.run([str(v) for v in argv],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240)
    build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-voice-waveforms','-B',build,'-DCMAKE_BUILD_TYPE=Release'])
    run('build',['cmake','--build',build,'--target','native-voice-waveform-test','-j4'])
    deps={'tests/native-voice-waveforms/CMakeLists.txt','tools/test-native-voice-waveforms.py','tools/private_audio_backend.py'}
    for path in build.rglob('*.o.d'):
        for name in shlex.split(path.read_text().replace('\\\n',' ')):
            if Path(name).is_absolute():
                source=Path(name).resolve()
                if source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):deps.add(str(source.relative_to(ROOT)))
    if deps-set(declared['sources']):raise ValueError('Undeclared compiler inputs: '+repr(sorted(deps-set(declared['sources']))))
    source=[((i*997)%22001-11000,(i*1571+431)%18001-9000) for i in range(128)]
    pcm=b''.join(struct.pack('<hh',*frame) for frame in source)
    sounds=out/'sounds';sounds.mkdir()
    with wave.open(str(sounds/'Test.wav'),'wb') as f:f.setnchannels(2);f.setsampwidth(2);f.setframerate(48000);f.writeframes(pcm)
    backend=PrivateAudioBackend(out);player=None;capture=None;mapping=None;channel=None;serial=0;calls=[]
    try:
        with backend:
            backend.add_sink('a');backend.command('set-sink-volume','mnm_private_a','100%')
            env=dict(backend.environment,QT_QPA_PLATFORM='offscreen',QT_MEDIA_BACKEND='ffmpeg')
            status=out/'status.json';command=out/'command.json';final=out/'final.json';path=out/'voice.bin'
            with (out/'player.log').open('x') as log:
                player=subprocess.Popen([str(build/'native-voice-waveform-test'),str(path),str(status),str(command),str(final),str(sounds)],env=env,stdout=log,stderr=subprocess.STDOUT)
                deadline=time.monotonic()+10
                while not status.exists():
                    if player.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native voice output not admitted')
                    time.sleep(.02)
                assert json.loads(status.read_text())['output_available']
                channel=path.open('r+b');mapping=mmap.mmap(channel.fileno(),128+16*1024*1024)
                def request(op,voice=0,value=0,length=0,payload=b'',fmt=None,flags=0):
                    nonlocal serial
                    words=[op,voice,value&0xffffffff,length,0,0,0,0,0,0,flags]
                    if fmt:words[4:10]=fmt
                    struct.pack_into('<11I',mapping,20,*words)
                    if payload:mapping[128:128+len(payload)]=payload
                    serial+=1;struct.pack_into('<I',mapping,16,serial);end=time.monotonic()+2
                    while struct.unpack_from('<I',mapping,64)[0]!=serial:
                        if time.monotonic()>end:raise TimeoutError('Waveform ACK')
                        time.sleep(.001)
                    code,result=struct.unpack_from('<2I',mapping,68);calls.append(dict(operation=op,voice=voice,status=code,value=result));assert code==0,calls[-1];return result
                count=request(17,length=8,payload=b'Test.wav');assert count==len(pcm) and mapping[128:128+count]==pcm
                fmt=[48000,2,16,4,192000,1]
                root=request(1,length=len(pcm),fmt=fmt,flags=0xea);request(2,root,length=len(pcm),payload=pcm)
                constant=request(1,length=len(pcm),fmt=fmt,flags=0xea);request(2,constant,length=len(pcm),payload=struct.pack('<hh',30000,30000)*128)
                request(13,value=1);request(5,root,value=1)
                def quantize(v):return max(-32768,min(32767,int(math.copysign(math.floor(abs(v)+.5),v))))
                specs=[('stereo',0,0,0,48000,False),('voice-gain',-2000,0,0,48000,False),('pan-left',0,-10000,0,48000,False),('pan-right',0,10000,0,48000,False),('master-gain',0,0,-2000,48000,False),('pitch',0,0,0,36000,False),('overlap-clipping',0,0,0,48000,True)]
                for label,volume,pan,master,frequency,overlap in specs:
                    request(8,root,value=volume);request(9,root,value=pan);request(11,value=master);request(18,root,value=frequency)
                    if overlap:request(5,constant,value=1)
                    time.sleep(.15) # Retire samples queued before the control acknowledgements.
                    audio=out/(label+'.s16');capture_error=(out/(label+'-capture.log')).open('x')
                    with audio.open('xb') as output:
                        capture=subprocess.Popen(['parec','--server',backend.server,'--device=mnm_private_a.monitor','--format=s16le','--rate=48000','--channels=2','--latency-msec=20'],env=env,stdout=output,stderr=capture_error)
                        end=time.monotonic()+2
                        while audio.stat().st_size<48000*4//3:
                            if capture.poll() is not None or time.monotonic()>end:raise RuntimeError('Monitor capture did not progress')
                            time.sleep(.02)
                        capture.terminate();capture.wait(timeout=3);capture=None
                    capture_error.close()
                    frames=list(struct.iter_unpack('<hh',audio.read_bytes()))
                    period=128*48000//math.gcd(frequency,128*48000)
                    expected=[]
                    for n in range(period):
                        position=n*frequency/48000;i=int(position)%128;fraction=position-math.floor(position)
                        pair=[]
                        for c in range(2):
                            value=source[i][c]+(source[(i+1)%128][c]-source[i][c])*fraction
                            gain=10**((volume+master)/2000)
                            if (c==0 and pan>0) or (c==1 and pan<0):gain*=0 if abs(pan)>=10000 else 10**(-abs(pan)/2000)
                            pair.append(quantize(value*gain+(30000 if overlap else 0)))
                        expected.append(tuple(pair))
                    # Unknown capture start latency is represented by a phase. Require
                    # a long contiguous window, rather than counting isolated matches.
                    matched=0;best_error=None;best=None;length=2048
                    for begin in range(0,max(0,len(frames)-length),256):
                        first=frames[begin]
                        phases=[i for i,pair in enumerate(expected) if max(abs(first[c]-pair[c]) for c in (0,1))<=1]
                        for phase in phases:
                            error=max(abs(frames[begin+n][c]-expected[(phase+n)%period][c]) for n in range(length) for c in (0,1))
                            if best_error is None or error<best_error:best_error=error;best=dict(start=begin,phase=phase)
                            if error<=1:matched=length;break
                        if matched:break
                    entry=dict(case=label,frames=len(frames),expected_period=period,matched_frames=matched,max_sample_error=best_error,window=best,capture_sha256=sha(audio));report['cases'].append(entry)
                    if matched!=length:raise AssertionError('Actual native waveform mismatch: '+json.dumps(entry))
                request(6,root);request(6,constant);request(4,root);request(4,constant)
                command.write_text(json.dumps(dict(schema=1,operation='stop')));player.wait(timeout=5);assert player.returncode==0
                report['native']=json.loads(final.read_text());assert report['native']['success'] and report['native']['ready_after_stop'] is False
                assert struct.unpack_from('<I',mapping,80)[0]==0
                backend.finish_graph();report['success']=True
    except Exception as error:report['error']=repr(error);raise
    finally:
        if capture is not None and capture.poll() is None:capture.terminate();capture.wait(timeout=3)
        if player is not None and player.poll() is None:player.terminate();player.wait(timeout=5)
        if mapping is not None:mapping.close()
        if channel is not None:channel.close()
        backend.close();report['calls']=calls;report['backend_cleanup']=backend.report
        report['compiler_dependencies']=sorted(deps)
        report['sources_stable']=all(sha(ROOT/path)==value for path,value in declared['sources'].items())
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_stable'];print('Native software endpoint waveforms passed',out/'report.json')
if __name__=='__main__':main()

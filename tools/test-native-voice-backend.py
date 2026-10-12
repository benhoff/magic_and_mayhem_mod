#!/usr/bin/env python3
"""Actual native voice version5 transport and consumed cursors under endpoint loss."""
import argparse
import hashlib
import json
import mmap
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
import time
import wave
from private_audio_backend import PrivateAudioBackend

ROOT=Path(__file__).resolve().parents[1]
SIZE=128+16*1024*1024
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True)
    declaration=json.loads(parser.parse_args().claims.read_text())
    if any(sha(ROOT/path)!=value for path,value in declaration['sources'].items()):raise ValueError('Voice backend declaration drift')
    parent=ROOT/'working/tests/native-voice-backend';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=declaration['sources'],
                scope='Actual native host version5 voice/PCM/control/Restore/consumed-cursor transport under private software endpoint loss/replacement/default switch; PE32 ABI, physical waveform and original equivalence remain separate.')
    def run(label,command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(value) for value in command],stdout=log,stderr=subprocess.STDOUT,cwd=ROOT,check=True,timeout=240)
    sounds=out/'sounds';sounds.mkdir();pcm=bytes(48000*3*4)
    with wave.open(str(sounds/'Test.wav'),'wb') as wav:
        wav.setnchannels(2);wav.setsampwidth(2);wav.setframerate(48000);wav.writeframes(pcm)
    build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-voice-backend','-B',build,'-DCMAKE_BUILD_TYPE=Release'])
    run('build',['cmake','--build',build,'--target','native-voice-backend-test','-j4'])
    dependencies={'tests/native-voice-backend/CMakeLists.txt','tools/test-native-voice-backend.py','tools/private_audio_backend.py'}
    for depfile in build.rglob('*.o.d'):
        for name in shlex.split(depfile.read_text().replace('\\\n',' ')):
            if not Path(name).is_absolute():continue
            path=Path(name).resolve()
            if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):dependencies.add(str(path.relative_to(ROOT)))
    if dependencies-set(declaration['sources']):raise ValueError('Undeclared compiler dependencies: '+repr(sorted(dependencies-set(declaration['sources']))))
    backend=PrivateAudioBackend(out);process=None;channel=None;handle=None
    try:
        with backend:
            backend.add_sink('a')
            channel_path=out/'voice.bin';status=out/'status.json';command=out/'command.json';final=out/'final.json'
            with (out/'player.log').open('x') as log:
                process=subprocess.Popen([str(build/'native-voice-backend-test'),str(channel_path),str(status),str(command),str(final),str(sounds)],
                                         env=backend.environment,stdout=log,stderr=subprocess.STDOUT)
                def current():return json.loads(status.read_text()) if status.exists() else {}
                def wait(predicate,description,seconds=10):
                    deadline=time.monotonic()+seconds
                    while time.monotonic()<deadline:
                        value=current()
                        if predicate(value):return value
                        if process.poll() is not None:raise RuntimeError(description+': voice host ended '+json.dumps(value))
                        time.sleep(.02)
                    raise TimeoutError(description+': '+json.dumps(current()))
                initial=wait(lambda v:v.get('ready') and v.get('output_available'),'Native voice ready')
                handle=channel_path.open('r+b');channel=mmap.mmap(handle.fileno(),SIZE)
                assert channel[:16]==b'MNMAUD01'+struct.pack('<II',5,SIZE)
                serial=0;requests=[]
                def request(op,voice=0,value=0,length=0,payload=b'',flags=0,format_=None,expected=0):
                    nonlocal serial
                    words=[op,voice,value & 0xffffffff,length,0,0,0,0,0,0,flags]
                    if format_:words[4:10]=format_
                    struct.pack_into('<11I',channel,20,*words)
                    if payload:channel[128:128+len(payload)]=payload
                    serial+=1;struct.pack_into('<I',channel,16,serial)
                    deadline=time.monotonic()+2
                    while struct.unpack_from('<I',channel,64)[0]!=serial:
                        if process.poll() is not None or time.monotonic()>deadline:raise TimeoutError('Native voice ACK')
                        time.sleep(.001)
                    code,result=struct.unpack_from('<2I',channel,68)
                    requests.append(dict(serial=serial,operation=op,voice=voice,status=code,value=result))
                    assert code==expected,requests[-1]
                    return result
                count=request(17,length=len(b'Test.wav'),payload=b'Test.wav')
                assert count==len(pcm) and channel[128:128+count]==pcm
                voice=request(1,length=count,flags=0xea,format_=[48000,2,16,4,192000,1])
                request(2,voice,length=count,payload=pcm);request(18,voice,value=40000)
                clone=request(3,voice);assert clone!=voice
                assert request(19,voice)==40000 and request(19,clone)==40000
                request(11,value=-2000);assert request(12)==((-2000)&0xffffffff)
                request(8,voice,value=-3000);request(9,voice,value=-5000)
                request(13,value=1);request(5,voice,value=1)
                time.sleep(.25);before_cursor=request(20,voice);assert before_cursor>0
                backend.remove_sink('a');lost=wait(lambda v:v.get('output_available') is False,'Actual native voice loss')
                time.sleep(.1);frozen=request(20,voice);time.sleep(.2);assert request(20,voice)==frozen
                assert request(10,voice)&2 and struct.unpack_from('<I',channel,80)[0]==1
                request(22,voice,expected=0x88780096)
                request(18,voice,value=45000);assert request(19,voice)==45000
                backend.add_sink('b');recovered=wait(lambda v:v.get('output_available') and v.get('output_recoveries',0)>=1,'Native replacement output')
                request(22,voice);assert request(19,voice)==45000 and request(19,clone)==40000
                assert request(12)==((-2000)&0xffffffff)
                time.sleep(.15);resumed=request(20,voice);assert resumed>frozen and request(10,voice)==5
                backend.add_sink('c');switched=wait(lambda v:v.get('output_available') and v.get('output_recoveries',0)>=2,'Native distinct default switch')
                assert request(19,voice)==45000 and request(19,clone)==40000
                time.sleep(.1);assert request(20,voice)>resumed
                request(6,voice);request(4,clone);request(4,voice);request(14)
                command.write_text(json.dumps(dict(schema=1,operation='stop')));process.wait(timeout=5)
                final_report=json.loads(final.read_text());assert process.returncode==0 and final_report['success'] and not final_report['ready_after_stop'],final_report
                assert final_report['consumed_frames']>0 and final_report['generated_frames']>=final_report['consumed_frames']
                report.update(initial=initial,lost=lost,recovered=recovered,switched=switched,final=final_report,
                              cursor_before_loss=before_cursor,cursor_frozen=frozen,cursor_resumed=resumed,requests=requests)
            backend.finish_graph();report['success']=True
    except Exception as error:
        report['error']=repr(error)
        if 'requests' in locals():report['requests']=requests
        for name in ['initial','lost','recovered','switched','before_cursor','frozen','resumed']:
            if name in locals():report[name]=locals()[name]
        raise
    finally:
        if process and process.poll() is None:
            process.terminate()
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
        if channel:channel.close()
        if handle:handle.close()
        backend.close();report['backend_cleanup']=backend.report
        report['sources_stable']=all(sha(ROOT/path)==value for path,value in declaration['sources'].items())
        report['compiled_sources']={name:sha(ROOT/name) for name in sorted(dependencies)}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_stable'];print('Actual native voice endpoint recovery passed',out/'report.json')

if __name__=='__main__':main()

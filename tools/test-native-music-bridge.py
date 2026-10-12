#!/usr/bin/env python3
"""Independent synthetic music wire peer against the actual native host worker."""
import argparse
import hashlib
import importlib.util
import json
import mmap
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import music_v1 as wire
spec=importlib.util.spec_from_file_location('music_fixture',ROOT/'tools/test-native-music-service.py');fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def wait(test,process,seconds=5):
    deadline=time.monotonic()+seconds
    while not test():
        if process.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native music peer deadline/exit')
        time.sleep(.002)
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--build',type=Path,default=ROOT/'working/build/audio');parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    if any(sha(ROOT/path)!=digest for path,digest in sources.items()):raise ValueError('Music transport declaration drift')
    parent=ROOT/'working/tests/native-music-transport';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],scope='Independent Python wire peer versus real silent native Qt music worker/server and synthetic strict CD catalog. Request/status/event/TOC/PCM/lifecycle guards, no game/original MCI/notifications or physical output.')
    process=None;mapping=None;handle=None
    try:
        catalog=out/'catalog';fixture.fixture(catalog);channel=out/'music.bin';native=out/'native.json';inputs={str(path.relative_to(out)):sha(path) for path in catalog.iterdir()}
        with (out/'build.log').open('x') as log:subprocess.run(['cmake','--build',str(args.build.resolve()),'--target','mnm-music-server','-j','4'],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
        with (out/'server.log').open('x') as log:
            process=subprocess.Popen([str(args.build.resolve()/'mnm-music-server'),'--channel',str(channel),'--catalog',str(catalog),'--silent','--seconds','12','--report',str(native)],stdout=log,stderr=subprocess.STDOUT)
            wait(lambda:channel.exists() and channel.stat().st_size==wire.SIZE,process)
            handle=channel.open('r+b');mapping=mmap.mmap(handle.fileno(),wire.SIZE)
            def word(offset):return struct.unpack_from('<I',mapping,offset)[0]
            def put(offset,value):struct.pack_into('<I',mapping,offset,value)
            wait(lambda:word(wire.READY_OFFSET)==1,process)
            if not wire.valid_header(mapping[:wire.HEADER_SIZE],wire.SIZE):raise ValueError('Music wire identity')
            if word(wire.TRACK_COUNT_OFFSET)!=3 or struct.unpack_from('<6I',mapping,wire.TOC_OFFSET)!=(0,2,2,77,77,152):raise ValueError('Music TOC differs from independent fixture')
            serial=0;commands=[]
            def request(operation,track=0,from_frame=0,to_frame=0,flags=0,expected=0,cancel=False):
                nonlocal serial
                serial+=1;sequence=word(wire.REQUEST_SEQUENCE_OFFSET);put(wire.REQUEST_SEQUENCE_OFFSET,sequence+1)
                for at,value in [(wire.REQUEST_ID_OFFSET,serial),(wire.OPERATION_OFFSET,operation),(wire.TRACK_OFFSET,track),(wire.FROM_FRAME_OFFSET,from_frame),(wire.TO_FRAME_OFFSET,to_frame),(wire.FLAGS_OFFSET,flags)]:put(at,value)
                if cancel:put(wire.CANCELLED_REQUEST_ID_OFFSET,serial)
                put(wire.REQUEST_SEQUENCE_OFFSET,sequence+2)
                def acknowledged():
                    seq=word(wire.RESPONSE_SEQUENCE_OFFSET)
                    return not(seq&1) and word(wire.RESPONSE_ID_OFFSET)==serial and seq==word(wire.RESPONSE_SEQUENCE_OFFSET)
                wait(acknowledged,process,2)
                status=word(wire.RESPONSE_STATUS_OFFSET)
                if status!=expected:raise ValueError('Unexpected music status: '+str((operation,status,expected)))
                commands.append(dict(id=serial,operation=operation,status=status,cancelled=cancel));return serial
            request(wire.OPERATION_OPEN);request(wire.OPERATION_VOLUME,from_frame=32768)
            if word(wire.VOLUME_OFFSET)!=32768:raise ValueError('Music volume snapshot')
            play=request(wire.OPERATION_PLAY,track=2)
            wait(lambda:word(wire.EVENT_ID_OFFSET)==play and word(wire.EVENT_KIND_OFFSET)==wire.EVENT_COMPLETE,process)
            frames=word(wire.DECODED_FRAMES_LOW_OFFSET)|(word(wire.DECODED_FRAMES_HIGH_OFFSET)<<32)
            if frames!=44100 or word(wire.EVENT_SEQUENCE_OFFSET)&1:raise ValueError('Completed PCM/event publication')
            request(wire.OPERATION_PLAY,track=3,flags=wire.FLAG_LOOP)
            wait(lambda:word(wire.PHASE_OFFSET)==wire.PHASE_PLAYING,process)
            request(wire.OPERATION_PAUSE);wait(lambda:word(wire.PHASE_OFFSET)==wire.PHASE_PAUSED,process)
            request(wire.OPERATION_SEEK,from_frame=30);request(wire.OPERATION_RESUME);request(wire.OPERATION_STOP)
            if word(wire.CURRENT_TRACK_OFFSET)!=0:raise ValueError('Stopped track retained')
            request(wire.OPERATION_PLAY,track=1,expected=wire.STATUS_INVALID)
            request(wire.OPERATION_PLAY,track=2,to_frame=1,expected=wire.STATUS_INVALID)
            request(wire.OPERATION_PLAY,track=2,flags=4,expected=wire.STATUS_INVALID)
            request(wire.OPERATION_PLAY,track=2,expected=wire.STATUS_INVALID,cancel=True)
            if word(wire.CURRENT_TRACK_OFFSET)!=0:raise ValueError('Cancelled request played')
            looping=request(wire.OPERATION_PLAY,track=2,flags=wire.FLAG_LOOP)
            put(wire.CANCELLED_REQUEST_ID_OFFSET,looping)
            wait(lambda:word(wire.EVENT_ID_OFFSET)==looping and word(wire.EVENT_KIND_OFFSET)==wire.EVENT_CANCELLED and word(wire.CURRENT_TRACK_OFFSET)==0,process)
            # Service callbacks during each dispatch must never ACK an unfinished
            # command. Repeated replacement exposes synchronous player startup.
            for repeat in range(16):
                request(wire.OPERATION_VOLUME,from_frame=1000+repeat)
                request(wire.OPERATION_PLAY,track=2+repeat%2)
            request(wire.OPERATION_STOP)
            request(wire.OPERATION_CLOSE)
            process.wait(timeout=15);observed=json.loads(native.read_text())
            if process.returncode or observed.get('ready_after_stop') is not False:raise ValueError('Music worker retirement')
            report.update(commands=commands,native=observed,completed_frames=frames,ready_after_stop=word(wire.READY_OFFSET),
                          sources_stable=all(sha(ROOT/path)==digest for path,digest in sources.items()),inputs=inputs,inputs_stable=all(sha(out/path)==digest for path,digest in inputs.items()))
            if report['ready_after_stop'] or not report['sources_stable'] or not report['inputs_stable']:raise ValueError('Music transport final guards')
            report['success']=True
    except Exception as error:report['error']=str(error);raise
    finally:
        if process and process.poll() is None:process.terminate();process.wait(timeout=5)
        if mapping:mapping.close()
        if handle:handle.close()
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()

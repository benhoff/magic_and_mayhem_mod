#!/usr/bin/env python3
"""Actual PE32 protected storage and ordered active-Lock CPU history controls.

The FIFO responder is an independent synthetic image model, not an original game
or GPU comparison. Existing actual-GL CPU_READ controls are a separate stage.
"""
import argparse
from array import array
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import time

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import frame_v1,render_commands_v2 as ring

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--wine-prefix',type=Path,required=True)
    args=parser.parse_args();declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Prospective source changed')
    prefix=args.wine_prefix.resolve()
    if not prefix.is_dir() or not prefix.is_relative_to(ROOT/'working/tests'):raise ValueError('Owned isolated test prefix required')
    parent=ROOT/'working/tests/native-active-reader';parent.mkdir(exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,sources=declaration['sources'],claims=declaration['claims'],scope=__doc__,cases=[],live_game_executed=False,complete_replacement=False)
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINEPREFIX','WINEDLLOVERRIDES'))}
    env.pop('DISPLAY',None);env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=',MNM_NATIVE_CPU_GUARD='3')
    reservation=None;child=None
    def run(label,cmd,cwd=None,extra=None,expected=0):
        with (out/(label+'.log')).open('x') as log:
            result=subprocess.run([str(p) for p in cmd],cwd=cwd or out,env=env|dict(extra or {}),stdout=log,stderr=log,timeout=120)
        if result.returncode!=expected:raise ValueError(f'{label}: {result.returncode}, expected {expected}')
    try:
        guard=module('active_guard_runner','tools/test-native-cpu-guard.py');reservation=guard.reserve_prefix(prefix)
        native=module('active_native_build','tools/build-native-draw.py');dll=native.build(out/'native',declaration['sources'])
        render=module('active_render_build','tools/build-render-bridge.py');bridge=render.build(True)
        build=json.loads((bridge.parent/'manifest.json').read_text());closure=set(build['compiled_sources'])|set(build['protocol_sources'])
        if closure-set(declaration['sources']):raise ValueError('Undeclared bridge inputs: '+repr(sorted(closure-set(declaration['sources']))))
        imports={k:dict(v) for k,v in native.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8,CreateThread=24,WaitForSingleObject=8,GetTickCount=0,Sleep=4)
        imports['user32']['GetDesktopWindow']=0;imports['gdi32'].update(GetDIBColorTable=16,GetObjectA=12)
        libs=[]
        for name,entries in imports.items():
            definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{k}@{n}\n' for k,n in entries.items()))
            library=out/(name+'.lib');run(name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libs.append(library)
        run('compile',[*native.FLAGS,'-MD','-MF',out/'probe.d','-c',ROOT/'tests/native-active-reader-probe.c','-o',out/'probe.obj'])
        for token in (out/'probe.d').read_text().replace('\\\n',' ').split():
            p=Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working') and str(p.relative_to(ROOT)) not in declaration['sources']:
                raise ValueError('Undeclared probe dependency: '+str(p))
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',*libs])
        run('decoder-build',['cc','-std=c11','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/native-cpu-store-test.c','-o',out/'decoder'])
        run('decoder',[out/'decoder']);report['decoder']=json.loads((out/'decoder.log').read_text())
        report['binaries']={p.name:sha(p) for p in [dll,bridge,out/'probe.exe',out/'decoder']}
        for label,mode,expected in [('concurrent-store-observer','c',0),('held-dc-history','d',0),('protected-world-handoff','w',0),('expired-lease','e',0),('foreign-raw-reader','r',124),('ordered-active-readers','i',0)]:
            dest=out/label;dest.mkdir();shutil.copyfile(dll,dest/'MnmDraw.dll');shutil.copyfile(bridge,dest/'MnmRender.dll')
            extra=dict(MNM_COHERENCE_CASE=mode)
            if mode!='i':run(label,['wine',out/'probe.exe'],dest,extra,expected)
            else:
                channel,frame,capture=dest/'commands.bin',dest/'frame.bin',dest/'capture';capture.mkdir()
                header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
                for p,h,size in [(channel,header,ring.SIZE),(frame,frame_v1.initial_header(),frame_v1.SIZE)]:
                    with p.open('xb') as f:f.write(h);f.truncate(size)
                def wine_path(p):return 'Z:'+str(p).replace('/','\\')
                extra.update(MNM_RENDER_COMMAND_CHANNEL=wine_path(channel),MNM_RENDER_STREAM=wine_path(frame),MNM_RENDER_LOCK_CAPTURE_DIR=wine_path(capture),MNM_RENDER_CONTINUOUS='1',MNM_RENDER_ORDERED_COPIES='1',MNM_PUBLIC_RESIDENT_WORLD='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_AUTO_SHUTDOWN='0')
                images={};identities={};collected=bytearray();parsed=16;consumed=0;sequence=0;requests=0;worlds=0;written_words=0;dirty_since_world=0
                with channel.open('r+b') as f,mmap.mmap(f.fileno(),0) as mapping,(dest/'wine.log').open('x') as log:
                    child=subprocess.Popen(['wine',str(out/'probe.exe')],cwd=dest,env=env|extra,stdout=log,stderr=log)
                    deadline=time.monotonic()+120
                    while True:
                        published=struct.unpack_from('<I',mapping,20)[0]
                        if published<consumed or published-consumed>ring.CAPACITY:raise ValueError('Invalid FIFO publication')
                        count=published-consumed;offset=consumed%ring.CAPACITY;first=min(count,ring.CAPACITY-offset)
                        collected+=mapping[64+offset:64+offset+first]+mapping[64:64+count-first];consumed=published;struct.pack_into('<I',mapping,36,consumed)
                        if len(collected)>=16 and collected[:16]!=b'MNMCMD05'+struct.pack('<II',5,16):raise ValueError('Wrong stream envelope')
                        while len(collected)>=parsed+12:
                            op,seq,size=struct.unpack_from('<III',collected,parsed)
                            if len(collected)<parsed+12+size:break
                            if seq!=sequence+1:raise ValueError('Unordered records')
                            sequence=seq;data=bytes(collected[parsed+12:parsed+12+size]);parsed+=12+size
                            if op==1:
                                sid,w,h,bits,*masks=struct.unpack_from('<7I',data);assert (w,h,bits,masks)==(800,600,16,[0xf800,0x7e0,31]);image=array('H');image.frombytes(data[28:]);images[sid]=image
                            elif op==19:
                                sid,g,gh,n,nh=struct.unpack('<5I',data);assert (gh,n,nh)==(0,17,34)
                                if worlds:assert dirty_since_world==(3 if worlds==17 else 2 if worlds<=18 else 1),(worlds,dirty_since_world)
                                images[sid]=array('H',((i*71+g*19)&65535 for i in range(800*600)));identities[sid]=(g,gh,n,nh);worlds+=1;dirty_since_world=0
                            elif op==20:
                                sid,x,y,w,h=struct.unpack_from('<5I',data);pixels=w*h;assert len(data)==20+pixels*3
                                for row in range(h):
                                    for col in range(w):
                                        at=row*w+col;mask=data[20+pixels*2+at];assert mask in (0,1)
                                        if mask:images[sid][(y+row)*800+x+col]=struct.unpack_from('<H',data,20+at*2)[0];written_words+=1;dirty_since_world+=1
                            elif op==21:
                                sid,ticket,high,session,w,h,g,gh,n,nh=struct.unpack('<10I',data);assert ticket==requests+1 and (high,session,w,h)==(0,123,800,600) and identities[sid]==(g,gh,n,nh)
                                # Writes are staged privately, then merged into this native-history reply.
                                assert dirty_since_world==(1 if g==17 else 0),(g,dirty_since_world)
                                reply=struct.pack('<16I',0x434d4e4d,0x44414552,1,64,session,ticket,high,sid,w,h,w*h*2,g,gh,n,nh,1)+images[sid].tobytes()
                                temporary=channel.with_suffix('.reply.tmp');temporary.write_bytes(reply);os.replace(temporary,Path(str(channel)+'.cpu-reply'));requests+=1
                            elif op==7:images.pop(struct.unpack('<I',data)[0])
                            elif op==2:
                                sid,x,y,w,h=struct.unpack_from('<5I',data)
                                assert (x,y,w,h)==(0,0,800,600) and len(data)==20+w*h*2
                                result=array('H');result.frombytes(data[20:])
                                # The native self-copy changes no pixels. This
                                # checks all producer bytes against the model.
                                assert result==images[sid]
                                images[sid]=result
                            elif op not in (6,8):raise ValueError('Unexpected synthetic operation '+str(op))
                        if child.poll() is not None and struct.unpack_from('<I',mapping,20)[0]==consumed:break
                        if time.monotonic()>deadline:raise TimeoutError('Active reader fixture deadline')
                        time.sleep(.001)
                    if child.returncode:raise ValueError('PE32 reader failed: '+str(child.returncode))
                    assert parsed==len(collected) and not images and worlds==19 and requests==19 and written_words==38,(parsed,len(collected),worlds,requests,written_words)
                    assert dirty_since_world==1 and struct.unpack_from('<II',mapping,24)==(2,0)
                assert (capture/'session-00000001.bin').read_bytes()==collected
                counts=struct.unpack('<12I',(dest/'native-cpu-read-counts.bin').read_bytes())
                assert counts==(0x524d4e4d,0x31304344,1,48,19,0,18,1,0,0,0,0),counts
                report['ordered_readers']=dict(worlds=worlds,cpu_reads=requests,exact_cpu_word_writes=written_words,byte_merge=True,partial_lock=True,equal_baseline_stores=True,complete_shutdown=True,fifo_archive_equal=True)
                report['cpu_reader_counts']=list(counts)
                child=None
            row=dict(id=label,success=True,exit_code=expected)
            if expected==124:
                fault=struct.unpack('<24I',(dest/'native-cpu-guard-fault.bin').read_bytes());assert fault[6]==0 and fault[20]==1;row['fault']=list(fault)
            else:
                result=struct.unpack('<4I',(dest/'active-reader-result.bin').read_bytes());assert result[:2]==(1,ord(mode));row['result']=list(result)
            report['cases'].append(row)
        report['success']=True
    except (OSError,ValueError,AssertionError,RuntimeError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if child and child.poll() is None:child.terminate();child.wait(timeout=10)
        if reservation:
            subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15)
            subprocess.run(['wineserver','-w'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15);reservation.close()
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in declaration['sources'].items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json',flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())

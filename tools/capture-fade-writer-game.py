#!/usr/bin/env python3
"""Run native fade takeover on an isolated automatic menu/battle-startup journey."""
import argparse
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import re
import shutil
import signal
import struct
import subprocess
import tempfile
import time
from fade_live_capture import load as load_fades
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def stage_preferences(game):
    encoder=module('writer_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(ROOT)
    edits=[];expected={'SOUND':{'CDMUSICENABLED'},'VIDEO':{'PLAYFMV','PLAYFMVOUT'}}
    for name,packed in [('CFG/prefs.cfg',False),('CFG/Encrypted/prefs.cfg',True)]:
        path=game/name
        if not path.exists():continue
        if not path.resolve().is_relative_to(game.resolve()):raise ValueError('Preference path escapes isolated game')
        before=path.read_bytes();text=decoder.decode(before)[1] if packed else before;section='';found=set();lines=[]
        for line in text.splitlines(keepends=True):
            heading=re.match(rb'\s*\[([^]]+)\]',line)
            if heading:section=heading[1].decode('ascii').upper()
            setting=re.match(rb'(\s*([A-Za-z]+)\s*=\s*)(TRUE|FALSE)([ \t]*(?:;[^\r\n]*)?)(\r?\n)?$',line,re.I)
            if setting and setting[2].decode('ascii').upper() in expected.get(section,set()):
                key=(section,setting[2].decode('ascii').upper())
                if key in found:raise ValueError('Duplicate staged preference')
                found.add(key);line=setting[1]+b'FALSE'+setting[4]+(setting[5] or b'')
            lines.append(line)
        if len(found)!=3:raise ValueError('Missing staged preference')
        changed=b''.join(lines);after=encoder.encode(changed,decoder)[0] if packed else changed
        if packed and decoder.decode(after)[1]!=changed:raise ValueError('Preference round trip')
        edits.append((path,before,after))
    if not edits:raise ValueError('No staged preferences')
    result=[]
    for path,before,after in edits:
        path.write_bytes(after);result.append(dict(path=str(path.relative_to(game)),before_sha256=hashlib.sha256(before).hexdigest(),after_sha256=sha(path)))
    return result
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64')
    args=parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('Use private xvfb-run')
    claims=json.loads(args.claims.read_text());sources={p:sha(ROOT/p) for p in claims['sources']}
    if sources!=claims['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/experiments/canvas-writers';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    report=dict(schema=1,success=False,claims=claims['claims'],sources=sources,experiment=str(root.relative_to(ROOT)),original_sha256=HASH,live_equivalence=False,original_work_bypassed=False,live_replacement=False,
        scope='Installed opt-in native positive bounded fade loop under original Lock/Unlock in automatic Main/Single Player/Quick Battle/Map/startup. Full physical output compared with independent unchanged PE32 replay for each admitted captured call. No glyph/image/panel/world replacement, manual play or whole-canvas/native renderer claim.')
    commands=[];process=None;channel=None;env=None;trace=[]
    def run(label,argv,timeout=180,environment=None):
        log=root/(label+'.log')
        with log.open('x') as f:r=subprocess.run([str(a) for a in argv],stdout=f,stderr=subprocess.STDOUT,timeout=timeout,env=environment)
        commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=r.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)));r.check_returncode()
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        source=ROOT/'working/game-nocd';data=(source/'Chaos.exe').read_bytes()
        if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unsupported source executable')
        run('build-writers',['python3',ROOT/'tools/build-canvas-writers.py']);run('build-menu',['python3',ROOT/'tools/build-menu-observer.py'])
        run('copy-game',['cp','-a','--reflink=auto',source.resolve(),root/'game'])
        if (root/'game').is_symlink() or sha(root/'game/Chaos.exe')!=HASH:raise ValueError('Disposable copy changed/aliased')
        patch=module('writer_import','tools/prepare-shadow-experiment.py').add_import
        staged=patch(data,'MnmMenu.dll','MenuAnchor',b'.mnmenu');staged=patch(staged,'MnmCanvasWriters.dll','CanvasWritersAnchor',b'.mnwrite')
        pe=struct.unpack_from('<I',data,60)[0];table=pe+24+struct.unpack_from('<H',data,pe+20)[0]
        for i in range(struct.unpack_from('<H',data,pe+6)[0]):
            size,at=struct.unpack_from('<II',data,table+i*40+16)
            if staged[at:at+size]!=data[at:at+size]:raise ValueError('Original section changed')
        (root/'game/Chaos.exe').write_bytes(staged)
        for part,name in [('menu-observer','MnmMenu.dll'),('canvas-writers','MnmCanvasWriters.dll')]:
            shutil.copy2(ROOT/'working/build'/part/name,root/'game'/name);shutil.copy2(ROOT/'working/build'/part/'manifest.json',root/(part+'-build.json'))
        preferences=stage_preferences(root/'game')
        metadata=dict(origin='fade_writer_takeover',source_sha256=HASH,staged_sha256=sha(root/'game/Chaos.exe'),menu_dll_sha256=sha(root/'game/MnmMenu.dll'),writer_dll_sha256=sha(root/'game/MnmCanvasWriters.dll'),preferences=preferences)
        (root/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
        closure={'tools/build-shadow-bridge.py','tools/build-menu-observer.py','tools/build-canvas-writers.py','tools/capture-fade-writer-game.py','tools/canvas-writer-game-runner.py','tools/fade_live_capture.py','tools/prepare-shadow-experiment.py','tools/encode-cfg.py','tools/decode-cfg.py','tools/run-game.sh','tools/original-manifest.sh','tests/canvas-fade-reference.cpp','tests/sprite-binary-reference.cpp','tests/test-fade-live-capture.py'}
        for part in ['menu-observer','canvas-writers']:
            build=json.loads((root/(part+'-build.json')).read_text())
            for p in build['sources']:
                closure.add(('protocols/include/mnm/'+p if p.startswith('menu_v') else 'runtime/menu/'+p) if part=='menu-observer' else p)
        # Existing menu build manifests use paths relative to the repository.
        if closure-sources.keys():raise ValueError('Unbound build/run dependencies: '+repr(sorted(closure-sources.keys())))
        report['executed_project_sources']=sorted(closure)
        run('decoder-test',['python3',ROOT/'tests/test-fade-live-capture.py'])
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None)
        env.update(MNM_CANVAS_WRITER_EXPERIMENT=str(root),WINEDEBUG='-all',WINEPREFIX=str(root/'wineprefix'))
        run('copy-prefix',['cp','-a','--reflink=auto',args.prefix_template.resolve(),root/'wineprefix'])
        run('wineboot',['wineboot','-u'],90,env)
        raw=bytearray(32768);raw[:16]=b'MNMMCMD2'+struct.pack('<II',2,32768);struct.pack_into('<III',raw,16,2,1,1)
        (root/'channel.bin').write_bytes(raw)
        with (root/'channel.bin').open('r+b') as f:channel=mmap.mmap(f.fileno(),0)
        heartbeat=1;sequence=2;request=0
        def state():
            first=struct.unpack_from('<I',channel,128)[0];row=struct.unpack_from('<7I',channel,132);payload=bytes(channel[160:16924])
            return (row,payload) if not first&1 and first==struct.unpack_from('<I',channel,128)[0] else None
        def tick():
            nonlocal heartbeat,sequence
            heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1);struct.pack_into('<I',channel,24,heartbeat);struct.pack_into('<I',channel,16,sequence)
        def wait(screen=None,ack=None):
            deadline=time.monotonic()+60
            while time.monotonic()<deadline:
                row=state()
                if row and (screen is None or row[0][1]==screen and row[0][2]) and (ack is None or row[0][3]==ack):return row
                if process.poll() is not None:raise ValueError('Original exited before menu reply')
                tick();time.sleep(.02)
            raise ValueError('Menu startup/reply timed out')
        def send(screen,action,argument=0,rules=None):
            nonlocal heartbeat,sequence,request
            row=wait(screen);heartbeat+=1;sequence+=2;request+=1
            struct.pack_into('<I',channel,16,sequence-1);struct.pack_into('<6I',channel,20,1,heartbeat,request,action,row[0][0],argument)
            if rules is not None:struct.pack_into('<17I',channel,44,*rules)
            struct.pack_into('<I',channel,16,sequence)
            reply=wait(ack=request)
            if reply[0][4]!=1:raise ValueError('Original menu action refused')
            trace.append(dict(screen=screen,action=action,argument=argument,ack=request,status=reply[0][4]))
        log=(root/'game.log').open('x')
        process=subprocess.Popen([str(ROOT/'tools/run-game.sh'),'launch','--no-gamescope','--prefix',env['WINEPREFIX'],'--runner',str(ROOT/'tools/canvas-writer-game-runner.py')],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        send(3,1);send(22,4);row=wait(14);rules=list(struct.unpack_from('<13I',row[1],8))+[struct.unpack_from('<I',row[1],60+i*48+12)[0] for i in range(4)];rules[2]=0
        send(14,6,rules=rules);send(25,9,2);send(14,7,rules=rules)
        deadline=time.monotonic()+3
        while time.monotonic()<deadline:
            if process.poll() is not None:raise ValueError('Original exited after battle start')
            tick();time.sleep(.05)
        os.killpg(process.pid,signal.SIGTERM);process.wait(timeout=10);process=None
        run('wine-stop',['wineserver','-k'],10,env)
        rows=load_fades(root/'fades.bin')
        if len(rows)<5:raise ValueError('Insufficient admitted live fade calls')
        corpus=bytearray(b'MNMFADE1'+struct.pack('<I',len(rows)))
        for row in rows:corpus+=struct.pack('<4I',row['width'],row['height'],row['stride'],row['format'])+row['before']
        (root/'live-cases.bin').write_bytes(corpus)
        reference=root/'original-fade-reference'
        run('reference-build',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',ROOT/'tests/canvas-fade-reference.cpp','-o',reference])
        run('independent-original-replay',[reference,source/'Chaos.exe',root/'live-cases.bin',root/'original.bin'])
        original=(root/'original.bin').read_bytes();at=0;failures=[];summaries=[]
        for row in rows:
            byte_count=row['words']*2;result=original[at:at+byte_count];at+=byte_count+row['width']*row['height']*2
            if result!=row['after']:failures.append(row['index'])
            summaries.append({k:v for k,v in row.items() if k not in ('before','after')})
        if at!=len(original) or failures:raise ValueError('Independent live fade pixel mismatch: '+repr(failures))
        report.update(success=True,live_equivalence=True,original_work_bypassed=True,live_replacement=True,menu_trace=trace,case_count=len(rows),compared_words=sum(r['words'] for r in rows),native_calls=rows[-1]['native'],forwarded_calls=rows[-1]['forwarded'],cases=summaries,failures=failures)
        for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','menu_dll_sha256'),('MnmCanvasWriters.dll','writer_dll_sha256')]:
            if sha(root/'game'/name)!=metadata[key]:raise ValueError('Staged file changed')
    except Exception as error:report.update(success=False,error=repr(error),menu_trace=trace)
    finally:
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGTERM)
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        if env:subprocess.run(['wineserver','-k'],env=env,capture_output=True,timeout=10,check=False)
        if channel:channel.close()
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        report.update(commands=commands,sources_stable=all(sha(ROOT/p)==h for p,h in sources.items()),source_input_stable=sha(ROOT/'working/game-nocd/Chaos.exe')==HASH)
        report['success']=report['success'] and report['sources_stable'] and report['source_input_stable']
        report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(root.iterdir()) if p.is_file()}
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable']}),flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())

#!/usr/bin/env python3
"""Capture unchanged no-CD fill wrapper outputs through real Wine Surface2."""
import argparse
import importlib.util
import gzip
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES=['tests/surface-fill-reference.c','tests/surface-key-reference.c','runtime/shadow/win32_min.h','tools/capture-original-surface-keys.py','tools/capture-original-surface-fills.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
spec=importlib.util.spec_from_file_location('key_capture',ROOT/'tools/capture-original-surface-keys.py')
key_capture=importlib.util.module_from_spec(spec);spec.loader.exec_module(key_capture)
reserve_wine=key_capture.reserve_wine
def cases():
    shapes=[('full-null',[0,0,8,6],1),('full-explicit',[0,0,8,6],0),('partial',[1,1,6,4],0),('one-pixel',[7,5,8,6],0),('bottom-row',[0,5,8,6],0),('left',[-2,1,3,4],0),('right',[6,1,10,4],0),('outside',[9,0,12,3],0),('empty',[1,1,1,4],0),('reversed',[5,4,1,1],0)]
    return [dict(id=i,entry=0x58bac0,fast=0,no_wait=0,pattern=0,key=color,mode=clip,phase=null,source=[0,0,0,0],destination=rect,label=label) for i,(color,clip,label,rect,null) in enumerate((color,clip,label,rect,null) for color in [0,0x8000,0xffff,0x12345678,0xffffffff] for clip in range(5) for label,rect,null in shapes)]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,default=ROOT/'working/game-nocd/Chaos.exe');p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');p.add_argument('--report',type=Path,required=True);p.add_argument('--fixture-dir',type=Path,default=ROOT/'tests/fixtures/surfaces');a=p.parse_args()
    stem='original-rgb565-fills';corpus=a.fixture_dir/(stem+'.json.gz');catalog=a.fixture_dir/(stem+'-corpus.json')
    if any(x.exists() for x in (a.report,corpus,catalog)):p.error('Refusing to overwrite retained evidence')
    if not os.environ.get('DISPLAY'):p.error('Run under Xvfb or a display')
    wine_reservation=reserve_wine()
    parent=ROOT/'working/tests/original-surface-fills';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    sources={n:sha(ROOT/n) for n in SOURCES};env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
    def command(name,argv,timeout=60):
        with (run/(name+'.log')).open('w') as log:subprocess.run(argv,cwd=run,env=env,stdout=log,stderr=log,check=True,timeout=timeout)
    command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
    try:
        if sha(a.executable)!=BUILD_HASH:raise ValueError('Unsupported original executable hash')
        image=a.executable.read_bytes();(run/'original.exe').write_bytes(image)
        command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
        imports={'VirtualAlloc':16,'VirtualProtect':16,'GetFileSize':8,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')])
        command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/surface-fill-reference.c'),'-o',str(run/'probe.obj')])
        command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/include:_original_reservation','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
        probe=(run/'probe.exe').read_bytes();pe=struct.unpack_from('<I',probe,60)[0];opt=pe+24
        assert struct.unpack_from('<I',probe,opt+28)[0]==0x400000 and struct.unpack_from('<I',probe,opt+16)[0]>=32*1024*1024,'Harness code overlaps original mapping reservation'
        inputs=cases();(run/'inputs.bin').write_bytes(b''.join(struct.pack('<8I8i',*(c[k] for k in ['id','entry','fast','no_wait','pattern','key','mode','phase']),*c['source'],*c['destination']) for c in inputs))
        probe_hash=sha(run/'probe.exe');command('driver',['wine',str(run/'probe.exe')],60)
        raw=(run/'outputs.bin').read_bytes();expected=inputs;assert len(raw)==904*len(expected)
        rows=[]
        for i,c in enumerate(expected):
            data=raw[i*904:(i+1)*904];assert list(struct.unpack_from('<8I8i',data))==[*(c[k] for k in ['id','entry','fast','no_wait','pattern','key','mode','phase']),*c['source'],*c['destination']]
            trace=list(struct.unpack_from('<16I',data,64));pixels=list(struct.unpack_from('<192H',data,344));assert trace[0]==1 and trace[1]==(2 if c['fast'] else 1) and trace[14:]==[1,0]
            rows.append(dict(input=c,trace=trace,hresult=trace[3],destination_desc=list(struct.unpack_from('<27I',data,236)),destination_before=pixels[48:96],destination_after=pixels[144:],dialogs=struct.unpack_from('<I',data,752)[0],destroy_calls=struct.unpack_from('<I',data,756)[0],clip_before=list(struct.unpack_from('<18I',data,760)),clip_after=list(struct.unpack_from('<18I',data,832))))
        assert all(sha(ROOT/n)==h for n,h in sources.items()) and sha(a.executable)==BUILD_HASH and sha(run/'original.exe')==BUILD_HASH and sha(run/'probe.exe')==probe_hash
        a.fixture_dir.mkdir(parents=True,exist_ok=True);payload=json.dumps(rows,separators=(',',':')).encode();corpus.write_bytes(gzip.compress(payload,mtime=0))
        scope='Unchanged original fill wrapper0x58bac0 executed in a private PE mapping against Wine built-in Surface2 RGB565 system-memory objects. Independent original/API outputs; original error-reporting UI imports observed without modal windows. No lost-surface retry/Restore, live gameplay or Windows-driver equivalence.'
        metadata=dict(schema=1,path=corpus.name,sha256=sha(corpus),raw_sha256=hashlib.sha256(payload).hexdigest(),cases=len(rows),original_sha256=BUILD_HASH,entry=0x58bac0,oracle='unchanged_original_fill_wrapper_real_surface2_post_call_lock',scope=scope);catalog.write_text(json.dumps(metadata,indent=2)+'\n')
        sources.update({str(x.relative_to(ROOT)):sha(x) for x in [corpus,catalog]})
        report=dict(schema=1,success=True,sources=sources,original_sha256=BUILD_HASH,original_wrapper_executed=True,original_instruction_bytes_patched=False,ui_imports_observed=[dict(iat=0x5c520c,name="MessageBoxA"),dict(iat=0x5c5208,name="DestroyWindow")],driver_calls=True,native_renderer_executed=False,scope=scope,environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),ddraw_sha256=sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b',surface_interface='IDirectDrawSurface2',caps=0x840,cooperative_level='NORMAL'),cases=len(rows),hresults={f'0x{h:08x}':sum(r['hresult']==h for r in rows) for h in sorted({r['hresult'] for r in rows})},artifacts=str(run.relative_to(ROOT)),probe_sha256=probe_hash,inputs_sha256=sha(run/'inputs.bin'),outputs_sha256=sha(run/'outputs.bin'),fixture=metadata)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
    report['original_manifest_verified_before_after']=True;a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    wine_reservation.close()
    print(json.dumps(dict(cases=report['cases'],hresults=report['hresults'])))
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Capture unchanged no-CD keyed overlap wrapper outputs through real Wine Surface2."""
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
SOURCES=['tests/surface-keyed-overlap-reference.c','tests/surface-key-reference.c','runtime/shadow/win32_min.h','tools/capture-original-surface-keys.py','tools/capture-original-surface-keyed-overlap.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
spec=importlib.util.spec_from_file_location('key_capture',ROOT/'tools/capture-original-surface-keys.py')
key_capture=importlib.util.module_from_spec(spec);spec.loader.exec_module(key_capture)
reserve_wine=key_capture.reserve_wine
def cases():
    shapes=[('right',[0,1,6,5],[1,1,7,5]),('left',[1,1,7,5],[0,1,6,5]),('down',[1,0,7,5],[1,1,7,6]),('up',[1,1,7,6],[1,0,7,5]),('down-right',[0,0,7,5],[1,1,8,6]),('identical',[1,1,7,5],[1,1,7,5]),('cross-clip-down',[0,0,8,4],[0,2,8,6]),('source-outside-late',[0,0,8,7],[0,-1,8,6])]
    return [dict(id=i,entry=0x58ca90,fast=fast,no_wait=no_wait,pattern=pattern,key=key,mode=alias,phase=clip,source=src,destination=dst,label=label)
      for i,(fast,no_wait,alias,pattern,key,clip,label,src,dst) in enumerate((fast,no_wait,alias,pattern,key,clip,label,src,dst) for fast in (0,1) for no_wait in (0,1) for alias in (0,1) for pattern in (0,1) for key in (0,0x8000,0xffff) for clip in range(6) for label,src,dst in shapes)]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,default=ROOT/'working/game-nocd/Chaos.exe');p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');p.add_argument('--report',type=Path,required=True);p.add_argument('--fixture-dir',type=Path,default=ROOT/'tests/fixtures/surfaces');a=p.parse_args()
    stem='original-rgb565-keyed-overlap';corpus=a.fixture_dir/(stem+'.json.gz');catalog=a.fixture_dir/(stem+'-corpus.json')
    if any(x.exists() for x in (a.report,corpus,catalog)):p.error('Refusing to overwrite retained evidence')
    if not os.environ.get('DISPLAY'):p.error('Run under Xvfb or a display')
    wine_reservation=reserve_wine()
    parent=ROOT/'working/tests/original-surface-keyed-overlap';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    sources={n:sha(ROOT/n) for n in SOURCES};env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
    def command(name,argv,timeout=60):
        with (run/(name+'.log')).open('w') as log:subprocess.run(argv,cwd=run,env=env,stdout=log,stderr=log,check=True,timeout=timeout)
    try:
        command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
        if sha(a.executable)!=BUILD_HASH:raise ValueError('Unsupported original executable hash')
        image=a.executable.read_bytes();(run/'original.exe').write_bytes(image)
        command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
        imports={'VirtualAlloc':16,'VirtualProtect':16,'GetFileSize':8,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')])
        command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/surface-keyed-overlap-reference.c'),'-o',str(run/'probe.obj')])
        command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/include:_original_reservation','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
        probe=(run/'probe.exe').read_bytes();pe=struct.unpack_from('<I',probe,60)[0];opt=pe+24
        assert struct.unpack_from('<I',probe,opt+28)[0]==0x400000 and struct.unpack_from('<I',probe,opt+16)[0]>=32*1024*1024,'Harness code overlaps original mapping reservation'
        inputs=cases();(run/'inputs.bin').write_bytes(b''.join(struct.pack('<8I8i',*(c[k] for k in ['id','entry','fast','no_wait','pattern','key','mode','phase']),*c['source'],*c['destination']) for c in inputs))
        probe_hash=sha(run/'probe.exe');command('driver',['wine',str(run/'probe.exe')],60)
        raw=(run/'outputs.bin').read_bytes();expected=inputs;assert len(raw)==912*len(expected)
        rows=[]
        for i,c in enumerate(expected):
            data=raw[i*912:(i+1)*912];assert list(struct.unpack_from('<8I8i',data))==[*(c[k] for k in ['id','entry','fast','no_wait','pattern','key','mode','phase']),*c['source'],*c['destination']]
            trace=list(struct.unpack_from('<16I',data,64));pixels=list(struct.unpack_from('<192H',data,344));assert trace[0]==1 and trace[1]==(2 if c['fast'] else 1) and trace[14:]==[0,1]
            rows.append(dict(input=c,trace=trace,hresult=trace[3],source_desc=list(struct.unpack_from('<27I',data,128)),destination_desc=list(struct.unpack_from('<27I',data,236)),source_before=pixels[:48],destination_before=pixels[48:96],source_after=pixels[96:144],destination_after=pixels[144:],key_before=list(struct.unpack_from('<3I',data,728)),key_after=list(struct.unpack_from('<3I',data,740)),identity=list(struct.unpack_from('<4I',data,752)),clip_before=list(struct.unpack_from('<18I',data,768)),clip_after=list(struct.unpack_from('<18I',data,840))))
        assert all(sha(ROOT/n)==h for n,h in sources.items()) and sha(a.executable)==BUILD_HASH and sha(run/'original.exe')==BUILD_HASH and sha(run/'probe.exe')==probe_hash
        a.fixture_dir.mkdir(parents=True,exist_ok=True);payload=json.dumps(rows,separators=(',',':')).encode();corpus.write_bytes(gzip.compress(payload,mtime=0))
        scope='Unchanged original keyed overlap wrapper0x58ca90 executed in a private PE mapping against Wine built-in Surface2 RGB565 system-memory objects. Independent original/API outputs; source/destination COM identity and complete pixel/clip/key states recorded. No original retry/Restore, masked/other-format overlap, live gameplay or Windows-driver equivalence.'
        metadata=dict(schema=1,path=corpus.name,sha256=sha(corpus),raw_sha256=hashlib.sha256(payload).hexdigest(),cases=len(rows),original_sha256=BUILD_HASH,entry=0x58ca90,oracle='unchanged_original_keyed_overlap_wrapper_real_surface2_post_call_lock',scope=scope);catalog.write_text(json.dumps(metadata,indent=2)+'\n')
        sources.update({str(x.relative_to(ROOT)):sha(x) for x in [corpus,catalog]})
        report=dict(schema=1,success=True,sources=sources,original_sha256=BUILD_HASH,original_wrapper_executed=True,original_instruction_bytes_patched=False,driver_calls=True,native_renderer_executed=False,scope=scope,environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),ddraw_sha256=sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b',surface_interface='IDirectDrawSurface2',caps=0x840,cooperative_level='NORMAL'),cases=len(rows),hresults={f'0x{h:08x}':sum(r['hresult']==h for r in rows) for h in sorted({r['hresult'] for r in rows})},artifacts=str(run.relative_to(ROOT)),probe_sha256=probe_hash,inputs_sha256=sha(run/'inputs.bin'),outputs_sha256=sha(run/'outputs.bin'),fixture=metadata)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10)
        try:command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
        finally:wine_reservation.close()
    report['original_manifest_verified_before_after']=True;a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    wine_reservation.close()
    print(json.dumps(dict(cases=report['cases'],hresults=report['hresults'])))
if __name__=='__main__':main()

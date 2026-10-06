#!/usr/bin/env python3
"""Capture real Wine DirectDraw Surface2 clipping/error outputs independently of native code."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def cases():
    shapes = [
        ('inside', [1,1,5,4], [2,1,6,4]),
        ('left', [0,0,4,3], [-2,1,2,4]),
        ('top', [0,0,4,3], [1,-2,5,1]),
        ('right', [0,0,4,3], [6,1,10,4]),
        ('bottom', [0,0,4,3], [1,5,5,8]),
        ('corner', [0,0,4,3], [-2,-2,2,1]),
        ('outside-left', [0,0,4,3], [-8,1,-4,4]),
        ('outside-right', [0,0,4,3], [8,1,12,4]),
        ('outside-top', [0,0,4,3], [1,-4,5,-1]),
        ('outside-bottom', [0,0,4,3], [1,6,5,9]),
        ('negative-source', [-2,0,2,3], [1,1,5,4]),
        ('excess-source', [6,4,10,7], [1,1,5,4]),
        ('empty-source-width', [1,1,1,3], [1,1,1,3]),
        ('empty-source-height', [1,1,3,1], [1,1,3,1]),
        ('reversed-source', [4,3,1,1], [1,1,-2,-1]),
        ('empty-destination', [0,0,4,3], [1,1,1,4]),
        ('reversed-destination', [0,0,4,3], [4,4,1,1]),
        ('full', [0,0,8,6], [0,0,8,6]),
        ('clipped-negative-source', [-2,0,6,6], [0,0,8,6]),
        ('clipped-excess-source', [2,0,10,6], [0,0,8,6]),
        ('outside-invalid-source', [-2,0,2,3], [9,0,13,3]),
    ]
    result=[]
    for fast in (0,1):
        for label,src,dst in shapes:
            # BltFast has x/y only; destination RECT size has no meaning there.
            if fast and label in ('empty-destination','reversed-destination'):
                continue
            for clip in range(5):
                for wait in (0,1):
                    result.append(dict(id=len(result),label=label,fast=fast,clip=clip,
                        flags=(0x10 if fast else 0x1000000) if wait else 0,
                        held=0,source=src,destination=dst))
        for held in (1,2):
            result.append(dict(id=len(result),label='locked-source' if held==1 else 'locked-destination',
                fast=fast,clip=0,flags=0,held=held,source=[0,0,4,3],destination=[1,1,5,4]))
        for label,flag in [('missing-source-key',1 if fast else 0x8000),('invalid-flags',0x80000000)]:
            result.append(dict(id=len(result),label=label,fast=fast,clip=0,flags=flag,
                held=0,source=[0,0,4,3],destination=[1,1,5,4]))
    return result

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64')
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--fixture-dir',type=Path,required=True)
    parser.add_argument('--stem',default='driver-clipping-surface2')
    args=parser.parse_args()
    if not args.stem or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-' for c in args.stem):
        parser.error('Stem must use lowercase letters, digits and hyphens')
    if args.report.exists() or any((args.fixture_dir/p).exists() for p in [args.stem+'-original.json.gz',args.stem+'-corpus.json']):
        parser.error('Refusing to overwrite evidence or fixtures')
    if not os.environ.get('DISPLAY'):
        parser.error('Run under Xvfb or a display')
    parent=ROOT/'working/tests/surface-driver';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    sources=['tests/surface-driver-reference.c','runtime/shadow/win32_min.h','tools/capture-surface-driver.py','tests/test-surface-driver.py']
    fingerprints={p:sha(ROOT/p) for p in sources}
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
    def command(name,argv,timeout=60):
        with (run/(name+'.log')).open('w') as log:
            subprocess.run(argv,cwd=run,env=env,stdout=log,stderr=log,check=True,timeout=timeout)
    command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
    try:
        command('prefix',['cp','-a','--reflink=auto',str(args.prefix_template.resolve()),env['WINEPREFIX']])
        command('wineboot',['wineboot','-u'],120)
        imports={'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')])
        command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/surface-driver-reference.c'),'-o',str(run/'probe.obj')])
        command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
        inputs=cases();(run/'inputs.bin').write_bytes(b''.join(struct.pack('<5I8i',c['id'],c['fast'],c['clip'],c['flags'],c['held'],*c['source'],*c['destination']) for c in inputs))
        probe_hash=sha(run/'probe.exe');command('driver',['wine',str(run/'probe.exe')],60)
        assert sha(run/'probe.exe')==probe_hash
        raw=(run/'outputs.bin').read_bytes();assert len(raw)==780*len(inputs),'Capture length'
        captures=[]
        for index,c in enumerate(inputs):
            data=raw[index*780:(index+1)*780];before=struct.unpack_from('<5I8i',data);after=struct.unpack_from('<5I8i',data,52)
            assert before==(c['id'],c['fast'],c['clip'],c['flags'],c['held'],*c['source'],*c['destination'])
            pixels=struct.unpack_from('<192H',data,396)
            record=dict(input=c,hresult=struct.unpack_from('<I',data,104)[0],input_after=list(after),
                source_desc=list(struct.unpack_from('<27I',data,108)),destination_desc=list(struct.unpack_from('<27I',data,216)),
                clip_size=struct.unpack_from('<I',data,324)[0],clip_result=struct.unpack_from('<I',data,328)[0],clip_data=list(struct.unpack_from('<16I',data,332)),
                source_before=list(pixels[:48]),destination_before=list(pixels[48:96]),
                source_after=list(pixels[96:144]),destination_after=list(pixels[144:]))
            captures.append(record)
        assert all(sha(ROOT/p)==h for p,h in fingerprints.items()),'Sources changed during capture'
        args.fixture_dir.mkdir(parents=True,exist_ok=True)
        corpus=args.fixture_dir/(args.stem+'-original.json.gz');catalog=args.fixture_dir/(args.stem+'-corpus.json')
        payload=json.dumps(captures,separators=(',',':')).encode();corpus.write_bytes(gzip.compress(payload,mtime=0))
        metadata=dict(schema=1,path=corpus.name,sha256=sha(corpus),raw_sha256=hashlib.sha256(payload).hexdigest(),cases=len(captures),
            scope='Real Wine built-in DirectDraw Surface2 system-memory RGB565 API outputs; external driver policy, not Windows 1998 equivalence, wrapper failure handling or native renderer validation.')
        catalog.write_text(json.dumps(metadata,indent=2)+'\n')
        fingerprints.update({str(p.resolve().relative_to(ROOT)):sha(p) for p in [corpus,catalog]})
        version=subprocess.check_output(['wine','--version'],text=True).strip()
        driver=run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'
        report=dict(schema=1,success=True,cases=len(captures),sources=fingerprints,scope=metadata['scope'],
            environment=dict(wine=version,ddraw_sha256=sha(driver),override='ddraw=b',surface_caps=0x840,width=8,height=6,bits=16,masks=[0xf800,0x7e0,31],surface_interface='IDirectDrawSurface2',cooperative_level='NORMAL',software_gl=True),
            driver_calls=True,original_game_executed=False,original_game_text_patched=False,native_renderer_executed=False,
            artifacts=str(run.relative_to(ROOT)),probe_sha256=probe_hash,inputs_sha256=sha(run/'inputs.bin'),outputs_sha256=sha(run/'outputs.bin'),
            summary=dict(hresults={f'0x{h:08x}':sum(c['hresult']==h for c in captures) for h in sorted({c['hresult'] for c in captures})}),fixture=metadata)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
    report['original_manifest_verified_before_after']=True
    args.report.parent.mkdir(parents=True,exist_ok=True)
    with args.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps(report['summary']))
if __name__=='__main__':main()

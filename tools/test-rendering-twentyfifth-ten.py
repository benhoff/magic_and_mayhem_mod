#!/usr/bin/env python3
"""Ten native BMP domains: prospective claims, bounded controls, SQLite evidence."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import read_bytes,write_report
ROOT=Path(__file__).resolve().parents[1]
NAMES=['indexed1','indexed4','indexed8','rgb555','bgrx32','masks16','masks32','rle4','rle8','core']
IDS=['NP.bmp.'+name for name in NAMES]
PALETTE=[(19,37,53),(255,0,0),(0,255,0),(0,0,255)]


def fixtures(root):
    root.mkdir();records=[];families={k:0 for k in NAMES}
    def emit(name,width,height,data,rgb,family):
        (root/(name+'.bmp')).write_bytes(data);(root/(name+'.rgb')).write_bytes(bytes(rgb))
        records.append(f'{name} {width} {height}\n')
        if family in families:families[family]+=1
    def uncompressed(name,bits,width,top,core=False,masks=None,default_palette=False):
        height=3;indexed=bits<=8;palette=(PALETTE[:2] if bits==1 else PALETTE)
        count=(1<<bits) if core or default_palette else len(palette)
        table=b''.join(bytes((c[2],c[1],c[0]))+(b'' if core else b'\xa5') for c in (palette+[(0,0,0)]*256)[:count]) if indexed else b''
        extra=struct.pack('<III',*masks) if masks else b''
        header=struct.pack('<IHHHH',12,width,height,1,bits) if core else struct.pack('<IiiHHIIiiII',40,width,-height if top else height,1,bits,3 if masks else 0,0,0,0,0 if default_palette else len(palette) if indexed else 0,0)
        stride=((width*bits+31)//32)*4;rows=[];expected=[]
        actual_masks=masks or ((0x7c00,0x3e0,0x1f) if bits==16 else (0xff0000,0xff00,0xff))
        for y in range(height):
            row=bytearray(stride);rgb=[]
            for x in range(width):
                if indexed:
                    index=(x+y)%len(palette);rgb.extend(palette[index])
                    if bits==8:row[x]=index
                    elif bits==4:row[x//2]|=index<<(0 if x%2 else 4)
                    else:row[x//8]|=index<<(7-x%8)
                elif bits==24:
                    c=((x*31+y*13)%256,(x*47+y*11)%256,(x*73+y*19)%256);rgb.extend(c);row[x*3:x*3+3]=bytes(reversed(c))
                else:
                    pixel=0x8000 if bits==16 and not masks else 0xa5000000 if bits==32 and not masks else 0
                    for k,mask in enumerate(actual_masks):
                        shift=(mask&-mask).bit_length()-1;maximum=mask>>shift
                        v=[0,maximum,maximum//2,1][(x+y+k)%4]
                        pixel|=v<<shift;rgb.append(v*255//maximum)
                    row[x*(bits//8):(x+1)*(bits//8)]=pixel.to_bytes(bits//8,'little')
            rows.append(bytes(row));expected.extend(rgb)
        payload=b''.join(rows if top else reversed(rows));offset=14+len(header)+len(extra)+len(table)
        file=bytearray(struct.pack('<2sIHHI',b'BM',offset+len(payload),0,0,offset)+header+extra+table+payload)
        if not core:struct.pack_into('<I',file,34,len(payload))
        emit(name,width,height,file,expected,'core' if core else next(k for k,v in [('indexed1',1),('indexed4',4),('indexed8',8),('rgb555',16),('bgrx32',32),('regression',24)] if v==bits) if not masks else 'masks'+str(bits))
    for family,bits in [('indexed1',1),('indexed4',4),('indexed8',8),('rgb555',16),('bgrx32',32),('masks16',16),('masks32',32)]:
        for width in [1,2,3,4,7,8,9,13,17]:
            for top in [False,True]:
                masks=(0xf800,0x7e0,0x1f) if family=='masks16' else (0x3ff00000,0xffc00,0x3ff) if family=='masks32' else None
                uncompressed(f'{family}-'+('top' if top else 'bottom')+f'-{width}',bits,width,top,masks=masks)
    for bits in [1,4,8,24]:
        for width in [1,2,3,4,7,8,9,13,17]:uncompressed(f'core-{bits}-{width}',bits,width,False,core=True)
    for bits in [1,4,8]:uncompressed(f'indexed{bits}-default',bits,13,False,default_palette=True)
    # Preserve the old installed-domain path as a regression, not an extra task.
    for top in [False,True]:uncompressed('bgr24-'+str(int(top)),24,13,top)
    for bits in [4,8]:
        width=13;height=3
        mixed=bytes([3,0x12 if bits==4 else 1,0,5]+([0x32,0x10,0x30,0] if bits==4 else [3,2,1,0,3,0])+[0,2,2,1,3,0x21 if bits==4 else 2,0,0,0,13]+([0x01,0x23,0x01,0x23,0x01,0x23,0,0] if bits==4 else [i%4 for i in range(13)]+[0])+[0,0,0,1])
        for label,stream,rows in [
            ('mixed',mixed,[[i%4 for i in range(13)],[0]*10+([2,1,2] if bits==4 else [2]*3),([1,2,1] if bits==4 else [1]*3)+[3,2,1,0,3]+[0]*5]),
            ('empty',bytes([0,1]),[[0]*13 for _ in range(3)]),
            ('delta-zero',bytes([0,2,0,0,1,0x30 if bits==4 else 3,0,1]),[[0]*13,[0]*13,[3]+[0]*12]),
            ('final-row',bytes([0,2,0,2,13,0x12 if bits==4 else 1,0,1]),[([1,2]*7)[:13] if bits==4 else [1]*13,[0]*13,[0]*13]),
        ]:
            offset=70;header=struct.pack('<2sIHHIIiiHHIIiiII',b'BM',offset+len(stream),0,0,offset,40,width,height,1,bits,2 if bits==4 else 1,len(stream),0,0,4,0)
            table=b''.join(bytes([c[2],c[1],c[0],0]) for c in PALETTE)
            emit(f'rle{bits}-{label}',width,height,header+table+stream,[v for row in rows for i in row for v in PALETTE[i]],'rle'+str(bits))
    (root/'cases.txt').write_text(''.join(records));(root/'malformed.bmp').write_bytes(b'BM')
    (root/'.generator-sha256').write_text(hashlib.sha256(Path(__file__).read_bytes()).hexdigest()+'\n')
    return families


def digest(p):return hashlib.sha256(read_bytes(p,root=ROOT)).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path);parser.add_argument('--report',type=Path);parser.add_argument('--reuse-builds',type=Path);parser.add_argument('--fixtures-only',type=Path);args=parser.parse_args()
    if args.fixtures_only:
        target=args.fixtures_only.resolve()
        if not target.is_relative_to(ROOT/'working'):parser.error('Fixture output must be under owned working/')
        if target.exists():
            if not (target/'cases.txt').is_file():parser.error('Refuse unrelated existing fixture directory')
            marker=target/'.generator-sha256'
            if not marker.is_file() or marker.read_text().strip()!=hashlib.sha256(Path(__file__).read_bytes()).hexdigest():parser.error('Existing fixture generator is stale; select a new output directory')
        else:print(json.dumps(fixtures(target)))
        return 0
    if not args.output or not args.report:parser.error('Require output and report')
    out=args.output.resolve();destination=args.report.absolute()
    if out.exists() or destination.exists() or not out.is_relative_to(ROOT/'working/tests') or not destination.is_relative_to(ROOT/'research/runtime'):parser.error('Require new owned immutable paths')
    buildroot=args.reuse_builds.resolve() if args.reuse_builds else out
    if args.reuse_builds and (not buildroot.is_relative_to(ROOT/'working/tests') or not (buildroot/'.retain').is_file()):parser.error('Require retained owned build root')
    out.mkdir(parents=True);(out/'.retain').write_text('Retain N25 raw fixtures, binaries and execution logs.\n');(out/'tmp').mkdir()
    env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2')
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes());bs={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    paths=set().union(*(required_sources(bs[i])|set(bs[i]['documents']) for i in IDS));candidates={}
    # Pin the native asset/renderer build input domain before execution. Compiler
    # dependency files below must be a subset; this includes linked utility code.
    for base in ['assets','renderer']:
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/base).rglob('*') if p.is_file() and (p.suffix in {'.hpp','.h','.inc','.cpp','.c','.S'} or p.name=='CMakeLists.txt'))
    for name in subprocess.check_output(['rg','--files'],cwd=ROOT,text=True).splitlines():
        p=ROOT/name
        if p.suffix in {'.hpp','.h','.inc','.cpp','.c','.S'}:candidates.setdefault(p.name,[]).append(p)
    pending=list(paths);visited=set()
    while pending:
        name=pending.pop()
        if name in visited:continue
        visited.add(name);p=ROOT/name
        if p.suffix not in {'.hpp','.h','.inc','.cpp','.c','.S'}:continue
        for include in re.findall(r'^\s*#\s*include\s+"([^"]+)"',p.read_text(),re.M):
            direct=(p.parent/include).resolve()
            for t in [direct] if direct.is_file() else candidates.get(Path(include).name,[]):
                if t.is_relative_to(ROOT) and not t.is_relative_to(ROOT/'working'):
                    rel=t.relative_to(ROOT).as_posix()
                    if rel not in paths:paths.add(rel);pending.append(rel)
    sources={p:digest(ROOT/p) for p in sorted(paths)}
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds)) for i in IDS],commands=[],native={},inputs={},original_game_executed=False,original_artifacts_consumed=False,live_replacement=False,
        source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix(),scope='Ten native BMP decoder domains under partial AS.bmp. CPU fixture byte/RGB comparisons, bounded hostile-input controls and real resource-manager/RGB565/upload/draw/retirement composition on software OpenGL. Native policy only; no original loader comparison, installed asset domain expansion requirement, whole-scene/hardware/performance or live acceptance. Explicit defaults and mask rounding are native choices.')
    def run(label,argv,timeout=1800):
        argv=[str(x) for x in argv];res=subprocess.run(argv,cwd=ROOT,env=env,capture_output=True,timeout=timeout);data=(res.stdout+b'\n'+res.stderr).decode(errors='replace');(out/(label+'.log')).write_text(data)
        report['commands'].append(dict(label=label,argv=argv,exit_code=res.returncode,output_sha256=hashlib.sha256(data.encode()).hexdigest(),output_tail=data[-4000:]));res.check_returncode();return res.stdout.decode()
    try:
        report['families']=fixtures(out/'fixtures');assert all(report['families'].values())
        report['inputs']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((out/'fixtures').iterdir())}
        report['sanitizer_scope']='ASan and UBSan. External Qt/driver leak scanning disabled; decoded/GPU retirement balances checked explicitly.'
        for mode,flags in [('release',['-DCMAKE_BUILD_TYPE=Release']),('sanitized',['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=buildroot/(mode+'-build');print('Building '+mode,flush=True)
            run(mode+'-configure',['cmake','-S',ROOT/'renderer/resources','-B',build,*flags]);run(mode+'-build',['cmake','--build',build,'--target','bmp-encoding-test','bmp-loader-test','bmp-rendering-test','resource-manager-test','resource-preparation-test','mnm-bmp-inspect','-j2'])
            cpu=build/'resources/assets';gpu=build/'bmp-rendering-test';resource=build/'resources'
            report['native'][mode]=dict(cpu=json.loads(run(mode+'-cpu',[cpu/'bmp-encoding-test',out/'fixtures'])),gpu=json.loads(run(mode+'-gpu',[sys.executable,ROOT/'tools/run-surface-wrapper-control.py','--log-root',out/(mode+'-display'),'--',gpu,out/'fixtures'])))
            for label,binary in [('baseline',cpu/'bmp-loader-test'),('manager',resource/'resource-manager-test'),('preparation',resource/'resource-preparation-test')]:run(mode+'-'+label,[binary])
            report['native'][mode]['baseline_manager_preparation_pass']=True
            run(mode+'-ctest',['ctest','--test-dir',build,'--output-on-failure','-R','^(bmp-native-encodings|native-bmp-rendering)$'])
            report['native'][mode]['ctest_wiring_pass']=True
            report['native'][mode]['binary_sha256']={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [cpu/'bmp-encoding-test',gpu]}
            deps=set()
            for p in build.rglob('*.o.d'):
                for token in p.read_text().replace('\\\n',' ').split()[1:]:
                    t=Path(token).resolve()
                    if t.is_file() and t.is_relative_to(ROOT) and not t.is_relative_to(ROOT/'working'):deps.add(t.relative_to(ROOT).as_posix())
            if deps-set(sources):raise ValueError('Undeclared compiler inputs: '+str(sorted(deps-set(sources))))
            report['native'][mode]['declared_compiler_inputs']=sorted(deps)
        report['success']=True
    except Exception as error:
        report['error']=repr(error)
        report['owned_source_variants']={p:(ROOT/p).read_text() for p in paths if p in {'assets/bmp.cpp','assets/bmp.hpp','assets/CMakeLists.txt','renderer/resources/CMakeLists.txt','tests/bmp-loader-test.cpp','tests/bmp-encoding-test.cpp','tests/bmp-rendering-test.cpp','tools/test-rendering-twentyfifth-ten.py'}}
    finally:
        report['sources_stable']=all(digest(ROOT/p)==h for p,h in sources.items());report['inputs_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inputs'].items());report['success']=bool(report['success'] and report['sources_stable'] and report['inputs_stable'])
        write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','sources_stable','inputs_stable']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())

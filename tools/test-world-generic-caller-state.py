#!/usr/bin/env python3
"""Compare proposed generic World state and independent pixels with original code."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import shlex
import struct
import subprocess
import tempfile
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]
MAIN=(ROOT/subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--git-common-dir'],text=True).strip()).resolve().parent
PREFIX='research/runtime/caller-state-results-20261108/'
ORIGINAL='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES=(0x5947b2,0x59521a,0x595b47,0x59603e,0x595677,0x596490,0x5968a4)

def analyze(raw):
    if raw[:16]!=b'MNMGENS1'+struct.pack('<II',1,320)or (len(raw)-16)%320:
        raise ValueError('Incomplete generic state wire')
    counts=Counter();examples=[]
    for index,at in enumerate(range(16,len(raw),320)):
        row=struct.unpack_from('<80I',raw,at)
        n=index;variant=n%12;n//=12;placement=n%12;n//=12;seed=n%35;kind=n//35
        if kind>=7 or row[:6]!=(index+1,ENTRIES[kind],kind if kind<5 else 6,seed,placement,variant):
            raise ValueError('Missing/repeated generic state domain')
        if row[6]&~255 or row[7]not in(0xcc5,0xcd5)or row[79]or row[72]>1 or row[73]>1:
            raise ValueError('Unknown state bits or reserved/domain fields')
        wanted_cw=0x7f|((0,2,3)[variant//4]<<8)|((variant%4)<<10)
        if row[69]!=wanted_cw:raise ValueError('Declared floating controls not exercised')
        counts['cases']+=1;counts['complete_matches']+=row[6]==255
        for bit,name in enumerate(('workspace','arguments','gpr','flags','floating_state','pixels_source_guards','native_admission','commit_canaries')):
            counts[name+'_matches']+=bool(row[6]&(1<<bit))
        if row[6]!=255 and len(examples)<12:examples.append(dict(case=index,entry=hex(row[1]),seed=seed,placement=placement,variant=variant,mask=row[6],
            original_words=list(row[8:24]),native_words=list(row[24:40]),original_args=list(row[56:61]),native_args=list(row[61:66]),
            original_flags=row[66],native_flags=row[67],original_status=row[70],native_status=row[71]))
    return dict(counts=dict(counts),examples=examples,complete=counts['cases']==35280,
                equivalent=counts['cases']==35280 and counts['complete_matches']==35280)

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--name',required=True);parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    if not re.fullmatch('[a-z0-9-]+',args.name):parser.error('Unique lowercase name required')
    destination=ROOT/(PREFIX+args.name+'.json')
    if destination.exists():raise ValueError('Preserve prior immutable evidence')
    claims=json.loads(args.claims.read_text());sha=lambda b:hashlib.sha256(b).hexdigest();frozen={}
    for folder in ('runtime','reconstruction','renderer','tests','protocols'):
        for path in (ROOT/folder).rglob('*'):
            if path.is_file()and path.suffix in('.c','.h','.cpp','.hpp','.S'):frozen[path.relative_to(ROOT).as_posix()]=path.read_bytes()
    fixed=set(claims['sources'])|{'tools/test-world-generic-caller-state.py','tools/original-manifest.sh'}
    for p in fixed:frozen[p]=(ROOT/p).read_bytes()
    if any(sha(frozen[p])!=h for p,h in claims['sources'].items()):raise ValueError('Prospective source changed')
    used=set(fixed)
    report=dict(schema=1,kind='world_generic_caller_state_comparison',success=False,claims=claims['claims'],
        original_sha256=ORIGINAL,original_game_runs=0,newly_passed_requirements=0,commands=[],
        scope='Private unchanged original generic indexed/paired, black and displacement routines versus proposed prepared state and existing native CPU pixels. Seven entries, clipped/hidden/empty/refused geometry, padding, twelve masked precision/rounding modes and sixteen incoming x87 condition patterns. Full GPR, five arguments, scratch workspace, defined flags/DF, selected semantic FX and complete guarded pixels. No production callback installation, batch transport, driver or player acceptance claim.')
    out=Path(tempfile.mkdtemp(prefix='mnm-world-generic-state-',dir='/tmp'))
    report.update(temporary_directory=str(out),temporary_classification='Compiler objects/depfiles and RAM staging are reproducible temporary output. The exact tested executable, deterministic source/seed domains, state wire, result bytes and logs are retained in SQLite. RAM data is held for reviewed cleanup, including failed attempts.')
    def retain(label,raw):
        path=PREFIX+args.name+'-'+label;write_report(ROOT,path,raw)
        return dict(path=path,sha256=sha(raw),bytes=len(raw))
    def run(label,argv,*,check=True):
        r=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,timeout=180)
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode,log=retain(label+'.log.bin',r.stdout+r.stderr)))
        if check and r.returncode:raise RuntimeError(label+': exit '+str(r.returncode))
        return r
    try:
        # Compile refusals need exact unique source preimages too.
        report['source_preimages']={}
        for p in sorted(fixed):
            old=subprocess.run(['git','show','HEAD:'+p],cwd=ROOT,capture_output=True)
            if old.returncode or old.stdout!=frozen[p]:report['source_preimages'][p]=retain('source-'+sha(p.encode())[:16]+'.bin',frozen[p])
        run('manifest-before',[MAIN/'tools/original-manifest.sh','verify'])
        original=MAIN/'original/Arcane_Nocd/Chaos.exe'
        if sha(original.read_bytes())!=ORIGINAL:raise ValueError('Unsupported original image')
        run('clang-version',['clang','--version']);run('cxx-version',['clang++','--version'])
        objects=[]
        for index,name in enumerate(('reconstruction/rendering/raster_pixels.c','reconstruction/rendering/pixel_kernel.c','tests/world-generic-caller-state-probe.S')):
            obj=out/(str(index)+'.o');objects.append(obj)
            run('compile-'+str(index),['clang','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
                '-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj])
        exe=out/'reference'
        run('link',['clang++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-fdeclspec','-Wno-ignored-attributes','-Wall','-Wextra','-Werror',
            '-MMD','-MF',out/'reference.d',ROOT/'tests/world-generic-caller-state-reference.cpp',*objects,'-o',exe])
        for dep in out.glob('*.d'):
            for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT)and not path.is_relative_to(ROOT/'working'):used.add(path.relative_to(ROOT).as_posix())
        if used-frozen.keys():raise ValueError('Unfrozen compiler closure: '+repr(sorted(used-frozen.keys())))
        if any((ROOT/p).read_bytes()!=frozen[p]for p in used):raise ValueError('Sources changed before execution')
        for p in sorted(used):
            old=subprocess.run(['git','show','HEAD:'+p],cwd=ROOT,capture_output=True)
            if old.returncode or old.stdout!=frozen[p]:report['source_preimages'][p]=retain('source-'+sha(p.encode())[:16]+'.bin',frozen[p])
        report['tested_executable']=retain('reference.bin',exe.read_bytes())
        result=run('original-native',[exe,original,out/'states.bin',out/'comparison.json'],check=False)
        report['comparison']=analyze((out/'states.bin').read_bytes())
        report['native_result']=json.loads((out/'comparison.json').read_text())
        report['success']=result.returncode==0 and report['comparison']['equivalent']and report['native_result']['success']
    except (ValueError,OSError,RuntimeError,subprocess.SubprocessError)as error:report['error']=str(error)
    finally:
        report['artifacts']={}
        for name in ('states.bin','comparison.json'):
            path=out/name
            if path.is_file():report['artifacts'][name]=retain(name+'.bin',path.read_bytes())
        try:run('manifest-after',[MAIN/'tools/original-manifest.sh','verify'])
        except (OSError,RuntimeError,subprocess.SubprocessError)as error:report.update(success=False,manifest_error=str(error))
        report['sources']={p:sha(frozen[p])for p in sorted(used&frozen.keys())}
        report['sources_stable']=all((ROOT/p).read_bytes()==frozen[p]for p in used&frozen.keys())
        report['inputs_stable']=sha((MAIN/'original/Arcane_Nocd/Chaos.exe').read_bytes())==ORIGINAL
        report['success']=report['success']and report['sources_stable']and report['inputs_stable']
        write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'],error=report.get('error'),comparison=report.get('comparison',{}).get('counts'))))
    return 0 if report['success']else 1

if __name__=='__main__':raise SystemExit(main())

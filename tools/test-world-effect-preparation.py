#!/usr/bin/env python3
"""Compare retained effect-table initialization and outgoing floating state."""
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
def analyze(raw):
    if raw[:16]!=b'MNMEFX01'+struct.pack('<II',1,3392)or (len(raw)-16)%3392:
        raise ValueError('Incomplete effect initializer wire')
    counts=Counter();examples=[]
    for index,at in enumerate(range(16,len(raw),3392)):
        row=struct.unpack_from('<16I',raw,at)
        n=index;positive=n%2;n//=2;condition=n%16;n//=16;variant=n%12;n//=12;amplitude=n%16+1;kind=n//16
        if kind>=2 or row[:6]!=(index+1,kind,amplitude,variant,condition,positive)or any(row[11:]):
            raise ValueError('Missing/repeated initializer case or reserved fields')
        if any(x not in(0,1)for x in row[6:9]):raise ValueError('Unknown result flags')
        counts['cases']+=1;counts['floating_state_matches']+=row[6];counts['table_matches']+=row[7];counts['table_changed']+=row[8]
        if not all(row[6:9])and len(examples)<12:examples.append(dict(case=index,fields=list(row)))
    return dict(counts=dict(counts),examples=examples,complete=counts['cases']==12288,
                equivalent=counts['cases']==12288 and all(counts[k]==12288 for k in('floating_state_matches','table_matches','table_changed')))

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
    fixed=set(claims['sources'])|{'tools/test-world-effect-preparation.py','tools/original-manifest.sh'}
    for p in fixed:frozen[p]=(ROOT/p).read_bytes()
    if any(sha(frozen[p])!=h for p,h in claims['sources'].items()):raise ValueError('Prospective source changed')
    used=set(fixed)
    report=dict(schema=1,kind='world_effect_initializer_state_comparison',success=False,claims=claims['claims'],
        original_sha256=ORIGINAL,original_game_runs=0,newly_passed_requirements=0,commands=[],
        scope='Private unchanged original wave5806f0/source-row580260 routines and unchanged conversion helper versus retained zero-frame initializer executed with incoming engine FX. Exact initialized table bytes and semantic CW/SW/tag/active values/XMM/MXCSR across sixteen amplitudes, twelve precision/rounding modes, sixteen condition patterns and empty/positive hidden geometry. No native raster pixel comparison, production callback installation or player acceptance claim.')
    out=Path(tempfile.mkdtemp(prefix='mnm-world-effect-state-',dir='/tmp'))
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
        run('manifest-before',[MAIN/'tools/original-manifest.sh','verify'])
        original=MAIN/'original/Arcane_Nocd/Chaos.exe'
        if sha(original.read_bytes())!=ORIGINAL:raise ValueError('Unsupported original image')
        run('clang-version',['clang','--version']);run('cxx-version',['clang++','--version'])
        objects=[]
        for index,name in enumerate(('tests/world-generic-caller-state-probe.S',)):
            obj=out/(str(index)+'.o');objects.append(obj)
            run('compile-'+str(index),['clang','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',
                '-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj])
        exe=out/'reference'
        run('link',['clang++','-m32','-O2','-std=c++17','-mno-sse','-mno-mmx','-fno-pie','-no-pie','-fdeclspec','-Wno-ignored-attributes','-Wall','-Wextra','-Werror',
            '-MMD','-MF',out/'reference.d',ROOT/'tests/world-effect-prepare-reference.cpp',*objects,'-o',exe])
        for dep in out.glob('*.d'):
            for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT)and not path.is_relative_to(ROOT/'working'):used.add(path.relative_to(ROOT).as_posix())
        if used-frozen.keys():raise ValueError('Unfrozen compiler closure: '+repr(sorted(used-frozen.keys())))
        if any((ROOT/p).read_bytes()!=frozen[p]for p in used):raise ValueError('Sources changed before execution')
        report['source_preimages']={}
        for p in sorted(used):
            old=subprocess.run(['git','show','HEAD:'+p],cwd=ROOT,capture_output=True)
            if old.returncode or old.stdout!=frozen[p]:report['source_preimages'][p]=retain('source-'+sha(p.encode())[:16]+'.bin',frozen[p])
        report['tested_executable']=retain('reference.bin',exe.read_bytes())
        result=run('original-native',[exe,original,out/'states.bin',out/'comparison.json'],check=False)
        report['comparison']=analyze((out/'states.bin').read_bytes())
        report['native_result']=json.loads((out/'comparison.json').read_text())
        report['success']=result.returncode==0 and report['comparison']['equivalent']and report['native_result']['success']and report['native_result']['policy_refusals']==5
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

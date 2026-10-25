#!/usr/bin/env python3
"""Compare the complete original cursor setup caller with scripted OS/CRT/resources."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import write_report
from original_x86 import PE32
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original-root',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--report',required=True)
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/a.report).exists():p.error('Require new owned output and report paths')
    out.mkdir(parents=True);(out/'.retain').write_text('Unchanged original cursor setup, scripted endpoints and exact execution logs/binaries.\n')
    original_root=a.original_root.resolve();image=original_root/'original/Arcane_Nocd/Chaos.exe'
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='DRAW.cursor-resource-setup')
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(required_sources(b))}
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{b['id']:b for b in r['builds']}),scenarios={})],commands=[],original_sha256=BUILD,original_game_executed=False,live_replacement=False,all_source_alias_ownership_proved=False,scope=b['scope'])
    def run(label,argv):
        log=out/(label+'.log');print(label,flush=True)
        with log.open('x') as f:
            result=subprocess.run(list(map(str,argv)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120,env=os.environ|dict(TMPDIR=str(ROOT/'working/tmp'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'))
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),exit=result.returncode,log=log.relative_to(ROOT).as_posix()))
        if result.returncode:raise ValueError(label+': '+log.read_text(errors='replace')[-3000:])
        return log.read_text()
    verified=False
    try:
        run('original-before',[original_root/'tools/original-manifest.sh','verify']);verified=True
        raw=image.read_bytes()
        if hashlib.sha256(raw).hexdigest()!=BUILD:raise ValueError('Unsupported original image')
        pe=PE32(raw);report['original_functions']={hex(0x4a30b0):dict(bytes=1643,sha256=hashlib.sha256(pe.read(0x4a30b0,1643)).hexdigest())}
        for name,flags in [('optimized',['-O2']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            dep=out/(name+'.d');binary=out/name
            run(name+'-build',['g++','-m32','-std=c++17','-fno-pie','-no-pie','-mstackrealign','-Wall','-Wextra','-Werror',*flags,'-MMD','-MF',dep,ROOT/'tests/cursor-setup-reference.cpp','-o',binary])
            for item in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(item).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiled dependency '+str(path))
            report[name]=json.loads(run(name,[binary,image]))
            if report[name]!=dict(success=True,cases=528,descriptors_per_case=22,profile_calls_per_case=90,allocation_calls_per_case=2,original_instructions_unchanged=True):raise ValueError('Incomplete setup execution denominator')
        report['success']=True
    except (ValueError,OSError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        if verified:
            try:run('original-after',[original_root/'tools/original-manifest.sh','verify']);report['manifest_before_after']=True
            except (OSError,ValueError,subprocess.SubprocessError) as exc:report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success'] &= report['sources_stable']
        report['artifacts']={f.relative_to(ROOT).as_posix():dict(bytes=f.stat().st_size,sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in out.iterdir() if f.is_file()}
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'),optimized=report.get('optimized'),sanitized=report.get('sanitized'))),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())

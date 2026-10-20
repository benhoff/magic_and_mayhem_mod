#!/usr/bin/env python3
"""Execute original empty-frame effect table preparation without readable pixel inputs."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=json.load(open(ROOT/'research/runtime/coverage/register.json'));b=next(b for b in r['behaviors'] if b['id']=='NP.world-effect-producer-admission');s=next(s for s in r['scenarios'] if s['id']=='native-world-effect-producers')
    paths=required_sources(b)
    return [dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe');a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();report=dict(schema=1,success=False,claims=claims,sources=sources,original_sha256=BUILD,scope=__doc__,live_replacement=False,original_instructions_unchanged=True)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd):
        with (out/(name+'.log')).open('x') as f:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=300)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if sha(a.executable)!=BUILD:raise ValueError('Pinned original required')
        run('build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-effect-table-reference.cpp','-o',out/'reference'])
        run('original',[out/'reference',a.executable]);result=json.loads((out/'original.log').read_text())
        if not result['success'] or len(result['source_tables'])!=16 or len(result['wave_words'])!=256 or result['cached_calls_unchanged']!=17:raise ValueError('Incomplete preparation matrix')
        if declaration()!=(claims,sources) or sha(a.executable)!=BUILD:raise ValueError('Source/original changed')
        report.update(success=True,sources_stable=True,empty_frame_non_drawing=True,result=result)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()

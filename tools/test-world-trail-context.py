#!/usr/bin/env python3
"""Execute the production context-bound kind9 leaf guard with original queue/helper stacks."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIOR='NP.world-trail-caller-admission'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=json.load(open(ROOT/'research/runtime/coverage/register.json'));b=next(b for b in r['behaviors'] if b['id']==BEHAVIOR);s=next(s for s in r['scenarios'] if s['id']=='native-world-trail-context')
    sources=required_sources(b)|set(s['tests'])
    for directory in ['runtime/scene','runtime/shadow','protocols/include']:
        sources.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and p.suffix in ['.c','.h','.S'])
    return [dict(behavior=BEHAVIOR,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(sources)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe');a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();report=dict(schema=1,success=False,claims=claims,sources=sources,original_sha256=BUILD,live_replacement=False,scope='Five pinned580a90 continuation signatures under actual4df840 primary/software stack ancestry, seven-byte prologue and two queue returns. Production C guard extracted without reimplementation. Synthetic leaf binding and memory-query facade; no original raster pixels, Windows query equivalence, natural live reachability, writer/source ownership or full replacement.')
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd):
        with (out/(name+'.stdout')).open('x') as f,(out/(name+'.stderr')).open('x') as e:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=e,check=True,timeout=300)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if sha(a.executable)!=BUILD:raise ValueError('Unsupported original')
        source=(ROOT/'runtime/scene/world_producer_bypass.c').read_text();start=source.index('static int raster_caller(');end=source.index('\n\nstatic u32 bypass_hash',start);wrapper=source[start:end]+'\n';(out/'trail-caller-wrapper.inc').write_text(wrapper)
        for name,file in [('native','world-trail-context-test.cpp'),('original','world-trail-context-reference.cpp')]:
            run(name+'-build',['g++','-m32','-fno-pie','-no-pie','-fno-omit-frame-pointer','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests'/file,'-o',out/name])
            run(name,[out/name,*([a.executable] if name=='original' else [])])
        native=json.loads((out/'native.stdout').read_text());original=json.loads((out/'original.stdout').read_text())
        if not native.get('success') or native['context_pairs']!=10 or original['cases']!=127449 or original['sites']!=5 or original['parents']!=2:raise ValueError('Incomplete context comparison')
        run('callers',['python3',ROOT/'tools/test-world-raster-callers.py','--output',out/'callers'])
        callers=json.load(open(out/'callers/report.json'))
        if len(json.load(open(out/'callers/generated.json')).get('trail_context',[]))!=7:raise ValueError('Generator lost context bindings')
        if declaration()!=(claims,sources) or sha(a.executable)!=BUILD:raise ValueError('Source/input changed')
        report.update(success=True,sources_stable=True,native=native,original=original,original_queue_helper_instructions_unchanged=True,actual_leaf_stack_parent_matches=True,production_guard_extracted=True,guard_source_sha256=hashlib.sha256(wrapper.encode()).hexdigest(),original_raster_synthetic=True,production_and_selftest_hooks_compile=callers['production_and_selftest_hooks_compile'],caller_regression_sha256=sha(out/'callers/report.json'))
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()

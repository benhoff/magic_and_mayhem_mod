#!/usr/bin/env python3
"""Compare owned actor ownership/lifecycle models with unchanged PE32 bodies."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SUITES={'pool':('RS.actor-pool-admission',15360),
        'references':('RS.actor-reference-invalidation',15360),
        'map':('RS.actor-map-occupancy',10368),
        'release':('RS.actor-cleanup-release',3840),
        'activation':('RS.actor-activation-lifecycle',2592)}


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--suite',choices=SUITES,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require new retained output under working/tests')
    (out/'tmp').mkdir(parents=True)
    env=dict(os.environ,TMPDIR=str(out/'tmp'))
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bid,cases=SUITES[args.suite];behavior=next(b for b in register['behaviors'] if b['id']==bid)
    builds={b['id']:b for b in register['builds']}
    paths=required_sources(behavior)|{'tools/coverage_claims.py'}
    sources={p:sha(ROOT/p) for p in sorted(paths)}
    # Keep only this suite's small exact source closure, including failed-run
    # variants. This is neither a full checkout nor renewed historical proof.
    for p in sources:
        target=out/'sources'/p;target.parent.mkdir(parents=True,exist_ok=True)
        target.write_bytes((ROOT/p).read_bytes())
    report=dict(schema=1,success=False,sources=sources,original_sha256=PIN,
        scope=behavior['scope'],claims=[dict(behavior=bid,contract_sha256=behavior_contract(behavior,builds))],
        live_replacement=False,original_output_used_as_native_input=False,
        physical_driver_equivalence=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')

    def run(label,command,stream=False):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as log:
            if not stream:
                subprocess.run(list(map(str,command)),cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240)
                return
            with (out/(label+'.bin.zst')).open('xb') as target:
                encoder=subprocess.Popen(['zstd','-q','-3','-c'],stdin=subprocess.PIPE,stdout=target,stderr=log)
                child=subprocess.Popen(list(map(str,command)),cwd=ROOT,env=env,stdout=encoder.stdin,stderr=log)
                encoder.stdin.close()
                try:
                    if child.wait(timeout=240) or encoder.wait(timeout=30):
                        raise ValueError(label+' failed; inspect retained log')
                finally:
                    for p in [child,encoder]:
                        if p.poll() is None:p.kill()
                        p.wait()
    verified=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        original=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original)!=PIN:raise ValueError('Unreviewed original image')
        for name,flags in [('native',[]),('original',['-m32','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE'])]:
            run(name+'-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,
                '-MD','-MF',out/(name+'.d'),ROOT/'tests/actor-lifecycle-reference.cpp','-o',out/name])
            for p in (out/(name+'.d')).read_text().replace('\\\n',' ').split()[1:]:
                path=Path(p)
                if path.is_file() and path.resolve().is_relative_to(ROOT) and path.resolve().relative_to(ROOT).as_posix() not in sources:
                    raise ValueError('Undeclared compiler input: '+p)
            command=['bwrap','--die-with-parent','--unshare-net','--ro-bind','/','/',
                '--bind',out,out,'--dev','/dev','--proc','/proc','--',out/name]
            if name=='original':command.append(original)
            command.append(args.suite);run(name+'-execute',command,True)
            if 'cases '+str(cases) not in (out/(name+'-execute.log')).read_text():
                raise ValueError('Declared case matrix not completed: '+name)
        children=[subprocess.Popen(['zstd','-qdc',out/(name+'-execute.bin.zst')],stdout=subprocess.PIPE) for name in ['native','original']]
        count=0;digest=hashlib.sha256()
        try:
            while True:
                left=children[0].stdout.read(65536);right=children[1].stdout.read(65536)
                if left!=right:
                    at=next((i for i,(a,b) in enumerate(zip(left,right)) if a!=b),min(len(left),len(right)))
                    report['mismatch']={'byte':count+at,'native':left[max(0,at-16):at+32].hex(),'original':right[max(0,at-16):at+32].hex()}
                    raise ValueError('Actor lifecycle bytes differ')
                if not left:break
                digest.update(left);count+=len(left)
            if any(p.wait(timeout=10) for p in children):raise ValueError('Compressed output read failed')
        finally:
            for p in children:
                if p.poll() is None:p.kill()
                p.wait();p.stdout.close()
        report.update(success=True,cases=cases,bytes_compared=count,comparison_sha256=digest.hexdigest(),
            binaries_sha256={name:sha(out/name) for name in ['native','original']},
            outputs_sha256={name+'-execute.bin.zst':sha(out/(name+'-execute.bin.zst')) for name in ['native','original']},
            selected_original_bodies_unchanged=True)
    except (OSError,ValueError,subprocess.SubprocessError) as error:
        report['error']=str(error)
    finally:
        if verified:
            run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json')
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())

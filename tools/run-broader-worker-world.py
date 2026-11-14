#!/usr/bin/env python3
"""Execute one frozen current-player World case through its existing controller."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys

import coverage_evidence
from broader_worker_world_launch import WorldBinding
from coverage_evidence import Reader
from native_render_driver import require_nvidia
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plan',required=True)
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED')!='1':
        runtime=ROOT/'working/broader-player/world-runtime'/Path(args.report).stem
        runtime.mkdir(mode=0o700,parents=True,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_WORLD_BUS_OWNED='1',
            MNM_INTERACTION_BUS_OWNED='1',MNM_PAUSE_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    with Reader(ROOT)as r:
        plan=r.json(args.plan)
        require(plan['success']and plan['kind']=='current_broader_world_prospective_plan'
                and all(r.digest(n)==h for n,h in(plan['sources']|plan['compiler_sources']|plan['runtime_sources']).items()),'Current World phase changed')
        cases=[c for c in plan['broader_cases']if c['report']==args.report]
        require(len(cases)==1,'World case is not prospectively bound')
        case=cases[0];phase=dict(record=args.plan,sha256=r.digest(args.plan))
        binary=ROOT/plan['player_binary']['path']
        require(hashlib.sha256(binary.read_bytes()).hexdigest()==plan['player_binary']['sha256'],'Current tested player differs')
    binding=WorldBinding(ROOT,plan,args.report,case['provider'],args.plan)
    original_write=coverage_evidence.write_report
    def writer(root,path,raw):
        if str(path)==args.report:
            q=json.loads(raw);q['delegate_success']=q['success']
            same_binary=hashlib.sha256(binary.read_bytes()).hexdigest()==plan['player_binary']['sha256']
            stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in(plan['sources']|plan['compiler_sources']|plan['runtime_sources']).items())
            q.setdefault('sources',{}).update(plan['sources']|plan['compiler_sources']|plan['runtime_sources'])
            q['sources_stable']=q.get('sources_stable',False)and stable
            try:
                require(same_binary,'Original controller rebuilt a different player; no current combined phase')
                q['provider_adaptation']=binding.receipt()|dict(phase=phase)
                driver=q.get('resident',{}).get('renderer')
                if case['provider']=='nvidia':require_nvidia([driver])
                else:require(isinstance(driver,str)and driver.lower().startswith(('llvmpipe','softpipe')),'Actual software renderer missing')
                q['actual_provider_verified']=True
            except ValueError as error:
                q.update(success=False,provider_admission_error=str(error),actual_provider_verified=False)
            q['broader_phase']=phase
            if plan['family'] in ('quick','storage'):
                q.update(current_player_plan=phase,current_player_build=plan['current_player_build'],
                    current_player_profile=case['profile'],current_player_scenario=plan['family'],
                    runtime_sources_stable=stable,native_runtime_sources=sorted(plan['runtime_sources']))
            q['success']=q['success']and q['sources_stable']
            raw=(json.dumps(q,indent=2)+'\n').encode()
        return original_write(root,path,raw)
    coverage_evidence.write_report=writer
    binding.install()
    try:
        # Only the owned source fixture's explicitly permitted CFG selector is
        # changed. Every collector then retains the actual staged input bytes.
        patcher=binding.load('run-opengl-game.py')
        patcher.patch_staged_preferences(ROOT/'working/game-nocd','VIDEO',{'IsHighRes':'FALSE'if case['size'][0]==640 else'TRUE'})
        core=binding.load(case['controller_file'])
        if case.get('prepare_progression'):
            prep=binding.load('prepare-rendering-progression.py')
            sys.argv=[str(ROOT/'tools/prepare-rendering-progression.py'),'--game',str(ROOT/'working/game-nocd'),'--report',case['prepare_progression']]
            require(prep.main()in(None,0),'Current original progression input preparation refused')
        sys.argv=[str(ROOT/'tools'/case['controller_file']),*case['controller_arguments']]
        return core.main()
    finally:
        binding.restore();coverage_evidence.write_report=original_write


if __name__=='__main__':raise SystemExit(main())

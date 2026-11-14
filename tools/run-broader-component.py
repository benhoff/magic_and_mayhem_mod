#!/usr/bin/env python3
"""Bind selected-plane clipping/stretch controllers to the current real player."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys

import coverage_evidence
from broader_world_launch import WorldBinding
from coverage_evidence import Reader
from native_render_driver import require_nvidia
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--plan',required=True);p.add_argument('--report',required=True);args=p.parse_args()
    if os.environ.get('MNM_BROADER_COMPONENT_BUS_OWNED')!='1':
        runtime=ROOT/'working/broader-player/component-runtime'/Path(args.report).stem
        runtime.mkdir(mode=0o700,parents=True,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_COMPONENT_BUS_OWNED='1',MNM_INTERACTION_BUS_OWNED='1',MNM_COPY_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    with Reader(ROOT)as r:
        plan=r.json(args.plan);require(plan['success']and plan['kind']=='current_broader_component_prospective_plan'and all(r.digest(n)==h for n,h in plan['sources'].items()),'Current component plan changed')
        cases=[v for v in plan['broader_cases']if v['report']==args.report];require(len(cases)==1,'Unbound component domain');case=cases[0]
        phase=dict(record=args.plan,sha256=r.digest(args.plan))
    binding=WorldBinding(ROOT,plan,args.report,case['provider'],args.plan)
    original_write=coverage_evidence.write_report
    def writer(root,path,raw):
        if str(path)==args.report:
            q=json.loads(raw);q['delegate_success']=q['success']
            stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in plan['sources'].items())
            q.setdefault('sources',{}).update(plan['sources']);q['sources_stable']=q.get('sources_stable',False)and stable
            q['broader_phase']=phase
            try:
                require(hashlib.sha256((ROOT/plan['player_binary']['path']).read_bytes()).hexdigest()==plan['player_binary']['sha256'],'Current component player changed')
                q['provider_adaptation']=binding.receipt()|dict(phase=phase)
                if 'resident'not in q:
                    refs=[v for v in q.get('artifacts',[])if Path(v['path']).name=='resident-world-report.json'];require(len(refs)==1,'Missing unique actual component renderer report')
                    with Reader(ROOT)as r:
                        require(r.digest(refs[0]['record'])==refs[0]['sha256'],'Component renderer report changed')
                        q['resident']=r.json(refs[0]['record'])
                driver=q['resident']['renderer']
                if case['provider']=='nvidia':require_nvidia([driver])
                else:require(driver.lower().startswith(('llvmpipe','softpipe')),'Actual software component renderer missing')
                q['actual_provider_verified']=True
            except(ValueError,KeyError)as error:q.update(success=False,actual_provider_verified=False,provider_admission_error=str(error))
            if plan['family']=='stretch':q['configuration']['software_flag']=q['configuration']['software']
            q['success']=q['success']and q['sources_stable']
            raw=(json.dumps(q,indent=2)+'\n').encode()
        return original_write(root,path,raw)
    coverage_evidence.write_report=writer;binding.install()
    try:
        patcher=binding.load('run-opengl-game.py')
        patcher.patch_staged_preferences(ROOT/'working/game-nocd','VIDEO',{'IsHighRes':'FALSE'if case['size'][0]==640 else'TRUE'})
        core=binding.load(case['controller_file'])
        sys.argv=[str(ROOT/'tools'/case['controller_file']),*case['controller_arguments']]
        return core.main()
    finally:binding.restore();coverage_evidence.write_report=original_write


if __name__=='__main__':raise SystemExit(main())

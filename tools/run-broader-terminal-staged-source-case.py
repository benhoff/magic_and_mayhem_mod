#!/usr/bin/env python3
"""Preserve all derived guards while binding the actual retained staged manifest."""
import importlib.util,json,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_terminal_pause import verify_pause
from broader_retained_staged_manifest import verify_staged
from broader_derived_admission_current import verify_admission
from broader_interaction_recent_current import verify_recent
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
SUPPORT=['tools/run-broader-terminal-staged-source-case.py','tools/broader_retained_staged_manifest.py','tools/broader_terminal_pause.py']
def load(name,path):
    sp=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(sp);sp.loader.exec_module(m);return m
def main():
    require(len(sys.argv)>1 and sys.argv[1]=='admit'and sys.argv.count('--plan')==sys.argv.count('--case')==sys.argv.count('--validation')==1,'Exact retained admission only')
    core=load('staged_manifest_derived_driver','tools/run-broader-derived-source-case.py');core.verify_pause=verify_pause
    original_request=core.request_module
    def request_module():
        secondary=original_request();secondary.verify_staged=verify_staged;write=secondary.write_report
        def writer(root,target,body):
            if str(target).endswith('-runtime-variant-verification-01.json'):
                q=json.loads(body)
                with Reader(ROOT)as r:q['sources'].update({p:r.digest(p)for p in SUPPORT})
                body=(json.dumps(q,indent=2)+'\n').encode()
            return write(root,target,body)
        secondary.write_report=writer;return secondary
    core.request_module=request_module
    cid=sys.argv[sys.argv.index('--case')+1];name=P+cid+'-derived-admission-verification-01.json'
    with Reader(ROOT,memoize=True)as r:
        ref=lambda n:dict(record=n,sha256=r.digest(n));phase_ref=ref(sys.argv[sys.argv.index('--plan')+1]);phase=r.json(phase_ref['record']);case=next(v for v in phase['broader_cases']if v['id']==cid);acquisition=ref(case['report']);validation=ref(sys.argv[sys.argv.index('--validation')+1])
        if(ROOT/name).exists():
            recorded=r.json(name);a=r.json(acquisition['record']);v=r.json(validation['record'])
            require(recorded['success']and recorded['phase']==phase_ref and recorded['acquisition']==acquisition and recorded['validation']==validation
                and v['success']and v['sources_stable']and v['acquisition']==acquisition and all(r.digest(p)==h for p,h in recorded['sources'].items()),'Prior completed derived pre-admission changed')
            actual=verify_admission(r,ROOT,phase_ref,acquisition)
            require(recorded['actual_owned_runner_admission']==actual['actual_owned_runner_admission']and recorded['public_pause_collection']==verify_pause(r,a)
                and recorded['recent_interaction_sources']==verify_recent(r,phase,a),'Prior completed pre-admission no longer matches actual inputs')
            return request_module().main()
    write=core.write_report
    def writer(root,target,body):
        if str(target)==name:
            q=json.loads(body)
            with Reader(ROOT)as r:q['sources'].update({p:r.digest(p)for p in SUPPORT})
            body=(json.dumps(q,indent=2)+'\n').encode()
        return write(root,target,body)
    core.write_report=writer;return core.main()
if __name__=='__main__':raise SystemExit(main())

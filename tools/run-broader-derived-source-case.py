#!/usr/bin/env python3
"""Require explicit derived admission before the unchanged request state machine."""
import importlib.util,json,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_derived_budget import bound,predecessors,P
from broader_derived_admission import checked_configuration
from broader_derived_admission_current import verify_admission
from broader_interaction_recent_current import verify_recent
from broader_interaction_pause import verify_pause
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]


def request_module():
    path=ROOT/'tools/run-broader-secondary-source-case.py';sp=importlib.util.spec_from_file_location('derived_secondary_request_guard',path);module=importlib.util.module_from_spec(sp);sp.loader.exec_module(module)
    module.bound=bound;module.predecessors=predecessors
    return module


def main():
    require(len(sys.argv)>1 and sys.argv[1]in('acquire','admit')and sys.argv.count('--plan')==sys.argv.count('--case')==1,
            'Unique declared request operation/phase/case required')
    name=sys.argv[sys.argv.index('--plan')+1];cid=sys.argv[sys.argv.index('--case')+1]
    with Reader(ROOT,memoize=True)as reader:
        phase_ref=dict(record=name,sha256=reader.digest(name));phase=reader.json(name)
        case=next(c for c in phase['broader_cases']if c['id']==cid)
        if 'scene_runtime_variant'in phase:
            require(phase['family']=='interaction'and 'derived_native_admission'in phase,'Explicit derived startup declaration required')
            checked_configuration(reader,ROOT,phase_ref,case['report'])
            if sys.argv[1]=='admit':
                require(sys.argv.count('--validation')==1,'One completed base validation required')
                vn=sys.argv[sys.argv.index('--validation')+1];validation=dict(record=vn,sha256=reader.digest(vn));v=reader.json(vn)
                acquisition=dict(record=case['report'],sha256=reader.digest(case['report']))
                require(v['success']and v['sources_stable']and v['acquisition']==acquisition,'Incomplete/foreign current validation')
                proof=verify_admission(reader,ROOT,phase_ref,acquisition);proof.update(schema=1,
                    kind='current_derived_native_admission_pre_admission_verification',success=True,validation=validation,
                    actual_ledger_rows_renamed=False,sources={p:reader.digest(p)for p in ['tools/run-broader-derived-source-case.py','tools/broader_derived_admission_current.py','tools/broader_derived_admission.py']},sources_stable=True)
                require('recent_interaction_sources'in phase,'Missing current source-target declaration')
                proof['recent_interaction_sources']=verify_recent(reader,phase,reader.json(case['report']))
                proof['sources'].update({p:reader.digest(p)for p in ['tools/broader_interaction_recent_current.py','tools/broader_interaction_recent_sources.py']})
                require('interaction_pause_policy'in phase,'Missing declared actual pause collection policy')
                proof['public_pause_collection']=verify_pause(reader,reader.json(case['report']))
                proof['sources'].update({p:reader.digest(p)for p in ['tools/broader_interaction_pause.py','tools/compare-broader-paused-interaction.py']})
                report=P+cid+'-derived-admission-verification-01.json';require(not(ROOT/report).exists(),'Never overwrite a derived admission verification')
                digest=write_report(ROOT,report,(json.dumps(proof,indent=2)+'\n').encode());print(json.dumps(dict(stage='derived-admission-pre-admission-verified',report=report,sha256=digest)),flush=True)
    return request_module().main()
if __name__=='__main__':raise SystemExit(main())

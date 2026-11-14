"""Preserve every prior current predicate and require explicit derived startup admission."""
import sqlite3
from pathlib import Path
from types import FunctionType
import broader_player_secondary_matrix as previous
from broader_player_secondary_matrix import P,BINARY,replacements,replacement
from broader_derived_budget import bound,predecessors
from broader_derived_admission_current import verify_admission
from broader_player_acceptance import checked
from broader_interaction_recent_current import verify_recent
from broader_interaction_pause import verify_pause
from rendering_pilot import require
CATALOG=P+'derived-source-catalog-03.json'
specs=FunctionType(previous.specs.__code__,dict(previous.specs.__globals__,CATALOG=CATALOG),previous.specs.__name__,previous.specs.__defaults__,previous.specs.__closure__)


def requests(root,reader):
    with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
        db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
    plan=reader.json(P+'plan-48.json');limit=bound(reader,plan);charge=predecessors(reader,rows,plan)
    return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,
        maximum_additional_original_starts=limit,maximum_combined_original_starts=258,
        prior_WP10_original_starts=8,all_reservations_retained=True,rows=rows)


def current(root,reader,spec,row):
    value=previous.current(root,reader,spec,row);phase=reader.json(spec['plan'])
    if 'scene_runtime_variant'not in phase:return value
    require('derived_native_admission'in phase,'Missing explicit derived startup admission configuration')
    name=P+spec['case']['id']+'-derived-admission-verification-01.json';record=checked(reader,dict(record=name,sha256=reader.digest(name)))
    phase_ref=dict(record=spec['plan'],sha256=reader.digest(spec['plan']))
    require(record['kind']=='current_derived_native_admission_pre_admission_verification'
            and record['phase']==phase_ref and record['acquisition']==value['acquisition']
            and record['validation']==value['validation']and record['actual_ledger_rows_renamed']is False
            and all(reader.digest(p)==h for p,h in record['sources'].items()),'Derived admission verification changed')
    proof=verify_admission(reader,root,phase_ref,value['acquisition'])
    require(record['actual_owned_runner_admission']==proof['actual_owned_runner_admission']
            and record['actual_compiled_header_sha256']==proof['actual_compiled_header_sha256']
            and record['physical_header_sha256']==proof['physical_header_sha256'],
            'Current derived input/admission differs from admission')
    require('recent_interaction_sources'in phase,'Missing recent source target declaration')
    recent=verify_recent(reader,phase,reader.json(value['acquisition']['record']))
    require(record['recent_interaction_sources']==recent,'Current physical target differs from admission')
    value['recent_interaction_sources_current']=recent
    require('interaction_pause_policy'in phase,'Missing explicit public pause policy')
    paused=verify_pause(reader,reader.json(value['acquisition']['record']))
    require(record['public_pause_collection']==paused,'Actual pause collection differs from admission')
    value['public_pause_collection_current']=paused
    value['derived_admission']=dict(record=name,sha256=reader.digest(name));value['derived_admission_current']=proof
    value['phase']=dict(value['phase'],derived_native_admission_verified=True)
    return value

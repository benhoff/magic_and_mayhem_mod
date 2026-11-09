#!/usr/bin/env python3
"""Verify retained NCS1 chunks actually consumed by native resident replay."""
import argparse,importlib.util,json,tempfile
from collections import Counter
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_menu_acceptance import artifact
from rendering_pilot import require
from resident_capture import records
from natural_copy_applicability import execution_review

ROOT=Path(__file__).resolve().parents[1];P='research/runtime/rendering-copy-natural-results-20261108/'
NAMES=['default640-10','default800-02','software640-01','software800-01']
BIDS=['NP.native-stretch-copy-prefix','NP.natural-copy-checkpoint-observation']
SID='native.natural-copy-stretch-prefix'


def inspect_case(reader,name,inspector):
    ref=P+name+'-acquisition.json';acquired=reader.json(ref)
    require(acquired['success'] and acquired['sources_stable'] and acquired['private_wine_stopped']
            and acquired['retirement']['state']=='completed','Completed native session required')
    applicability=execution_review(reader,acquired)
    resident=json.loads(artifact(reader,acquired,'resident-world-report.json'));capture=resident['stage_profile']['capture']
    require(resident['normal_quit_retired'] and not resident['pending'] and not resident['error']
            and resident['stage_profile']['capture_complete'] and capture['complete'] and capture['finished']
            and not capture['truncated'] and not capture['error'] and capture['queue_bytes']==0
            and capture['accepted_source_chunks']==capture['written_source_chunks']==int(resident['source_chunks'])>0
            and int(resident['completed'])==int(resident['imports'])==int(resident['verified_returns'])==0,
            'Incomplete source-only lifetime/capture')
    raw=artifact(reader,acquired,Path(capture['path']).name)
    require(len(raw)==capture['file_bytes'],'Resident archive extent differs')
    chunks=[];callers=Counter();geometries=Counter();total=0;stretches=0
    with tempfile.TemporaryDirectory(prefix='stretch-prefix-') as directory:
        archive=Path(directory)/'sources.bin';archive.write_bytes(raw)
        for generation,chunk,source,reply in records(archive):
            require(chunk and generation==1 and not reply,'Source-only Grimoire capture required')
            _,decoded=inspector.decode(source,complete=False)
            require(not any(r[2]==13 for r,_ in decoded),'Source producer failure retained')
            selected=[r for r,_ in decoded if r[23]==0x4e435331]
            for r in selected:
                require(r[2]==6 and r[21]==0 and r[14]==31 and r[15]>0 and r[16]>0
                        and (r[12]-r[10],r[13]-r[11])!=(r[15],r[16]),'Unexpected natural NCS1 record')
                callers[hex(r[22])]+=1;geometries[(r[12]-r[10],r[13]-r[11],r[15],r[16])]+=1
            total+=len(decoded);stretches+=len(selected)
            chunks.append(dict(generation=generation,chunk=chunk,records=len(decoded),stretch_records=len(selected)))
    require(len(chunks)==int(resident['source_chunks']) and total==int(resident['source_chunk_records']) and stretches>0,
            'No positively consumed NCS1 prefix')
    return dict(acquisition=dict(record=ref,sha256=reader.digest(ref)),configuration=acquired['configuration'],
        identity=acquired['session_identity'],chunks=chunks,source_records=total,stretch_records=stretches,
        original_callers=dict(callers),geometries=[dict(source_size=list(k[:2]),destination_size=list(k[2:]),calls=v) for k,v in sorted(geometries.items())],
        source_applicability=applicability,source_only=True,worlds=0,normal_quit_retired=True,
        original_game_runs=acquired['original_game_runs'],renderer=resident['renderer'])


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--scout-only',action='store_true');a=p.parse_args()
    require(a.report.startswith(P) and not (ROOT/a.report).exists(),'Fresh source-prefix report required')
    spec=importlib.util.spec_from_file_location('stretch_source_inspector',ROOT/'tools/inspect-canvas-producers.py')
    inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
    with Reader(ROOT) as reader:
        register=reader.json('research/runtime/coverage/register.json')
        behaviors=[next(b for b in register['behaviors'] if b['id']==bid) for bid in BIDS]
        paths=set().union(*(required_sources(b) for b in behaviors))|{
            'tools/analyze-rendering-stretch-prefixes.py','tools/resident_capture.py','tools/coverage_evidence.py',
            'tools/coverage_claims.py','tools/natural_copy_applicability.py','tools/rendering_menu_acceptance.py','tools/rendering_pilot.py'}
        sources={n:reader.digest(n) for n in sorted(paths)}
        cases=[inspect_case(reader,n,inspector) for n in (NAMES[:1] if a.scout_only else NAMES)]
        require(len({c['identity'] for c in cases})==len(cases),'Repeated original session identity')
        claims=[]
        if not a.scout_only:
            require({(c['configuration']['software'],tuple(c['configuration']['actual_size'])) for c in cases}==
                    {(s,d) for s in (False,True) for d in ((640,480),(800,600))},'Four source-prefix domains required')
            scenario=next(s for s in register['scenarios'] if s['id']==SID)
            claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{}),scenarios={SID:scenario_contract(scenario)}) for b in behaviors]
        require(sources=={n:reader.digest(n) for n in paths},'Source-prefix analyzer changed')
        result=dict(schema=1,kind='natural_native_stretch_source_prefix_integration',success=True,sources_stable=True,
            sources=sources,cases=cases,verified_domains=len(cases),source_records=sum(c['source_records'] for c in cases),
            stretch_records=sum(c['stretch_records'] for c in cases),claims=claims,
            original_game_runs=0,newly_passed_requirements=0,worlds=0,
            scope='Actual native-only resident consumption of captured source-only NCS1 records and ordinary original/resource retirement. Prefix record counts are independent of visible C3 stretch-call counts. Whole hidden replay pixels, original driver sampling, natural alias reachability and World/gameplay are not claimed.')
    digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,sha256=digest,domains=len(cases),stretch_records=result['stretch_records'])))


if __name__=='__main__':main()

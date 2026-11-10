#!/usr/bin/env python3
"""Join actually executed indexed-primary callers to complete independent output."""
import argparse,hashlib,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources
from rendering_menu_acceptance import artifact
from rendering_pilot import require
from resident_capture import records
from required_boundary_source_review import review
from required_boundary_observation import PREFIX
ROOT=Path(__file__).resolve().parents[1]
BID='NP.world-indexed-primary-caller'

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('acquisition','comparison','report'):p.add_argument('--'+n,required=True)
    a=p.parse_args();require(a.report.startswith(PREFIX) and not(ROOT/a.report).exists(),'Fresh source analysis required')
    result=dict(schema=1,kind='installed_indexed_primary_caller_analysis',success=False,
        original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0)
    with Reader(ROOT)as reader:
        aq,cq=reader.json(a.acquisition),reader.json(a.comparison)
        result['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
        result['comparison']=dict(record=a.comparison,sha256=reader.digest(a.comparison))
        reg=reader.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']==BID)
        paths=required_sources(b)|{'tools/check-required-primary-installed.py','tools/required_boundary_source_review.py','tools/resident_capture.py','tools/inspect-canvas-producers.py'}
        sources={n:reader.digest(n)for n in sorted(paths)};result['sources']=sources
        result['claims']=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}))]
        try:
            require(aq['success'] and aq['sources_stable'] and cq['success'] and cq['sources_stable']
                    and cq['proof']['verified'] and cq['acquisition']==result['acquisition']
                    and cq['comparison']['mismatching_pixels']==0,'Incomplete actual execution/output')
            _,exceptions=review(reader,aq['sources']);result['source_applicability']=exceptions
            source=Path(aq['temporary_root'])/'worlds.bin'
            require(source.read_bytes()==artifact(reader,aq,'worlds.bin'),'Retained source capture changed')
            spec=importlib.util.spec_from_file_location('required_caller_producers',ROOT/'tools/inspect-canvas-producers.py')
            producer=importlib.util.module_from_spec(spec);spec.loader.exec_module(producer)
            counts={str(site):0 for site in (0x500477,0x500496)};examples={key:[]for key in counts}
            worlds=chunks=0
            for generation,chunk,stream,request in records(source):
                if chunk:chunks+=1
                else:worlds+=1
                _,rows=producer.decode(stream,complete=False)
                for r,_ in rows:
                    if r[2]!=9 or str(r[22])not in counts:continue
                    key=str(r[22]);counts[key]+=1
                    require(r[14]==0 and r[15]==(1 if r[22]==0x500477 else 3),'Unexpected required caller raster family')
                    if len(examples[key])<8:examples[key].append(dict(generation=generation,source_chunk=chunk,
                        sequence=r[1],family=r[14],variant=r[15],format=r[17],returned_eax=r[21]))
            result.update(worlds=worlds,source_chunks=chunks,counts=counts,examples=examples)
            require(worlds==cq['original_reference']['worlds'] and chunks==cq['original_reference']['source_chunks'],
                    'Incomplete source history')
            result.update(success=True,worlds=worlds,source_chunks=chunks,counts=counts,examples=examples,
                natural_primary_witness=counts[str(0x500477)]>0,natural_clipped_fallback_witness=counts[str(0x500496)]>0,
                independently_compared_pixels=cq['comparison']['pixels_compared'],
                preserved_source_phase=aq['sources'],
                scope='Count original500477 CPU-dispatch and500496 clipped continuations in the complete captured and independently compared source history. Zero counts remain scoped absence observations, not global unreachability or a fabricated repaired natural witness. Only the exact generic13/14 and adjacent clipped17 guard sites are added. Existing exact original continuation and generic complete-state/pixel controls supply their separate local semantics. No five optional helper reachability, global dead-code, arbitrary input or whole-player promotion.')
        except Exception as e:result['error']=str(e)
        finally:
            result['sources_stable']=all(reader.digest(n)==h for n,h in sources.items());result['success'] &= result['sources_stable']
            h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'),counts=result.get('counts'))),flush=True)
    return not result['success']
if __name__=='__main__':raise SystemExit(main())

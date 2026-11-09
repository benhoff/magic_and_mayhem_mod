#!/usr/bin/env python3
"""Freeze one bounded caller-state acquisition phase before any game start."""
import argparse
import hashlib
import importlib.util
import json
import shlex
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-installed-results-20261108/'
CONTROLS={'tools/run-rendering-caller-state.py','tools/prepare-rendering-caller-state.py',
          'tools/run-rendering-interaction.py','tools/rendering_interaction.py',
          'tools/rendering_interaction_budget.py','tools/rendering_interaction_live.py'}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',default=PREFIX+'input-plan-01.json')
    p.add_argument('--capacity-repair',help='Retained diagnosis and minimal cumulative budget extension; no charge reset')
    p.add_argument('--parent',help='Retain unchanged exact source snapshots from an earlier phase; request/start caps remain cumulative')
    a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned plan required')
    spec=importlib.util.spec_from_file_location('caller_campaign',ROOT/'tools/test-native-campaign.py')
    campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
    build=ROOT/'working/build/qt-shell'
    with Reader(ROOT)as reader:
        parent=reader.json(a.parent)if a.parent else None
        if parent:require((parent['bounds']['maximum_requests'],parent['bounds']['maximum_original_games'])in((8,6),(9,7)),'Parent budget differs')
        repair=reader.json(a.capacity_repair)if a.capacity_repair else None
        if repair:
            require(parent is not None and repair['success']and repair['kind']=='caller_state_capacity_failure_and_bounded_repair_plan'and repair['new_bounds']==dict(maximum_requests=9,maximum_original_games=7)and repair['requests_charged']==7 and repair['original_games_charged']==5,'Unreviewed capacity repair')
            require(repair['parent_plan']==dict(record=a.parent,sha256=reader.digest(a.parent)),'Capacity repair parent differs')
        compiled={n:reader.digest(n)for n in sorted(campaign.linked_compiler_sources(build,ROOT))}
        oracle=set()
        for dep in [ROOT/'working/original-reference.d',*build.glob('CMakeFiles/mnm-acceptance-world-history.dir/**/*.o.d')]:
            require(dep.exists(),'Missing oracle compiler dependency closure')
            for word in shlex.split(dep.read_text().replace('\\\n',' ')):
                path=Path(word).resolve()
                if path.is_file()and path.is_relative_to(ROOT)and not path.is_relative_to(ROOT/'working'):
                    oracle.add(path.relative_to(ROOT).as_posix())
        require({'tests/acceptance-world-reference.cpp','tests/acceptance-world-history.cpp'}<=oracle,'Incomplete oracle closure')
        def binary(path,label):
            data=(ROOT/path).read_bytes();record=a.report.removesuffix('.json')+'-'+label+'.bin'
            return dict(path=path,record=record,sha256=write_report(ROOT,record,data),bytes=len(data))
        sources={n:reader.digest(n)for n in sorted(native_runtime_source_paths(ROOT)|CONTROLS)}
        preimages={}
        for name,digest in (sources|compiled|{n:reader.digest(n)for n in oracle}).items():
            old=parent.get('source_preimages',{}).get(name)if parent else None
            if old and old['sha256']==digest:
                require(reader.digest(old['record'])==digest,'Lost parent source preimage')
                preimages[name]=old;continue
            record=a.report.removesuffix('.json')+'-source-'+hashlib.sha256(name.encode()).hexdigest()[:20]+'.bin'
            require(write_report(ROOT,record,(ROOT/name).read_bytes())==digest,'Source snapshot changed')
            preimages[name]=dict(record=record,sha256=digest)
        result=dict(schema=1,kind='caller_state_prospective_plan',success=True,
            sources={n:sources[n]for n in sorted(CONTROLS)},runtime_sources=sources,
            compiler_sources=compiled,source_preimages=preimages,
            player_binary=binary('working/build/qt-shell/mnm-qt-shell','player'),
            oracle=dict(native_sources={n:reader.digest(n)for n in sorted(oracle)},binaries=[
                binary('working/original-reference','original-reference'),
                binary('working/build/qt-shell/mnm-acceptance-world-history','original-history')]),
            bounds=dict(maximum_requests=9 if repair else 8,maximum_original_games=7 if repair else 6,primary_games=4,
                stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
            domains=[dict(software=s,size=z,scaling=['sharp','integer'])for s in(False,True)for z in([800,600],[640,480])],
            scope='Actual V5 primitive return state with exact source identity, final source prefix, independent original World and full viewport comparison, normal owner retirement. Current original semantic comparisons and provider contracts are separately required. No whole-engine or final rendering decision follows automatically.',
            independence='New caller-visible state question. Prior failed requests, exhausted caps and original starts remain historical and are neither erased nor refunded.')
        if repair:
            require(compiled==parent['compiler_sources']and result['player_binary']['sha256']==parent['player_binary']['sha256']and all(sources[n]==h for n,h in parent['runtime_sources'].items()if n not in repair['allowed_post_retirement_controls']),'Capacity repair changed native/player inputs')
            require({n for n,h in sources.items()if parent['runtime_sources'].get(n)!=h}==set(repair['allowed_post_retirement_controls']),'Capacity repair must change only the two reviewed host controls')
            result['capacity_repair']=dict(record=a.capacity_repair,sha256=reader.digest(a.capacity_repair))
        if parent:result['parent']=dict(record=a.parent,sha256=reader.digest(a.parent),scope='Host source phase and explicitly reviewed minimal capacity-repair extension, when present. Every acquisition request and original start remains charged.')
        digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
        require(reader.json(a.report)==result,'Plan read-back differs')
    print(json.dumps(dict(success=True,record=a.report,sha256=digest,compiler_sources=len(compiled),runtime_sources=len(sources))),flush=True)

if __name__=='__main__':main()

#!/usr/bin/env python3
"""Freeze the built player/original oracle and finite interaction budget."""
import argparse,hashlib,importlib.util,json,shlex
from pathlib import Path
from coverage_evidence import write_report,Reader
from rendering_build_binding import native_runtime_source_paths
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-interaction-results-20261107/'


def sha(p):return hashlib.file_digest(Path(p).open('rb'),'sha256').hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',default=PREFIX+'input-plan-01.json');parser.add_argument('--maximum-requests',type=int,default=6,choices=(6,7,8,9));args=parser.parse_args();target=args.report
    if not target.startswith(PREFIX) or '..' in Path(target).parts:raise ValueError('Owned immutable plan required')
    if (ROOT/target).exists():raise ValueError('Immutable plan exists')
    spec=importlib.util.spec_from_file_location('campaign',ROOT/'tools/test-native-campaign.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
    build=ROOT/'working/build/qt-shell'
    compiled={n:sha(ROOT/n) for n in sorted(m.linked_compiler_sources(build,ROOT))}
    def binary(path,label):
        raw=(ROOT/path).read_bytes();record=target.removesuffix('.json')+'-'+label+'.bin'
        return dict(path=path,record=record,sha256=write_report(ROOT,record,raw))
    oracle=set()
    for dep in [ROOT/'working/original-reference.d',*build.glob('CMakeFiles/mnm-acceptance-world-history.dir/**/*.o.d')]:
        for word in shlex.split(dep.read_text().replace('\\\n',' ')):
            p=Path(word).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):oracle.add(p.relative_to(ROOT).as_posix())
    assert {'tests/acceptance-world-reference.cpp','tests/acceptance-world-history.cpp'}<=oracle
    result=dict(schema=1,success=True,kind='finite_interaction_prospective_plan',
                sources={n:sha(ROOT/n) for n in ('tools/prepare-rendering-interaction.py','tools/run-rendering-interaction.py','tools/rendering_interaction.py','tools/rendering_interaction_live.py','tools/rendering_interaction_budget.py','tools/inspect-canvas-producers.py')},
                compiler_sources=compiled,player_binary=binary('working/build/qt-shell/mnm-qt-shell','player'),
                oracle=dict(native_sources={n:sha(ROOT/n) for n in sorted(oracle)},binaries=[binary('working/original-reference','original-reference'),binary('working/build/qt-shell/mnm-acceptance-world-history','original-history')]),
                runtime_sources={n:sha(ROOT/n) for n in sorted(native_runtime_source_paths(ROOT))},
                bounds=dict(maximum_requests=args.maximum_requests,maximum_original_games=args.maximum_requests-1,primary_games=4,repairs=args.maximum_requests-4,stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
                domains=[dict(software=s,size=z,scaling=['sharp','integer']) for s in (False,True) for z in ([800,600],[640,480])],
                map='Celtic Forest One (small)',actor=dict(slot=0,type=0,owner=0),
                input_policy=dict(target='Unique captured creature sprite at tracked draw anchor, nearest opaque pixel to frame centre after origin offsets; no hit-feedback search',minimum_key_hold_ms=220,paused_world='At most one observed-mode1 physical Space resume per scaling policy',movies='Enabled, at most two ordinary physical Escape cancellations; no movie-output credit'),
                scope='Four views, finite pan, original wizard hit/selection, cursor and complete independent viewport output. Numeric pool/slot identities are not allocation generations; no whole-player or final rendering closeout.')
    digest=write_report(ROOT,target,(json.dumps(result,indent=2)+'\n').encode())
    with Reader(ROOT) as r:assert r.json(target)==result
    print(json.dumps(dict(success=True,record=target,sha256=digest,compiler_sources=len(compiled),oracle_sources=len(oracle))))


if __name__=='__main__':main()

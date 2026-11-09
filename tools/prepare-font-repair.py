#!/usr/bin/env python3
"""Pin the sole remaining font scout after the retained direct-layout finding."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from font_lifetime import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-lifetime-results-20261108/'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--acquisition',required=True);p.add_argument('--review',required=True);p.add_argument('--controls',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned plan required')
    with Reader(ROOT) as r:
        old=r.json(PREFIX+'input-plan-01.json');acquired=r.json(a.acquisition);review=r.json(a.review);controls=r.json(a.controls)
        require(old['bounds']['maximum_charged_original_games']==2 and acquired['success']
                and acquired['original_game_runs']==1 and acquired['sources_stable']
                and review['success'] and review['source_binding_missing']
                and review['acquisition']==dict(record=a.acquisition,sha256=r.digest(a.acquisition))
                and controls['success'] and controls['sources_stable'],'Repair preconditions differ')
        require(all(r.digest(n)==h for n,h in controls['sources'].items()),'Repair controls no longer current')
        require(all(r.digest(n)==h for n,h in old['compiler_sources'].items()),'Retained player compiler closure changed')
        binary=ROOT/old['player_binary']['path']
        require(hashlib.sha256(binary.read_bytes()).hexdigest()==old['player_binary']['sha256'],'Tested player changed')
        names=set(old['compiler_sources'])|native_runtime_source_paths(ROOT)|{n.relative_to(ROOT).as_posix() for n in (ROOT/'tools').glob('*.py')}
        q=dict(old,kind='installed_font_direct_source_repair_plan',
               sources={n:r.digest(n) for n in sorted(names)},
               configurations=[dict(software=False,size=[800,600])],
               previous_plan=dict(record=PREFIX+'input-plan-01.json',sha256=r.digest(PREFIX+'input-plan-01.json')),
               actual_missing_data=dict(record=a.review,sha256=r.digest(a.review)),
               controls=dict(record=a.controls,sha256=r.digest(a.controls)),
               charged_prior_requests=1,charged_prior_original_games=1,remaining_original_games=1,
               temporary_evidence_staging='working/coverage/evidence-pending.sqlite3; paired verified import required before closeout',
               repair_reason='The normally retired first scout observes6loads/2replacements/4destructors and actual glyph calls, but no byte-draw parent executes. The existing glyph entry is now associated with exactly one current raw-font extent and retains each first complete normalized glyph source. One default-profile missing-data repair only; no Software expansion, new scenario, matrix or cap increase.',
               source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip())
    digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,record=a.report,sha256=digest,remaining_original_games=1)),flush=True)

if __name__=='__main__':raise SystemExit(main())

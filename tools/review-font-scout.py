#!/usr/bin/env python3
"""Review missing font binding in the first complete diagnostic acquisition."""
import argparse
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from font_lifetime import decode,require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-lifetime-results-20261108/'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned result required')
    with Reader(ROOT) as r:
        acquired=r.json(a.acquisition)
        require(acquired['success'] and acquired['sources_stable'] and acquired['original_game_runs']==1
                and acquired['retirement']['state']=='completed' and acquired['private_wine_stopped']
                and acquired['original_manifest_verified_before_after'],'Complete diagnostic acquisition required')
        refs=[v for v in acquired['artifacts'] if v['path'].endswith('/font-lifetime-v1.bin')]
        require(len(refs)==1 and r.digest(refs[0]['record'])==refs[0]['sha256'],'Missing exact diagnostic journal')
        observed=decode(r.read_bytes(refs[0]['record']))
        unknown=sum(row[2]==3 and row[23]==1 for row in observed['rows'])
        require(observed['version']==1 and not observed['uses'] and unknown>0 and not observed['live_at_detach']
                and {v['role'] for v in observed['loads']}=={1,2,3,4},'Unexpected missing-binding diagnostic')
        q=dict(schema=1,kind='font_installed_diagnostic_readiness',success=True,
               acquisition=dict(record=a.acquisition,sha256=r.digest(a.acquisition)),journal=refs[0],
               source_binding_missing=True,interesting_branches=unknown,unbound_actual_glyph_calls=unknown,
               bound_glyph_uses=0,loads=observed['loads'],retired=observed['retired'],live_at_detach={},
               font_lifecycle_qualified=False,newly_passed_requirements=0,original_game_runs=0,
               sources={n:r.digest(n) for n in ('tools/review-font-scout.py','tools/font_lifetime.py')},
               scope='Completed original load/reload/destructor trace with actual unparented glyph calls. This is a positive lead for one missing-data observation repair, not glyph/resource/pixel/DC acceptance. Original sources and historical limits remain unchanged.')
    digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,record=a.report,sha256=digest,unbound_glyph_calls=unknown)),flush=True)

if __name__=='__main__':raise SystemExit(main())

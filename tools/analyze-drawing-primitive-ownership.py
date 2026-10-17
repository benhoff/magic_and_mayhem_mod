#!/usr/bin/env python3
"""Rejoin source lifetimes with the unrelaxed first120update admission result."""
import argparse
import json
from pathlib import Path
import subprocess
from drawing_api_session import sha
from drawing_primitive_lifetimes import analyze
from drawing_primitive_ownership import join
ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('original','accounting','claims','report'):parser.add_argument('--'+name,type=Path,required=True)
    a=parser.parse_args()
    if a.report.exists():parser.error('Refusing evidence overwrite')
    declaration=json.loads(a.claims.read_bytes());sources=declaration['sources']
    stable=lambda:all(sha((ROOT/name).read_bytes())==digest for name,digest in sources.items())
    if not stable():raise ValueError('Prospective ownership sources changed')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,check=True)
    try:
        original_raw=a.original.read_bytes();original=json.loads(original_raw);accounting_raw=a.accounting.read_bytes();accounting=json.loads(accounting_raw)
        if not all(original.get(k) is True for k in ('success','sources_stable','original_drawing_active','original_manifest_verified_before_after','normal_original_quit_verified')) or accounting['original_report_sha256']!=sha(original_raw) or not accounting.get('raw_observations_match_original_report'):
            raise ValueError('Completed original and independently raw-bound accounting required')
        for name,digest in original['sources'].items():
            if sha((ROOT/name).read_bytes())!=digest:raise ValueError('Acquisition source changed: '+name)
        lifetimes=analyze(Path(original['experiment'])/'capture')
        if lifetimes!=original['primitive_lifetime_observation']:raise ValueError('Raw primitive observation changed')
        report=join(accounting,lifetimes)
        report.update(original_report=str(a.original),original_report_sha256=sha(original_raw),accounting_report=str(a.accounting),accounting_report_sha256=sha(accounting_raw),primitive_journal_sha256=lifetimes['journal_sha256'],sources=sources,claims=declaration['claims'],sources_stable=stable(),original_game_executed=False)
        if not report['sources_stable']:raise ValueError('Ownership source changed during analysis')
        with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
        print(json.dumps(dict(success=True,pending_calls=len(report['unique_pending_calls']),pending_tags=report['pending_tag_counts'],source_point_crossings=len(report['source_point_crossings']),crossings_after_original_return=report['crossings_after_original_return'])))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,check=True)


if __name__=='__main__':main()

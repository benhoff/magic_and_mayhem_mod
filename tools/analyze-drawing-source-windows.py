#!/usr/bin/env python3
"""Bind mapped-source read intervals to raw accepted-update time intervals."""
import argparse,json
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_wine_source_map import analyze as maps
from drawing_engine_windows import analyze as windows
from engine_cadence import decode
from drawing_source_windows import bind
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--original',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    raw=a.original.read_bytes();original=json.loads(raw)
    if original.get('source_sha256')!=BUILD or not all(original.get(k) is True for k in ('success','sources_stable','original_drawing_active','original_manifest_verified_before_after','normal_original_quit_verified','original_sections_unchanged')):raise ValueError('Completed original acquisition required')
    directory=Path(original['experiment'])/'capture';observed=maps(directory)
    if observed!=original['wine_source_map_observation'] or observed['wire_version'] not in (4,5):raise ValueError('Raw source maps changed or source clock missing')
    path=directory/'engine-cadence.bin';counter=windows(path)
    if counter!=original['engine_update_observation']:raise ValueError('Raw accepted update trace changed')
    frequency,rows=decode(path);last=[r for r in rows if r[1]==2][-1]
    if last[6]!=counter['excluded_final_sequence'] or frequency!=counter['frequency']:raise ValueError('Final update boundary differs')
    result=bind(observed['samples'],counter['windows'],last[3]|last[4]<<32,counter['owner_thread'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Analysis sources changed')
    result.update(sources=sources,sources_stable=True,claims=claims['claims'],original_report_sha256=sha(raw),original_report=str(a.original),source_journal_sha256=observed['journal_sha256'],counter_journal_sha256=counter['journal_sha256'],frequency=frequency,original_game_executed=False,scope='Temporal containment of mapped diagnostic source reads in observed accepted-update intervals. Foreign-thread, crossing and outside readings remain explicit; no producer, worker, alias, every-presentation or native parity claim.')
    a.report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:result[k] for k in ('source_samples','assigned_samples','crossing_samples','outside_samples')}))


if __name__=='__main__':main()

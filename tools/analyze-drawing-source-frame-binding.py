#!/usr/bin/env python3
"""Compare raw-bound mapped sources against original physical copy regions."""
import argparse,json,struct
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_source_frame_binding import evaluate
from drawing_wine_source_map import analyze
from drawing_wine_submission import decode as submissions
from drawing_api_physical import analyze as physical
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--original',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    raw=a.original.read_bytes();original=json.loads(raw)
    if not all(original.get(k) is True for k in ('success','sources_stable','original_drawing_active','original_manifest_verified_before_after','normal_original_quit_verified','original_sections_unchanged')):
        raise ValueError('Completed original mapping run required')
    if original.get('source_sha256')!=BUILD:raise ValueError('Original build identity differs')
    directory=Path(original['experiment'])/'capture'
    observed=analyze(directory)
    if observed!=original['wine_source_map_observation']:raise ValueError('Recorded map inputs changed')
    api={r[29]:r for r in struct.iter_unpack('<32I',(directory/'session.bin').read_bytes()[64:-128])}
    calls=submissions((directory/'wine-submission.bin').read_bytes(),api_calls=api,lifetime=(directory/'wine-lifetime.bin').read_bytes())
    if calls!=original['wine_submission_observation']:raise ValueError('Recorded submission fields changed')
    peer=Path(original['experiment'])/'physical-report.json'
    if sha(peer.read_bytes())!=original['physical_peer_report_sha256']:raise ValueError('Physical report changed')
    frames=physical(directory,peer)
    if frames!=original['physical_api_observation']:raise ValueError('Recorded physical fields changed')
    result=evaluate(calls['calls'],observed['samples'],observed['payloads'],frames['frames'],directory)
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Analysis sources changed')
    result.update(sources=sources,sources_stable=True,claims=claims['claims'],original_report_sha256=sha(raw),original_report=str(a.original),original_game_executed=False,scope='Reference-only direct RGB565 source/physical copy-region agreement at exact observed submissions and API witnesses. Retains every mismatch and unsupported witness. No production native-renderer, all-frame/routes or unobtrusive equivalence.')
    a.report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:result[k] for k in ('physical_witnesses','matched_regions','mismatched_regions','matched_pixels','all_witness_copy_regions_bound','all_compared_regions_match')}))


if __name__=='__main__':main()

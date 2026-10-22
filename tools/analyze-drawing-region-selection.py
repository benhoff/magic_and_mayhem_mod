#!/usr/bin/env python3
"""Revalidate actual Region button/latch observations against retained input actions."""
import argparse,json
from pathlib import Path
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
from drawing_api_session import BUILD,sha
from drawing_region_input import decode
from drawing_region_selection import observe
ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser();p.add_argument('--reference',type=Path,required=True);p.add_argument('--execution',type=Path);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    raw=read_bytes(a.reference, root=ROOT);r=json.loads(raw)
    if not r.get('success') or r.get('source_sha256')!=BUILD or not r.get('normal_original_quit_verified') or not r['staging'].get('region_input'):
        raise ValueError('Pinned successful original Region/exit/execution reference required')
    if a.execution:
        execution_raw=read_bytes(a.execution, root=ROOT);execution=json.loads(execution_raw)
        if execution['report_sha256']!=sha(raw) or execution['return_code']:raise ValueError('External execution declaration differs')
        provenance=dict(path=str(a.execution),sha256=sha(execution_raw),requested_environment=execution['requested_environment'])
    else:
        if 'requested_environment' not in r['staging']:raise ValueError('Captured environment or external execution declaration required')
        provenance=dict(boundary='capture_staging_manifest',requested_environment=r['staging']['requested_environment'])
    run=Path(r['experiment']);journal=(run/'capture/region-input.bin').read_bytes();region=decode(journal)
    if region!=r['region_input_observation']:raise ValueError('Recorded Region journal changed')
    result=observe(region,r['input_schedule']['actions'])
    names=['tools/drawing_region_selection.py','tools/analyze-drawing-region-selection.py','tools/drawing_region_input.py','tools/drawing_api_session.py','tests/test-drawing-region-selection.py']
    witnesses=[]
    for ordinal in [1,3,8]:
        action=r['input_schedule']['actions'][ordinal-1];path=run/'capture'/action['presentation_file']
        witnesses.append(dict(action_ordinal=ordinal,path=str(path),sha256=sha(path.read_bytes()),synchronized_original_frame=False))
    report=dict(schema=1,success=True,review_only=True,sources={n:evidence_digest(ROOT/n,root=ROOT) for n in names},reference=dict(path=str(a.reference),sha256=sha(raw)),execution=provenance,region_journal_sha256=sha(journal),analysis=result,visual_witnesses=witnesses,original_game_reexecuted=False,scope='Static immutable callback/action/raw-latch analysis and independent original visual witness hashes. Exact original mask1/2calls change and retain actor+68/count/mode on same observed pool/slot/type/owner; optional V3tracked sample establishes later latch when currenthit absent. No allocation generation, full persistent selection/tooltip semantics, source/frame/all-route/native parity or new original execution.')
    write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(result))
if __name__=='__main__':main()

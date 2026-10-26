#!/usr/bin/env python3
"""Search byte-checked original direct CFGs for computed alternate-slot writers."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from alternate_canvas_flow import inspect
from coverage_evidence import evidence_digest,read_bytes,read_json,write_report
from coverage_claims import behavior_contract,required_sources
from original_x86 import PE32,Decoder
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--frontier',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args()
 j=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT);b=next(b for b in j['behaviors'] if b['id']=='RS.alternate-canvas-provenance')
 paths=required_sources(b)|{'tools/alternate_canvas_flow.py','tools/recover-alternate-canvas-flow.py','tests/test-alternate-canvas-flow.py'}
 sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(paths)}
 report=dict(schema=1,success=False,static_only=True,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in j['builds']}))],original_sha256=BUILD,original_owner_recovered=False,all_writers_closed=False,global_unreachability_proved=False)
 before=False
 try:
  subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
  old=read_json(ROOT/'research/runtime/all-original-recovery-results-20261020/flow-review-final.json',root=ROOT)
  raw=read_bytes(a.frontier);digest=hashlib.sha256(raw).hexdigest()
  if digest!=old['raw_flow']['sha256']:raise ValueError('Retained recursive frontier changed')
  flow=json.loads(raw)
  if not flow['success'] or flow['original_sha256']!=BUILD:raise ValueError('Unadmitted frontier')
  retained='research/runtime/alternate-canvas-live-results-20261025/frontier-input.json'
  write_report(ROOT,retained,raw)
  image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
  if image.sha256!=BUILD:raise ValueError('Original image changed')
  rows={r['address']:r for r in flow['rows']};d=Decoder()
  try:
   for lo,hi in [(0x59455c,0x5945ec),(0x4e40d0,0x4e4263),(0x4fd438,0x4fd5af)]:
    at=lo
    while at<hi:
     row=d.decode(image.read(at,min(15,hi-at)),at);rows[at]=row;at+=row['size']
    if at!=hi:raise ValueError('Focused instruction boundary')
  finally:d.close()
  for at,r in rows.items():
   if image.read(at,r['size']).hex()!=r['bytes']:raise ValueError('Decoded bytes changed at '+hex(at))
  result=inspect(list(rows.values()),roots=[image.entry])
  report.update(result,success=True,raw_frontier=dict(record=retained,sha256=digest),prior_unresolved_indirect_edges=len(flow['unresolved_indirect_edges']),limitations=['Direct CFG and constant-only candidate search,not exhaustive alias analysis or proof of dormant code.','Calls discard register facts;computed call/jump targets,unknown memory values,stack aliases,external writers and arithmetic encodings remain unresolved.','Bulk writer records retain unknown direction and count;zero candidates never closes original ownership.','Original allocation,initial pixel clear,nonzero publication,base/source relationship and retirement remain unrecovered.'])
 except Exception as error:report['error']=str(error)
 finally:
  if before:
   try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);report['original_manifest_verified_before_after']=True
   except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
  report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report.get(k) for k in ('success','error','decoded_instructions','branch_entries_with_constants','unresolved_computed_stores')}|dict(sha256=digest,overlapping_writes=len(report.get('overlapping_writes',[])),escaped_pointer_addresses=len(report.get('escaped_pointer_addresses',[])),bulk_writers=len(report.get('bulk_writers',[])))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())

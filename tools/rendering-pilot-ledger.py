#!/usr/bin/env python3
"""Admit a narrowly scoped EP03 execution receipt; preserve both final decisions."""
import argparse,hashlib,json,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot_ledger import Workspace,payload
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('acquisition','comparison','software-probe','reason','report'):p.add_argument('--'+name,required=True)
 a=p.parse_args()
 if not a.report.startswith('research/runtime/rendering-acceptance-pilot-results-20261031/') or (ROOT/a.report).exists():p.error('New owned immutable report required')
 plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
 names=['tools/rendering-pilot-ledger.py','tools/rendering_pilot_ledger.py','tools/rendering_pilot.py','tools/rendering_acceptance.py','tools/rendering_execution.py','tools/rendering_completion.py','tools/coverage_claims.py','docs/native-rendering-closure.json']
 report=dict(schema=1,kind='rendering_pilot_admission',success=False,player_accepted=False,original_recovery_accepted=False,sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names})
 with Reader(ROOT) as reader:
  try:
   value=dict(kind='EP03.pipeline',reason=a.reason,**{k:dict(record=v,sha256=reader.digest(v)) for k,v in [('acquisition',a.acquisition),('comparison',a.comparison),('software_probe',a.software_probe)]})
   report['execution']=payload(reader,value);workspace=Workspace(ROOT,plan)
   try:report['receipt_id']=workspace.append(reader,value)
   finally:workspace.close()
   workspace=Workspace(ROOT,plan)
   try:report['snapshot']=workspace.snapshot(reader)
   finally:workspace.close()
   with tempfile.TemporaryDirectory(prefix='pilot-restore-',dir=ROOT/'working') as temporary:
    restored=Workspace(temporary,plan)
    try:
     restored.restore(reader,report['snapshot']);assert restored.snapshot(reader)==report['snapshot']
    finally:restored.close()
   report['reopen_and_restore_verified']=True;report['success']=True
  except Exception as e:report['error']=str(e)
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in report['sources'].items());report['success'] &= report['sources_stable']
  h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'),record=a.report,sha256=h,receipt_id=report.get('receipt_id'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())

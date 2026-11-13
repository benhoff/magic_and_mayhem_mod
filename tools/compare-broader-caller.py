#!/usr/bin/env python3
"""Run the unchanged caller comparer with an explicit empty-preimage schema binding."""
import argparse,json,runpy,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_nominal_schemas import caller_preimages
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(add_help=False);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);args,rest=p.parse_known_args()
 with Reader(ROOT)as r:
  a=r.json(args.acquisition);ref=a['prospective_plan'];require(r.digest(ref['record'])==ref['sha256'],'Caller phase identity differs');plan=r.json(ref['record']);_,changed=caller_preimages(plan);require(changed and plan['source_preimages']==[],'Only the actual empty-list phase may use this binding');sources={n:r.digest(n)for n in('tools/compare-broader-caller.py','tools/broader_nominal_schemas.py')}
 count=[0];original_reader=coverage_evidence.Reader;original_write=coverage_evidence.write_report
 class BoundReader(original_reader):
  def json(self,path):
   value=super().json(path)
   if str(path)==ref['record']:
    value,changed=caller_preimages(value);require(changed,'Caller plan schema changed');count[0]+=1
   return value
 def writer(root,path,raw):
  if str(path)==args.report:
   q=json.loads(raw);require(count[0]==1,'Expected one exact source-preimage interpretation');q['analysis_schema_binding']=dict(phase=ref,field='source_preimages',recorded_empty_sequence=True,interpreted_empty_mapping=True,hashes_or_snapshots_waived=False,old_payload_changed=False,scope='Both empty shapes contain no preimage objects. The unchanged comparer still verifies every actual source and preserves changed/untracked source bytes in its analysis bundle; no source lookup,hash or evidence is waived.');q['sources'].update(sources)
   with original_reader(ROOT)as r:stable=all(r.digest(n)==h for n,h in q['sources'].items())
   q['sources_stable'] &= stable;q['success'] &= stable;raw=(json.dumps(q,indent=2)+'\n').encode()
  return original_write(root,path,raw)
 coverage_evidence.Reader=BoundReader;coverage_evidence.write_report=writer
 try:
  sys.argv=[str(ROOT/'tools/check-rendering-caller-state.py'),*sys.argv[1:]]
  try:runpy.run_path(str(ROOT/'tools/check-rendering-caller-state.py'),run_name='__main__')
  except SystemExit as exc:return exc.code or 0
 finally:coverage_evidence.Reader=original_reader;coverage_evidence.write_report=original_write
if __name__=='__main__':raise SystemExit(main())

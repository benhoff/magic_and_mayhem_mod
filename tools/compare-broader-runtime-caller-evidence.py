#!/usr/bin/env python3
"""Run the unchanged caller analyser with exact build-root source preimages."""
import argparse,hashlib,io,json,runpy,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader,write_report
from broader_runtime_caller_preimages import resolve
from broader_runtime_caller_evidence import payload_preimages
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
PREFIX='research/runtime/caller-state-installed-results-20261108/'
SUPPORT=('tools/compare-broader-runtime-caller-evidence.py','tools/broader_runtime_caller_preimages.py','tools/broader_runtime_caller_evidence.py')

def main():
 parser=argparse.ArgumentParser(add_help=False);parser.add_argument('--acquisition',required=True);parser.add_argument('--report',required=True);args,rest=parser.parse_known_args()
 require(args.report.startswith(PREFIX)and not(ROOT/args.report).exists(),'Fresh caller analysis report required')
 require('--diagnostic-unresolved-parents'not in rest and '--claims'in rest,'Only nominal source-bound caller analysis is allowed')
 with Reader(ROOT)as reader:
  acquired=reader.json(args.acquisition);phase_ref=acquired['prospective_plan'];require(reader.digest(phase_ref['record'])==phase_ref['sha256'],'Caller phase identity changed');phase=reader.json(phase_ref['record'])
  require(phase['family']=='caller'and acquired['success']and acquired['sources_stable']and acquired['private_wine_stopped'],'Incomplete caller acquisition')
  require(any(c['report']==args.acquisition for c in phase['broader_cases']),'Foreign caller acquisition')
  build_ref=phase['current_player_build'];require(reader.digest(build_ref['record'])==build_ref['sha256'],'Caller tested build identity changed');build=reader.json(build_ref['record'])
  require(build['success']and phase['compiler_sources']==build['compiler_sources']and phase['runtime_sources']==build['runtime_sources'],'Caller source root belongs to another native build')
  expected=phase['compiler_sources']|phase['runtime_sources'];require(len(expected)==1376 and all(acquired['sources'].get(n)==h for n,h in expected.items()),'Caller native source closure differs')
  reference=phase['source_preimages']
  def object_bytes(value):
   stream=io.BytesIO();reader.resolver.copy(value['sha256'],value['size'],stream);return stream.getvalue()
  body=object_bytes(reference)
  def retain(value,raw):
   name=P+'caller-runtime-source-preimages-01/'+value['sha256']+'.bin'
   if(ROOT/name).exists():require(reader.digest(name)==value['sha256'],'Retained caller preimage changed')
   else:require(write_report(ROOT,name,raw)==value['sha256'],'Caller snapshot identity changed')
   return dict(record=name,sha256=value['sha256'])
  mapping=resolve(reference,build['source_preimages'],body,expected,object_bytes,retain)
  declared=json.loads(Path(rest[rest.index('--claims')+1]).read_bytes());model=reader.json(rest[rest.index('--model-build')+1]);requirements=reader.json(rest[rest.index('--requirements')+1]);consumed=acquired['sources']|acquired.get('native_admission',{}).get('native_only',{}).get('sources',{})|phase['oracle']['native_sources']|model['sources']|requirements['sources']
  for name,h in declared['sources'].items():require(name not in consumed or consumed[name]==h,'Declaration overwrites an actual source phase');consumed[name]=h
  evidence_mapping,evidence_references=payload_preimages(mapping,consumed,lambda name:(ROOT/name).read_bytes(),reader.read_bytes,retain);require(len(evidence_mapping)==7,'Reviewed seven evidence-source references required');mapping.update(evidence_mapping)
  root_record=P+'caller-runtime-source-preimages-01/root-'+reference['sha256']+'.bin'
  if(ROOT/root_record).exists():require(reader.digest(root_record)==reference['sha256'],'Caller root snapshot changed')
  else:require(write_report(ROOT,root_record,body)==reference['sha256'],'Caller root snapshot identity changed')
  support={n:reader.digest(n)for n in SUPPORT};support_preimages={n:retain(dict(sha256=h,size=len((ROOT/n).read_bytes())),(ROOT/n).read_bytes())for n,h in support.items()}
 original_reader=coverage_evidence.Reader;original_write=coverage_evidence.write_report;count=[0]
 class BoundReader(original_reader):
  def json(self,path):
   value=super().json(path)
   if str(path)==phase_ref['record']:
    require(value['source_preimages']==reference,'Caller source root changed');value=dict(value,source_preimages=mapping);count[0]+=1
   return value
 def writer(root,path,raw):
  if str(path)==args.report:
   q=json.loads(raw);require(count[0]==1,'Expected one caller source-root interpretation')
   q['analysis_schema_binding']=dict(phase=phase_ref,build=build_ref,recorded_object_root=reference,root_snapshot=dict(record=root_record,sha256=reference['sha256']),resolved_source_count=len(expected),resolved_evidence_payload_count=len(evidence_mapping),evidence_payload_references=evidence_references,resolved_mapping_sha256=hashlib.sha256(json.dumps(mapping,sort_keys=True,separators=(',',':')).encode()).hexdigest(),recorded_payload_changed=False,native_source_waivers=False,source_lookup_waivers=False,original_game_runs_added=0,scope='Decode the exact tested-build1376source object root into immutable per-source references for the unchanged legacy mapping reader. Every member and byte is verified; seven generated evidence sources retain original payload bytes rather than pointer-file encodings; all original phase/acquisition hashes, semantic checks, caps, deadlines and binaries remain exact.')
   q['sources'].update(support);q['source_preimages'].update(support_preimages)
   with original_reader(ROOT)as reader:stable=all(reader.digest(n)==h for n,h in q['sources'].items())
   q['sources_stable'] &= stable;q['success'] &= stable;raw=(json.dumps(q,indent=2)+'\n').encode()
  return original_write(root,path,raw)
 coverage_evidence.Reader=BoundReader;coverage_evidence.write_report=writer
 try:
  sys.argv=[str(ROOT/'tools/check-rendering-caller-state.py'),*sys.argv[1:]]
  try:runpy.run_path(str(ROOT/'tools/check-rendering-caller-state.py'),run_name='__main__')
  except SystemExit as stopped:return stopped.code or 0
 finally:coverage_evidence.Reader=original_reader;coverage_evidence.write_report=original_write
if __name__=='__main__':raise SystemExit(main())

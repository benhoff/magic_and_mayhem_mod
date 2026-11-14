#!/usr/bin/env python3
"""Retain an exact comparison after classifying its identical oracle compiler."""
import argparse,copy,hashlib,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_policy_acceptance import assess
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('comparison','oracle-build','controls','classification-review','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(not(ROOT/args.report).exists(),'Fresh reanalysis report required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n))
  parent=r.json(args.comparison);require(parent['success']and parent['sources_stable']and parent['comparison_processes_stopped'],'Completed exact parent required')
  old_ref=parent['native_edge_policy']['oracle_build'];old=r.json(old_ref['record']);require(r.digest(old_ref['record'])==old_ref['sha256'],'Changed old oracle build')
  new=r.json(args.oracle_build);review=r.json(args.classification_review);controls=r.json(args.controls)
  before='tools/build-broader-policy-reference.py';after='tools/compile-broader-policy-oracle.py'
  require(review['success'] and review['production_native_files_changed']is False and review['original_game_runs']==0
          and old['binary']['sha256']==new['binary']['sha256']==review['identical_oracle_binary_sha256'],'Oracle execution identity changed')
  require(set(old['sources'])-{before}==set(new['sources'])-{after} and all(new['sources'][n]==h for n,h in old['sources'].items()if n!=before),'Oracle C++ or header changed')
  source=(ROOT/after).read_bytes();preimage=source.replace(after.encode(),before.encode())
  require(hashlib.sha256(preimage).hexdigest()==old['sources'][before] and hashlib.sha256(source).hexdigest()==new['sources'][after],'Compiler change exceeds its source-path classification')
  require(new['success']and controls['success']and all(r.digest(n)==h for values in(new['sources'],controls['sources'],parent['sources'])for n,h in values.items()),'Retained analysis dependency changed')
  require(r.digest(old['binary']['record'])==r.digest(new['binary']['record'])==new['binary']['sha256'],'Exact oracle binaries unavailable')
  q=copy.deepcopy(parent);q['native_edge_policy'].update(oracle_build=ref(args.oracle_build),controls=ref(args.controls),original_reference_executions_added=False)
  q['retained_policy_analysis']=dict(parent=ref(args.comparison),original_oracle_execution_build=old_ref,classification_review=ref(args.classification_review),new_original_game_runs=0,new_original_reference_executions=0,new_pixel_comparisons=0,oracle_binary_identical=True,expected_and_actual_pixels_unchanged=True)
  q['sources'].update(new['sources']|controls['sources']|{'tools/rebind-broader-policy-oracle.py':r.digest('tools/rebind-broader-policy-oracle.py')})
  n=P+'oracle-compiler-classification-preimage-01.bin';q['retained_policy_analysis']['old_compiler_source']=dict(record=n,sha256=write_report(ROOT,n,preimage))
  proof=assess(r,q);require(proof['verified'],'Policy certificate reanalysis refused');q['retained_policy_analysis']['certificate_reanalysis']=proof
  h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,new_original_game_runs=0,new_pixel_comparisons=0)),flush=True)
if __name__=='__main__':main()

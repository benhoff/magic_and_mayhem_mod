#!/usr/bin/env python3
"""Compare owned observation values, full packed snapshot copies, expiry and surviving task references."""
from pathlib import Path
import hashlib
import importlib.util
import json
import subprocess
import sys
from owner_planner_comparison import SUITES, main, ROOT
from coverage_evidence import read_json, write_report

BID='AI.owner-observation-snapshot-ownership'
SCENARIO='observation-snapshot-ownership-20261023'
DIRECTORY='research/runtime/observation-snapshot-ownership-results-20261023'
SUITES['observation-snapshot']=dict(
 ranges=[(0x4181d0,0x4189fe),(0x418b60,0x418cc8),(0x418d50,0x4198fb),(0x505f70,0x50607c),(0x42a5d0,0x42a6d8),(0x42a6e0,0x42ab4e),(0x42cde0,0x42d099),(0x42fb40,0x42fce0),(0x42f2e0,0x42f322),(0x52cb60,0x52cba0),(0x58f6f0,0x58f725),(0x54dac0,0x54dac1),(0x413330,0x41393c),(0x40f7f0,0x40f903),(0x417e60,0x417e7e),(0x40a340,0x40a3a8),(0x407650,0x407707)],
 endpoints=[0x597850,0x597870,0x5979b0],
 scope='',suite='owner-observation-snapshot',
 sources=['tests/observation-snapshot-ownership-test.cpp','reconstruction/creature-ai/owner_knowledge.cpp'],wrapper='tools/test-observation-snapshot-ownership.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['observation-snapshot']['scope']=behavior['scope']
 declaration_path=ROOT/DIRECTORY/(out.name+'-inputs.json')
 result_path=ROOT/DIRECTORY/(out.name+'.json')
 if declaration_path.exists() or result_path.exists():raise ValueError('Immutable result identity already exists')
 if '--claims' in sys.argv:
  claims_path=Path(sys.argv[sys.argv.index('--claims')+1]);declaration=json.loads(claims_path.read_text())
 else:
  spec=importlib.util.spec_from_file_location('task_storage_declarations',ROOT/'tools/draft-coverage-claims.py')
  module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
  declaration=module.declaration(ROOT,register,[BID],[SCENARIO])
  # Existing comparison runner consumes a physical declaration. This bounded
  # interchange file has one consumer; the durable declaration is SQLite.
  interchange=ROOT/'working/tests'/('claims-'+out.name+'.json')
  with interchange.open('x') as f:json.dump(declaration,f,indent=2)
  claims_path=interchange;sys.argv.extend(['--claims',str(claims_path)])
 base=subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip()
 changes={}
 for name,digest in declaration['sources'].items():
  raw=(ROOT/name).read_bytes();old=subprocess.run(['git','-C',str(ROOT),'show',base+':'+name],capture_output=True)
  if old.returncode or hashlib.sha256(old.stdout).hexdigest()!=digest:changes[name]={'sha256':digest,'text':raw.decode()}
 declaration.update(base_commit=base,changed_inputs=changes)
 bound=write_report(ROOT,declaration_path,(json.dumps(declaration,indent=2)+'\n').encode())
 try:main('observation-snapshot')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['ownership_domain']='Full packed snapshot bytes; original4181d0 modes1/2 with owner0,one actor and absent task/notification collections. Owned construction,refresh,expiry; original413330 bank1 source creation/clone and40a340 clone cleanup. Separate populated505f70 object/list/buffer destructor,explicit virtual object release and buffer-free services. No resource-bearing mode2 refresh acceptance.'
   report['native_representation_policy']='Host ordered observation index preserves value address/lifetime; PE red/black node topology,colors/iterator ABI excluded. Raw values include borrowed fixed fixture CFG pointer. Native reconcile consumes independently recovered logical OwnerKnowledge decisions,never original output. Cache adapter host header is balanced fixture allocation. Freed blocks poisoned; retained task source35 identity/activity checked without dereferencing released snapshots.'
   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()

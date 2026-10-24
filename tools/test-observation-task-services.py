#!/usr/bin/env python3
"""Compare new-sighting task credit with actual source traversal, ancestry recipient and contribution account."""
from pathlib import Path
import hashlib
import importlib.util
import json
import subprocess
import sys
from owner_planner_comparison import SUITES, main, ROOT
from coverage_evidence import read_json, write_report

BID='AI.owner-observation-task-credit'
SCENARIO='observation-task-services-20261023'
DIRECTORY='research/runtime/observation-task-services-results-20261023'
SUITES['observation-task-services']=dict(
 ranges=[(0x522770,0x52279a),(0x407920,0x407982),(0x4079f0,0x407a73),(0x416d80,0x416ea0),(0x505bf0,0x505c49),(0x505c50,0x505c95),(0x59bee0,0x59bf07),(0x4181d0,0x4189fe),(0x418b60,0x418cc8),(0x418d50,0x4198fb),(0x505f70,0x50607c),(0x42a5d0,0x42a6d8),(0x42a6e0,0x42ab4e),(0x42cde0,0x42d099),(0x42fb40,0x42fce0),(0x42f2e0,0x42f322),(0x52cb60,0x52cba0),(0x58f6f0,0x58f725),(0x54dac0,0x54dac1),(0x413330,0x41393c),(0x40f7f0,0x40f903),(0x417e60,0x417e7e),(0x40a340,0x40a3a8),(0x407650,0x407707)],
 endpoints=[0x597850,0x597870,0x5979b0],
 scope='',suite='owner-observation-task-services',
 sources=['tests/observation-task-services-test.cpp','reconstruction/creature-ai/owner_knowledge.cpp','reconstruction/creature-ai/contribution_account.cpp'],wrapper='tools/test-observation-task-services.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['observation-task-services']['scope']=behavior['scope']
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
 try:main('observation-task-services')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['ownership_domain']='Actual4181d0 mode1/2 new-observation insertion and notification: actual407920/4079f0 full source drain at mode3, restore mode before actual522770 parent-chain recipient lookup, actual416d80 with real505bf0/505c50 and59bee0 numeric helper. Level=health,quantity=min(health,ordinary1/wizard10),zero float arguments;task2f modulo32 ADD. Nonempty independently authored source forests and preinstalled task backlinks;both families0/1. Missing observer/root,empty or signed-ineligible sources,absent terminal task and wrong family produce no credit. Refresh/expiry silence and reacquisition credits;normal task source retirement. Alloc/free are explicit endpoints;no recipient/account/traversal stubs.'
   report['native_representation_policy']='Headless one-owner0/one-observed-actor increasing-tick non-reentrant storage/profile composition. Knowledge update and owned snapshot commit precede new-insertion credits;native ordered index excludes PE RB topology/iterator ABI. Source/recipient storage borrowed,acyclic ancestry/depth and signed-positive health/type bounds are native admission. Existing task factories author identical forest inputs to both runs;task installation/scheduler execution and concrete World activation/visibility providers remain separate. No original output feeds native inputs;whole-brain/failure/live excluded.'
   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()

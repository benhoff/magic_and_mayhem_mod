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

BID='AI.owner-broader-task-credit-services'
SCENARIO='broader-task-services-20261023'
DIRECTORY='research/runtime/broader-task-services-results-20261023'
SUITES['broader-task-services']=dict(
 ranges=[(4619392, 4619473), (4621424, 4622830), (5367760, 5367828), (5327248, 5327958), (5273328, 5274656), (5273072, 5273257), (5283408, 5286096), (5275392, 5275548), (5275552, 5275702), (5275712, 5276235), (5367040, 5367086), (4614256, 4614320), (4267600, 4267630), (5317728, 5318649), (4287536, 4287637), (4384272, 4384317), (5384048, 5384090), (4287872, 4288160), (5266416, 5266505), (5266512, 5266581), (5881568, 5881607), (4300352, 4300680), (4292688, 4293043), (4224592, 4224775)],
 endpoints=[5863504, 5863536, 4837664, 5085328, 5074736, 5061136, 5061168, 5830704, 5830800, 4601792, 5268000, 5519344, 4836784, 5520480, 5521120, 5281712, 5656208, 5706512, 4620528, 5351664, 5306448, 5351984, 5286704, 5271776, 4606768, 4606992, 5561696, 5561024],
 scope='',suite='owner-broader-task-credit-services',
 sources=['tests/broader-task-services-test.cpp', 'reconstruction/creature-ai/contribution_account.cpp', 'reconstruction/creature-ai/resource.cpp', 'reconstruction/creature-ai/segment_completion.cpp', 'reconstruction/creature-ai/maintenance.cpp'],wrapper='tools/test-broader-task-services.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['broader-task-services']['scope']=behavior['scope']
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
 try:main('broader-task-services')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['ownership_domain']='Concrete416c30/42e610 signed-ID task lookup in both brain indexes;522770 source ancestry;actual416d80/profile/truncation valuation shared by467c80 activation,468470 healing/resource/flag item interactions,514990 nonlethal damage,positive5076f0 healing,509e50 periodic resource contribution and512460 pending completion. Real419e40 fresh task construction and418050/407650 normal owned source retirement. Actual51e7d0 FC/deadline setter,5075f0 maximum,466870 subject,507f00 resource change,507fa0 owner snapshots and51e500 inventory aggregate execute. Both source families,duplicate/absent/ancestor recipients,signed-key misses,read-only profiles,wrapping counters. Registration,presentation,item application/removal/next-item,scenario,effect allocation and position/environment/animation/terrain services explicitly supplied. Nonlethal damage;normal no-landing/no-region completion;ordinary inactive maintenance siblings;authored index insertion/unbinding and source forests. No original output seeds native state.'
   report['native_representation_policy']='OwnerTaskCreditServices composes existing ContributionAccount,Resource,SegmentCompletion and Maintenance APIs with existing EffectServices callers and common task/source storage. Byte-address inputs and pointers are checked/borrowed;native depth/domain guards are policies. Task indexes queried only,not a new general tree insertion/erase ABI. No real item/world/presentation backend,arbitrary callback reentry,death/scenario cleanup,negative-activity forest destruction,source split/merge,dual-brain scheduler,training/failure/SEH or World/live claim.'
   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()

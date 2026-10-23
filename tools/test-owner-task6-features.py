#!/usr/bin/env python3
"""Compare generated task6 dynamic feature buffers through real factories and retirement."""
from pathlib import Path
import hashlib
import importlib.util
import json
import subprocess
import sys
from owner_planner_comparison import SUITES, main, ROOT
from coverage_evidence import read_json, write_report

BID='AI.owner-task6-generated-feature-ownership'
SCENARIO='owner-task6-features-20261023'
DIRECTORY='research/runtime/owner-task6-feature-results-20261023'
SUITES['task6-features']=dict(
 ranges=[(0x423590,0x425c9e),(0x4284e0,0x4289d7),(0x431be0,0x4328cc),(0x426070,0x4261a8),(0x4261b0,0x426310),(0x4289e0,0x428b3d),(0x428b40,0x428e32),(0x418050,0x4181b3),(0x4766b0,0x476b33),(0x432900,0x432fb0),(0x514990,0x514c20)],
 endpoints=[0x416d80,0x597850,0x597870,0x476e30,0x50af10,0x50fd50,0x50fef0,0x416fe0],
 scope='',suite='owner-task6-features',
 sources=['tests/owner-task6-features-test.cpp','reconstruction/creature-ai/owner_group_task.cpp','reconstruction/creature-ai/owner_group_features.cpp','reconstruction/creature-ai/owner_planner_features.cpp','reconstruction/creature-ai/owner_memory_groups.cpp','reconstruction/creature-ai/owner_task_probe.cpp','reconstruction/creature-ai/regional_navigation.cpp','reconstruction/creature-ai/regional_danger.cpp','reconstruction/pathfinding/route_neighbors.cpp','reconstruction/pathfinding/route_search.cpp','reconstruction/creature-ai/owner_task_lifecycle.cpp','reconstruction/creature-ai/owner_task_inputs.cpp','reconstruction/creature-ai/owner_planner.cpp','reconstruction/combat/melee.cpp','reconstruction/creature-ai/brain_inference.cpp','assets/dat.cpp'],wrapper='tools/test-owner-task6-features.py')

def run():
 if '--output' not in sys.argv:raise ValueError('Require --output')
 out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):raise ValueError('Require a new owned working/tests output')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 behavior=next(b for b in register['behaviors'] if b['id']==BID)
 SUITES['task6-features']['scope']=behavior['scope']
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
 try:main('task6-features')
 finally:
  if (out/'report.json').exists():
   report=json.loads((out/'report.json').read_text())
   report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
   report['scope']=behavior['scope']
   report['original_output_used_as_native_input']=False
   report['ownership_domain']='Generated task6 candidate/saved72 buffers with real426070 factories and418050/428b40 base cleanup. Whole bounded423590 planner/body fixture uses authored World/model/scoring/training/command endpoints; fresh native feature headers and logical group ownership are explicit. Original suffix backing retention is observed, not silently fixed.'
   report['native_representation_policy']='Checked value-address buffer identities normalized to length/capacity/content. Native arena teardown and original fixture cleanup separately release retained suffix backing after baseline observations.'
   write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
  if out.exists():(out/'.retain').write_text('Original/native/sanitizer task storage execution, immutable declaration, binaries and diagnostics. Preserve failed results too.\n')
if __name__=='__main__':run()
